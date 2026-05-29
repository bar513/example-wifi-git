#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

// 1. Set your current version tag (Must match your GitHub Release Tag exactly, e.g., "v1.0")
const String CURRENT_VERSION = "v2.0"; 

// 2. Replace with your GitHub Username and Repository Name
const String GITHUB_USER = "bar513";
const String GITHUB_REPO = "example-wifi-git";

// This URL always points to the binary asset in your absolute latest release
const String firmwareUrl = "https://github.com/" + GITHUB_USER + "/" + GITHUB_REPO + "/releases/latest/download/firmware.bin";

void checkForUpdates() {
  Serial.println("Current version: " + CURRENT_VERSION);
  Serial.println("Checking GitHub Releases for updates...");

  NetworkClientSecure client;
  client.setInsecure(); // Keeps code simple by skipping complex SSL cert management

  HTTPClient http;
  
  // GitHub uses 302 redirects for its 'latest' shortcut link. 
  // We handle the link via HTTP to inspect where it points.
  http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS); 
  http.begin(client, firmwareUrl);
  
  int httpCode = http.GET();
  String locationHeader = http.header("Location");
  http.end();

  // If there's a redirect, the target URL contains the latest version tag (e.g., .../tags/v2.0/firmware.bin)
  if (httpCode == 302 && locationHeader.length() > 0) {
    
    // Quick logic to extract the version tag from the redirect URL
    int tagIndex = locationHeader.indexOf("/tags/");
    if (tagIndex != -1) {
      String latestVersion = locationHeader.substring(tagIndex + 6);
      latestVersion = latestVersion.substring(0, latestVersion.indexOf("/"));
      
      Serial.print("Current firmware version: "); Serial.println(CURRENT_VERSION);
      Serial.print("Latest release on GitHub: "); Serial.println(latestVersion);

      if (latestVersion != CURRENT_VERSION) {
        Serial.println("New release detected! Upgrading...");
        
        // This execution allows the library to follow redirects to actually download the file
        t_httpUpdate_return ret = httpUpdate.update(client, firmwareUrl);

        if (ret == HTTP_UPDATE_FAILED) {
          Serial.printf("OTA failed. Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        }
      } else {
        Serial.println("Your ESP32 is already running the latest release.");
      }
    }
  } else {
    Serial.printf("Could not resolve latest release. HTTP Code: %d\n", httpCode);
  }
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