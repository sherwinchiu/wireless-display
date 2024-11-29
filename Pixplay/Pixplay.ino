#include "Arduino.h"
#include "WiFi.h"
#include "Preferences.h"
#include "html-file.h"
#include "DisplayDriver.h"
// For Webserver
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// For getting the fil, need to setup secure client
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>

// Server Name
String serverName = "https://pixplay-540390702710.us-central1.run.app/bmp";

// Update time for display set at 30min
#define uS_TO_S_FACTOR 1000000  
#define TIME_TO_SLEEP 600

// for access point mode
const char* ap_ssid = "PixPlay";
const char* ap_pass = "sherwinrocks123";

// to get the ssid and pass from server
String wifi_ssid = "";
String wifi_pass = "";

const char* PARAM_WIFI_SSID = "ssid";
const char* PARAM_WIFI_PASS = "pass";

// For remembering hostname/server
Preferences preferences;
// WiFi server
AsyncWebServer server(80);

#define uS_TO_S_FACTOR 60000000  /* Conversion factor for micro seconds to minutes */
#define TIME_TO_SLEEP  1 // minutes

void setup() {
  Serial.begin(115200);
  preferences.begin("creds", false);
  // check in cache for ssid and apss
  wifi_ssid = preferences.getString(PARAM_WIFI_SSID, "DEFAULT");
  wifi_pass = preferences.getString(PARAM_WIFI_PASS, "DEFAULT");
  Serial.printf("Stored SSID: %s\n", wifi_ssid);
  Serial.printf("Stored Pass: %s\n", wifi_pass);
  // Attempt to connect...
  WiFi.useStaticBuffers(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_pass);
  int currTime = millis();
  while (WiFi.status() != WL_CONNECTED) { // Check WiFi status every 500ms. If after 7.5 attempts, no connection 
    delay(500);
    Serial.print(".");
    if (millis() - currTime > 15000) { // Start WiFi in access point mode.
      // If it doesn't exist, run server in access point mode. Wait until "submit" button is pressed and valid
      // credentials is put in. Attempt to connect, if connection isn't possible keep waiting.
      Serial.println("\nNo WiFi connection could be made. Started AP Mode. Please connect to PixPlay on your phone.");
      WiFi.softAP(ap_ssid, ap_pass);
      IPAddress IP = WiFi.softAPIP();
      Serial.print("Go to any webbrowser and type the following address: ");
      Serial.println(IP); // Use this to connect to the webpage
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
      });
      // Get requests for the server to get the inputs.
      server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Request Received.");
        if (request->hasParam(PARAM_WIFI_SSID)) {
          wifi_ssid = request->getParam(PARAM_WIFI_SSID)->value();
          preferences.putString(PARAM_WIFI_SSID, wifi_ssid); 
          Serial.print("SSID: ");
          Serial.println(wifi_ssid);
        } 
        if (request->hasParam(PARAM_WIFI_PASS)) {
          wifi_pass = request->getParam(PARAM_WIFI_PASS)->value();
          preferences.putString(PARAM_WIFI_PASS, wifi_pass);
          Serial.print("Pass: ");
          Serial.println(wifi_pass);
        }
        request->send(200, "text/plain", "Request processed successfully");
        preferences.end();
        ESP.restart();
      }); 
      server.begin(); // Start the server
      Serial.println("Server Started");
      while(true){}
    }
  }
  // Check in Cache if any WiFi settings already exist
  // If they already exist, attempt to connect. If connection successful, skip to loop. 
  // If connection is unsucessful, skip to next step.
  Serial.println();
  Serial.println("I'm connected to WiFi!");

  // Display code & Fetch Image
  // Setup TFT
  setupTFT();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //
}

void loop() {
  HTTPClient http;
  http.begin(serverName);
  int httpCode = http.GET();
  if (httpCode > 0) {
    WiFiClient* stream = http.getStreamPtr();
    size_t contentLength = http.getSize();

    // Buffer for one line of pixels (240 pixels × 2 bytes per pixel)
    const size_t lineWidth = 240 * 3; // 480 bytes
    uint16_t lineBuffer[240];

    size_t bytesRead = 0;
    size_t totalBytesRead = 0;
    int y = 0;
    size_t skippedBytes = 0;
    size_t headerBytes = 56;
    while (skippedBytes < headerBytes) {
        stream->read();  // Read and discard one byte at a time
        skippedBytes++;
    }
    tft.startWrite(); // Optimize for batch rendering

    // Read the image data line by line
    while (totalBytesRead < contentLength && y < 320) {
        // Read one line of data
        bytesRead = stream->readBytes((char*)lineBuffer, lineWidth);

        if (bytesRead > 0) {
            // Ensure that we read the full line (if bytesRead is less than expected, we may have to wait)
            while (bytesRead < lineWidth) {
                delay(1);  // Wait a bit before attempting to read more bytes
                bytesRead += stream->readBytes((char*)lineBuffer + bytesRead, lineWidth - bytesRead);
            }

            // Set the window for the current line
            tft.setAddrWindow(0, y, 239, y);

            // Write the pixel data directly to the display
            tft.writePixels(lineBuffer, 240);

            totalBytesRead += bytesRead;
            y++;

            // Delay a little before reading the next line, to slow things down
            // delay(30); // Adjust delay to slow down the display process
        } else {
            Serial.println("Stream read failed or no more data!");
            break;
        }
    }
    
    WiFi.disconnect();
    // esp_deep_sleep_start();
    delay(1000000);
    ESP.restart();
    tft.endWrite(); // End batch rendering
  
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  Serial.println("no crash here");
  
  http.end();

}
