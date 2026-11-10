
#include <WiFi.h>
#include "utils/WiFiContext.h"
#include "utils/Log.h"
#include "configuration.h"

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include "utils/MqttContext.h"

#define wifiTimeOfReconect pdMS_TO_TICKS(2000)


WiFiContext::WiFiContext(MqttContext &mqttContext)
    : _mqttContext(mqttContext)
    , _wifiReconnectTimer(NULL)
    , _settings()
    , _ipMode(IpModeStatic)
    , _attemptsInMode(0)
    , _hasIp(false)
    , _rescueApActive(false)
    , _lastIpMs(0)
    , _ipAcquiredMs(0)
{
}

void WiFiContext::initializate(const WiFiSettings &settings)
{
    LOG_INIT(PSTR("WiFiContext::initializate started"));

    _settings = settings;
    _ipMode = IpModeStatic;
    _attemptsInMode = 0;
    _hasIp = false;
    _rescueApActive = false;
    // Отсчёт до спасательной точки идёт с момента старта: если сеть
    // не поднимется вообще ни разу, канал всё равно появится.
    _lastIpMs = millis();
    _ipAcquiredMs = millis();

    _wifiReconnectTimer = xTimerCreate("wifiTimer", wifiTimeOfReconect, pdFALSE, this, reconnectTimerTrampoline);

    // Лямбда с this вместо свободной функции: WiFi.onEvent принимает
    // std::function<void(arduino_event_id_t, arduino_event_info_t)>.
    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
        (void)info;
        onWiFiEvent((WiFiEvent_t)event);
    });
    WiFi.mode(WIFI_STA);
    applyIpConfig();
    connectToWifi();

    LOG_INIT(PSTR("WiFiContext::initializate completed"));
}

bool WiFiContext::isRescueApActive() const
{
    return _rescueApActive;
}

bool WiFiContext::hasIp() const
{
    return _hasIp;
}

const char *WiFiContext::describeIpMode(IpMode mode)
{
    return (mode == IpModeStatic) ? PSTR("static") : PSTR("dhcp");
}

void WiFiContext::applyIpConfig()
{
    if (_ipMode == IpModeStatic)
    {
        WiFi.config(_settings.ip, _settings.gateway, _settings.subnet, _settings.dns);
    }
    else
    {
        // Нулевой адрес - сигнал ядру поднять DHCP-клиент:
        // set_esp_interface_ip() при info.ip.addr == 0 зовёт esp_netif_dhcpc_start().
        WiFi.config(IPAddress(), IPAddress(), IPAddress());
    }
    LOG(PSTR("WiFiContext::applyIpConfig mode: %s"), describeIpMode(_ipMode));
}

/* Вызывается только из supervise(), то есть из таска loop(). Переконфигурация
   радио из обработчика событий WiFi - плохая идея: это его же контекст. */
void WiFiContext::switchIpMode(IpMode mode)
{
    if (_ipMode == mode)
        return;

    LOG(PSTR("WiFiContext::switchIpMode %s -> %s"), describeIpMode(_ipMode), describeIpMode(mode));
    _ipMode = mode;
    _attemptsInMode = 0;

    // Если ассоциация жива, applyIpConfig() сам стартует DHCP-клиент и дело
    // закончится новым GOT_IP - реассоциация не нужна.
    applyIpConfig();

    if (!_hasIp)
    {
        connectToWifi();
    }
}

void WiFiContext::startRescueAp()
{
    if (_rescueApActive || _settings.rescueApSsid == NULL)
        return;

    _rescueApActive = true;

    // AP_STA, а не AP: station продолжает попытки, поэтому канал исчезнет
    // сам, как только домашняя сеть вернётся.
    WiFi.mode(WIFI_AP_STA);

    const bool secured = (_settings.rescueApPass != NULL && strlen(_settings.rescueApPass) >= 8);
    const bool ok = secured ? WiFi.softAP(_settings.rescueApSsid, _settings.rescueApPass)
                            : WiFi.softAP(_settings.rescueApSsid);

    LOG(PSTR("WiFiContext::startRescueAp ssid: %s secured: %d ok: %d ip: %s")
       ,_settings.rescueApSsid, (int)secured, (int)ok, WiFi.softAPIP().toString().c_str());
    LOG(PSTR("WiFiContext::startRescueAp OTA available at http://%s/update"), WiFi.softAPIP().toString().c_str());
}

void WiFiContext::stopRescueAp()
{
    if (!_rescueApActive)
        return;

    LOG(PSTR("WiFiContext::stopRescueAp home network is back"));
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    _rescueApActive = false;
}

/* Единственное место, где меняется конфигурация радио. Работает в таске
   loop(), раз в секунду - этого с запасом хватает для всех порогов. */
void WiFiContext::supervise()
{
    const uint32_t now = millis();

    if (_hasIp)
    {
        // Домашняя сеть вернулась - аварийный канал больше не нужен.
        stopRescueAp();

        /* Адрес есть, но брокер молчит слишком долго. Самая вероятная
           причина - статический IP прописан в подсети, которой больше нет:
           ассоциация при этом проходит, а маршрута нет. */
        if (_ipMode == IpModeStatic && wifiStaticIpGraceMs != 0
            && !_mqttContext.isConnected() && (now - _ipAcquiredMs) >= wifiStaticIpGraceMs)
        {
            LOG(PSTR("WiFiContext::supervise broker unreachable for %u ms on static ip, trying dhcp")
               ,(unsigned)(now - _ipAcquiredMs));
            switchIpMode(IpModeDhcp);
        }
        return;
    }

    /* Адреса нет совсем слишком долго - поднимаем аварийный канал.
       Проверяется ДО переключения режима и без выхода из функции: события
       DISCONNECTED приходят каждые ~2 с и сбрасывают счётчик попыток при
       каждом переключении, так что ветка ниже затыкала бы эту навсегда. */
    if (wifiRescueApAfterMs != 0 && (now - _lastIpMs) >= wifiRescueApAfterMs)
    {
        startRescueAp();
    }

    /* Ассоциация не удаётся. К режиму IP это напрямую не относится, но
       переключение покрывает и обратный случай - выключенный DHCP-сервер
       на роутере, когда рабочим оказывается статический адрес. */
    if (wifiMaxAttemptsPerMode != 0 && _attemptsInMode >= wifiMaxAttemptsPerMode)
    {
        LOG(PSTR("WiFiContext::supervise %u failed attempts in mode %s, switching")
           ,(unsigned)_attemptsInMode, describeIpMode(_ipMode));
        switchIpMode((_ipMode == IpModeStatic) ? IpModeDhcp : IpModeStatic);
    }
}

void WiFiContext::connectToWifi()
{
    LOG(PSTR("Connecting to Wi-Fi... (ip mode: %s)"), describeIpMode(_ipMode));
    WiFi.begin(_settings.ssid, _settings.pass);
    // Никакого delay(): функция работает колбэком программного таймера,
    // а таск службы таймеров один на все таймеры проекта - секундная
    // задержка здесь останавливала и опрос счётчика, и реконнект MQTT.
}

void WiFiContext::reconnectTimerTrampoline(TimerHandle_t timer)
{
    static_cast<WiFiContext *>(pvTimerGetTimerID(timer))->connectToWifi();
}

void WiFiContext::stopTimer()
{
    if (_wifiReconnectTimer == NULL)
        return;
    if (xTimerIsTimerActive(_wifiReconnectTimer) == pdTRUE)
    {
        LOG(PSTR("WiFiContext::stopTimer _wifiReconnectTimer"));
        xTimerStop(_wifiReconnectTimer, 0);
    }
}

void WiFiContext::startTimer()
{
    if (_wifiReconnectTimer == NULL)
        return;
    if (xTimerIsTimerActive(_wifiReconnectTimer) == pdFALSE)
    {
        LOG(PSTR("WiFiContext::startTimer _wifiReconnectTimer"));
        xTimerStart(_wifiReconnectTimer, 0);
    }
}

void WiFiContext::onWiFiEvent(WiFiEvent_t event)
{
    LOG(PSTR("WiFiContext::onWiFiEvent: %s"), WiFiContext::describeWiFiEvent(event));
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        LOG(PSTR("WiFiContext::onWiFiEvent WiFi connected, ip mode: %s"), describeIpMode(_ipMode));
        LOG(PSTR("WiFiContext::onWiFiEvent IP address: %s"), WiFi.localIP().toString().c_str());
        _hasIp = true;
        _attemptsInMode = 0;
        _lastIpMs = millis();
        _ipAcquiredMs = millis();
        stopTimer();
        _mqttContext.connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        _hasIp = false;
        _mqttContext.stopTimer();

        if (_attemptsInMode < 255)
        {
            _attemptsInMode++;
        }
        LOG(PSTR("WiFiContext::onWiFiEvent WiFi lost connection (attempt %u in mode %s)")
           ,(unsigned)_attemptsInMode, describeIpMode(_ipMode));
        startTimer();
        break;
    case SYSTEM_EVENT_WIFI_READY:
        LOG(PSTR("WiFiContext::onWiFiEvent WiFi interface ready"));
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        LOG(PSTR("WiFiContext::onWiFiEvent Completed scan for access points"));
        break;
    case SYSTEM_EVENT_STA_START:
        LOG(PSTR("WiFiContext::onWiFiEvent WiFi client started"));
        break;
    case SYSTEM_EVENT_STA_STOP:
        LOG(PSTR("WiFiContext::onWiFiEvent WiFi clients stopped"));
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        LOG(PSTR("WiFiContext::onWiFiEvent Connected to access point"));
        break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        LOG(PSTR("WiFiContext::onWiFiEvent Authentication mode of access point has changed"));
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        LOG(PSTR("WiFiContext::onWiFiEvent Lost IP address and IP address is reset to 0"));
        break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        LOG(PSTR("WiFiContext::onWiFiEvent WiFi Protected Setup (WPS): succeeded in enrollee mode"));
        break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        LOG(PSTR("WiFiContext::onWiFiEvent WiFi Protected Setup (WPS): failed in enrollee mode"));
        break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        LOG(PSTR("WiFiContext::onWiFiEvent WiFi Protected Setup (WPS): timeout in enrollee mode"));
        break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
        LOG(PSTR("WiFiContext::onWiFiEvent WiFi Protected Setup (WPS): pin code in enrollee mode"));
        break;
    case SYSTEM_EVENT_AP_START:
        LOG(PSTR("WiFiContext::onWiFiEvent WiFi access point started"));
        break;
    case SYSTEM_EVENT_AP_STOP:
        LOG(PSTR("WiFiContext::onWiFiEvent WiFi access point  stopped"));
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        LOG(PSTR("WiFiContext::onWiFiEvent Client connected"));
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        LOG(PSTR("WiFiContext::onWiFiEvent Client disconnected"));
        break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
        LOG(PSTR("WiFiContext::onWiFiEvent Assigned IP address to client"));
        break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
        LOG(PSTR("WiFiContext::onWiFiEvent Received probe request"));
        break;
    case SYSTEM_EVENT_GOT_IP6:
        LOG(PSTR("WiFiContext::onWiFiEvent IPv6 is preferred"));
        break;
    case SYSTEM_EVENT_ETH_START:
        LOG(PSTR("WiFiContext::onWiFiEvent Ethernet started"));
        break;
    case SYSTEM_EVENT_ETH_STOP:
        LOG(PSTR("WiFiContext::onWiFiEvent Ethernet stopped"));
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        LOG(PSTR("WiFiContext::onWiFiEvent Ethernet connected"));
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        LOG(PSTR("WiFiContext::onWiFiEvent Ethernet disconnected"));
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        LOG(PSTR("WiFiContext::onWiFiEvent Obtained IP address"));
        break;
    default:
        break;
    }
}


const char *WiFiContext::describeWiFiEvent(WiFiEvent_t event)
{
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        return PSTR("SYSTEM_EVENT_STA_GOT_IP");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        return PSTR("SYSTEM_EVENT_STA_DISCONNECTED");
        break;
    case SYSTEM_EVENT_WIFI_READY:
        return PSTR("SYSTEM_EVENT_WIFI_READY");
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        return PSTR("SYSTEM_EVENT_SCAN_DONE");
        break;
    case SYSTEM_EVENT_STA_START:
        return PSTR("SYSTEM_EVENT_STA_START");
        break;
    case SYSTEM_EVENT_STA_STOP:
        return PSTR("SYSTEM_EVENT_STA_STOP");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        return PSTR("SYSTEM_EVENT_STA_CONNECTED");
        break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        return PSTR("SYSTEM_EVENT_STA_AUTHMODE_CHANGE");
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        return PSTR("SYSTEM_EVENT_STA_LOST_IP");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        return PSTR("SYSTEM_EVENT_STA_WPS_ER_SUCCESS");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        return PSTR("SYSTEM_EVENT_STA_WPS_ER_FAILED");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        return PSTR("SYSTEM_EVENT_STA_WPS_ER_TIMEOUT");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
        return PSTR("SYSTEM_EVENT_STA_WPS_ER_PIN");
        break;
    case SYSTEM_EVENT_AP_START:
        return PSTR("SYSTEM_EVENT_AP_START");
        break;
    case SYSTEM_EVENT_AP_STOP:
        return PSTR("SYSTEM_EVENT_AP_STOP");
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        return PSTR("SYSTEM_EVENT_AP_STACONNECTED");
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        return PSTR("SYSTEM_EVENT_AP_STADISCONNECTED");
        break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
        return PSTR("SYSTEM_EVENT_AP_STAIPASSIGNED");
        break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
        return PSTR("SYSTEM_EVENT_AP_PROBEREQRECVED");
        break;
    case SYSTEM_EVENT_GOT_IP6:
        return PSTR("SYSTEM_EVENT_GOT_IP6");
        break;
    case SYSTEM_EVENT_ETH_START:
        return PSTR("SYSTEM_EVENT_ETH_START");
        break;
    case SYSTEM_EVENT_ETH_STOP:
        return PSTR("SYSTEM_EVENT_ETH_STOP");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        return PSTR("SYSTEM_EVENT_ETH_CONNECTED");
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        return PSTR("SYSTEM_EVENT_ETH_DISCONNECTED");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        return PSTR("SYSTEM_EVENT_ETH_GOT_IP");
        break;
    default:
        break;
    }
    return "";
}
