#include "Arduino.h"
#include "WiFi.h"
#include "Preferences.h"
#include "html-file.h"
// For Webserver
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


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
    if (millis() - currTime > 7500) { // Start WiFi in access point mode.
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
      break;
    }
  }
  // Check in Cache if any WiFi settings already exist
  // If they already exist, attempt to connect. If connection successful, skip to loop. 
  // If connection is unsucessful, skip to next step.
  Serial.println();
  Serial.println("I'm connected to WiFi!");
}

void loop() {
  // put your main code here, to run repeatedly:
  // ok i finished coding everything above
}
