#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>

const String GITHUB_USER = "bar513";
const String GITHUB_REPO = "example-wifi-git";

const String firmwareUrl =
  "https://github.com/" + GITHUB_USER + "/" +
  GITHUB_REPO +
  "/releases/latest/download/firmware.bin";

void checkForUpdates() {
  Serial.println("Checking for firmware update...");

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  if (!http.begin(client, firmwareUrl)) {
    Serial.println("HTTP begin failed");
    return;
  }

  int httpCode = http.GET();

  Serial.print("HTTP Code: ");
  Serial.println(httpCode);

  if (httpCode == HTTP_CODE_OK) {

    int contentLength = http.getSize();

    Serial.print("Firmware Size: ");
    Serial.println(contentLength);

    http.end();

    Serial.println("Starting OTA update...");

    t_httpUpdate_return ret =
      httpUpdate.update(client, firmwareUrl);

    switch (ret) {

      case HTTP_UPDATE_FAILED:
        Serial.printf(
          "Update failed Error (%d): %s\n",
          httpUpdate.getLastError(),
          httpUpdate.getLastErrorString().c_str()
        );
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("No updates.");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("Update successful.");
        break;
    }

  } else {
    Serial.printf("HTTP failed, code: %d\n", httpCode);
    http.end();
  }
}

void setup() {
  Serial.begin(115200);

  WiFiManager wm;

  if (!wm.autoConnect("ESP32-OTA")) {
    Serial.println("WiFi failed");
    ESP.restart();
  }

  Serial.println("Connected!");

  checkForUpdates();
}

void loop() {
}