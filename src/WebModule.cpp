#include "WebModule.h"
#include "WebPage.h"
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#else
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_system.h>
#endif
#if __has_include("NetworkConfig.h")
#include "NetworkConfig.h"
#else
#include "NetworkConfig.example.h"
#endif

namespace {
String numberOrNull(float value) { return isfinite(value) ? String(value, 2) : String("null"); }
}

bool WebModule::begin() {
    configured_ = strlen(TERMO_WIFI_SSID) > 0;
    if (!configured_) {
        Serial.println("Wi-Fi not configured: copy include/NetworkConfig.example.h to NetworkConfig.h.");
        return true;
    }
#if defined(ESP8266)
    controlToken_ = String(ESP.random(), HEX) + String(ESP.random(), HEX);
#else
    controlToken_ = String(esp_random(), HEX) + String(esp_random(), HEX);
#endif
#if defined(ESP8266)
    server_.collectHeaders("X-Termo-Control");
#else
    const char* headers[] = {"X-Termo-Control"};
    server_.collectHeaders(headers, 1);
#endif
    server_.on("/", HTTP_GET, [this]() {
        server_.sendHeader("Cache-Control", "no-store");
        server_.send_P(200, "text/html; charset=utf-8", webPage);
    });
    server_.on("/api/status", HTTP_GET, [this]() { status(); });
    server_.on("/api/average", HTTP_POST, [this]() {
        if (!validMutation()) return;
        const String value = server_.arg("minutes");
        bool digits = value.length() > 0 && value.length() <= 2;
        for (size_t i = 0; i < value.length(); ++i) digits = digits && value[i] >= '0' && value[i] <= '9';
        const int minutes = value.toInt();
        if (!digits || minutes < 1 || minutes > 60) {
            server_.send(400, "text/plain; charset=utf-8", "Период должен быть целым числом от 1 до 60");
            return;
        }
        if (!average_.saveMinutes(static_cast<uint8_t>(minutes))) {
            server_.send(500, "text/plain; charset=utf-8", "Не удалось сохранить период");
            return;
        }
        status();
    });
    server_.on("/api/pump", HTTP_POST, [this]() {
        if (!validMutation()) return;
        const String value = server_.arg("on");
        if (value != "0" && value != "1") {
            server_.send(400, "text/plain", "Expected on=0 or on=1");
            return;
        }
        pump_.setOn(value == "1");
        status();
    });
    server_.onNotFound([this]() { server_.send(404, "text/plain", "Not found"); });
    WiFi.persistent(false);
#if !defined(ESP8266)
    WiFi.setHostname(TERMO_HOSTNAME);
#endif
    WiFi.mode(WIFI_STA);
#if defined(ESP8266)
    WiFi.hostname(TERMO_HOSTNAME);
#endif
    WiFi.setAutoReconnect(true);
    WiFi.begin(TERMO_WIFI_SSID, TERMO_WIFI_PASSWORD);
    lastReconnect_ = millis();
    Serial.println("Connecting to Wi-Fi...");
    return true;
}

bool WebModule::validMutation() {
    // Custom header prevents other websites submitting control forms (no CORS).
    if (server_.header("X-Termo-Control") != controlToken_) {
        server_.send(403, "text/plain", "Refresh the device page before controlling it");
        return false;
    }
    return true;
}

void WebModule::status() {
    String json;
    json.reserve(240);
    json = "{\"temperature\":" + numberOrNull(sensor_.temperature());
    json += ",\"average\":" + numberOrNull(average_.temperature());
    json += ",\"minutes\":" + String(average_.minutes());
    json += ",\"pumpOn\":" + String(pump_.isOn() ? "true" : "false");
    json += ",\"ip\":\"" + WiFi.localIP().toString();
    json += "\",\"controlToken\":\"" + controlToken_ + "\"}";
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(200, "application/json", json);
}

void WebModule::update() {
    if (!configured_) return;
    const bool connected = WiFi.status() == WL_CONNECTED;
    if (connected != connected_) {
        connected_ = connected;
        if (connected) {
            if (MDNS.begin(TERMO_HOSTNAME)) MDNS.addService("http", "tcp", 80);
            server_.begin();
            Serial.printf("Web interface: http://%s/ (http://%s.local/)\n", WiFi.localIP().toString().c_str(), TERMO_HOSTNAME);
            Serial.printf("Wi-Fi MAC: %s\n", WiFi.macAddress().c_str());
        } else {
            server_.stop();
            MDNS.end();
            Serial.println("Wi-Fi disconnected; reconnecting.");
        }
    }
    if (connected) {
        server_.handleClient();
#if defined(ESP8266)
        MDNS.update();
#endif
    }
    else if (uint32_t(millis() - lastReconnect_) >= 15000) {
        lastReconnect_ = millis();
        WiFi.reconnect();
    }
}
