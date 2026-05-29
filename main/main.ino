#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

// 1. Set your current version tag (Must match your GitHub Release Tag exactly, e.g., "v1.0")
const String CURRENT_VERSION = "v3.0"; 

// 2. Replace with your GitHub Username and Repository Name
const String GITHUB_USER = "bar513";
const String GITHUB_REPO = "example-wifi-git";

// This URL always points to the binary asset in your absolute latest release
const String firmwareUrl = "https://github.com/" + GITHUB_USER + "/" + GITHUB_REPO + "/releases/latest/download/firmware.bin";

void checkForUpdates() {
  Serial.println("current version" + CURRENT_VERSION);
  Serial.println("Checking GitHub Releases for updates...");

  NetworkClientSecure client;
  client.setInsecure(); 

  HTTPClient http;
  
  // 1. Tell the ESP32 NOT to follow the redirect automatically so we can read the header
  http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS); 
  
  // 2. Tell the ESP32 to collect the Location header
  const char* headerKeys[] = {"Location"};
  http.collectHeaders(headerKeys, 1);
  
  http.begin(client, firmwareUrl);
  int httpCode = http.GET();
  
  String locationHeader = http.header("Location");
  http.end();

  Serial.print("GitHub responded with HTTP Code: "); Serial.println(httpCode);

  if (httpCode == 302 && locationHeader.length() > 0) {
    Serial.print("Redirect URL found: "); Serial.println(locationHeader);
    
    // 🔧 FIX: Hunt for "/download/" instead of "/tags/"
    int tagIndex = locationHeader.indexOf("/download/");
    if (tagIndex != -1) {
      // Cut out the version tag string (skipping past "/download/")
      String latestVersion = locationHeader.substring(tagIndex + 10);
      // Cut off everything after the version tag (the trailing "/firmware.bin")
      latestVersion = latestVersion.substring(0, latestVersion.indexOf("/"));
      
      Serial.print("Current firmware version: "); Serial.println(CURRENT_VERSION);
      Serial.print("Latest release on GitHub: "); Serial.println(latestVersion);

      if (latestVersion != CURRENT_VERSION) {
        Serial.println("New release detected! Upgrading firmware now...");
        
        // 🔧 1. Set a higher timeout (30 seconds) so it doesn't drop mid-stream
        client.setTimeout(30); 

        // 🔧 2. Configure strict redirect following for the update engine
        httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        
        // Execute the update using the updated client settings
        t_httpUpdate_return ret = httpUpdate.update(client, firmwareUrl);

        if (ret == HTTP_UPDATE_FAILED) {
          Serial.printf("OTA failed. Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        }
      } else {
        Serial.println("Your ESP32 is already running the latest release.");
      }
    } else {
      Serial.println("[Error] Could not locate version identifier component in redirect string.");
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