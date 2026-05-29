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
  
  // 1. Tell the ESP32 NOT to follow the redirect automatically...
  http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS); 
  
  // 2. ⚠️ CRUCIAL FIX: Tell the ESP32 to collect and remember the Location header!
  const char* headerKeys[] = {"Location"};
  http.collectHeaders(headerKeys, 1);
  
  http.begin(client, firmwareUrl);
  int httpCode = http.GET();
  
  // 3. Read where GitHub wants to redirect us
  String locationHeader = http.header("Location");
  http.end();

  Serial.print("GitHub responded with HTTP Code: "); Serial.println(httpCode);

  // If it's a 302 Redirect, we successfully found the asset path!
  if (httpCode == 302 && locationHeader.length() > 0) {
    Serial.print("Redirect URL found: "); Serial.println(locationHeader);
    
    // Extract the version tag (e.g., v2.0) from the redirect URL
    int tagIndex = locationHeader.indexOf("/tags/");
    if (tagIndex != -1) {
      String latestVersion = locationHeader.substring(tagIndex + 6);
      latestVersion = latestVersion.substring(0, latestVersion.indexOf("/"));
      
      Serial.print("Current firmware version: "); Serial.println(CURRENT_VERSION);
      Serial.print("Latest release on GitHub: "); Serial.println(latestVersion);

      if (latestVersion != CURRENT_VERSION) {
        Serial.println("New release detected! Upgrading firmware now...");
        
        // This execution automatically follows the redirect chain to download and flash the file
        t_httpUpdate_return ret = httpUpdate.update(client, firmwareUrl);

        if (ret == HTTP_UPDATE_FAILED) {
          Serial.printf("OTA failed. Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        }
      } else {
        Serial.println("Your ESP32 is already running the latest release.");
      }
    }
  } else {
    Serial.printf("Could not resolve latest release. Expected 302, got HTTP Code: %d\n", httpCode);
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