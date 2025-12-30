#include "network.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "pin_config.h"




void NETW::Init() {
    Serial.begin(115200);
    // Init WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.begin(115200);
        Serial.print(".");
    }
    Serial.println("\nConnesso!");
    Serial.println(WiFi.localIP());
}

bool NETW::SignalCompletion() {
    char url[255]; url[0] = 0;
    strcat(url,API_BASE_URL);
    strcat(url,"/spagdone");
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
        Serial.print("HTTP code: "); // should be 203
        Serial.println(httpCode);
        http.end();
        return true;
    } else {
        Serial.print("Errore HTTP: ");
        Serial.println(http.errorToString(httpCode));
        http.end();
        return false;
    }
}