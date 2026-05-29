#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

// 1. Set your current version tag (Must match your GitHub Release Tag exactly, e.g., "v1.0")
const String CURRENT_VERSION = "v2.1"; 

// 2. Replace with your GitHub Username and Repository Name
const String GITHUB_USER = "bar513";
const String GITHUB_REPO = "example-wifi-git";

// This URL always points to the binary asset in your absolute latest release
const String firmwareUrl = "https://github.com/" + GITHUB_USER + "/" + GITHUB_REPO + "/releases/latest/download/firmware.bin";

void checkForUpdates() {
  Serial.println("Checking GitHub Releases for updates...");

  NetworkClientSecure client;
  client.setInsecure(); // Skips SSL cert management for simplicity

  HTTPClient http;
  
  // 1. 🚀 CHANGE: Tell the ESP32 to automatically follow GitHub's redirects
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); 
  
  http.begin(client, firmwareUrl);
  
  // 2. We send a HEAD request instead of GET. 
  // This checks if the file exists and gets its size without downloading the whole binary yet.
  int httpCode = http.sendRequest("HEAD"); 

  Serial.print("GitHub final response HTTP Code: "); Serial.println(httpCode);

  // HTTP 200 means the redirect was followed successfully and the file is sitting there ready
  if (httpCode == HTTP_CODE_OK) {
    int fileSize = http.getSize();
    Serial.printf("Success! Found latest firmware asset. Size: %d bytes\n", fileSize);
    
    // Since we are forcing a check only on reboot/reset, we assume if a published
    // release binary exists on GitHub, we want to flash it.
    Serial.println("Starting OTA Update execution...");
    
    // This line downloads, flashes, and auto-reboots the ESP32
    t_httpUpdate_return ret = httpUpdate.update(client, firmwareUrl);

    if (ret == HTTP_UPDATE_FAILED) {
      Serial.printf("OTA failed. Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    }
  } else {
    Serial.printf("Could not resolve latest release asset. Got HTTP Code: %d\n", httpCode);
  }
  
  http.end();
}

void setup() {
  Serial.begin(115200);

  WiFiManager wm;
  // Automatically connects using saved credentials, or boots an AP portal if none exist
  if (!wm.autoConnect("ESP32-ReleaseUpdate-Portal")) {
    Serial.println("Failed to connect to Wi-Fi. Rebooting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("\nConnected to Wi-Fi!");

  // Only checks once on boot/hardware reset
  checkForUpdates();

  Serial.println("Setup complete. Entering main loop application...");
}

void loop() {
  // Your actual application code runs here safely.
}