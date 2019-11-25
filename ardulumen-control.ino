/*
 * ardulumen-control - LED Control for ESP32
 * Created by autinerd and margau
 */

// Wifi-Stuff

#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoOTA.h>

// Display-stuff
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

enum v_menu {
  BOOT,
  HOME,
  PROGRAMM,
  DETAILS
};

v_menu v_current = BOOT;
int input = -1;

#define BOOTSCREEN_DUR 2000
#define EFFECT_DISPLAY_DUR 2000

// Buttons

#define COLS 3
#define ROWS 4
char pin_rows[] = {14, 12, 13, 32};
char pin_cols[] = {25, 26, 27};
uint8_t row = 0;
boolean row_read = false;
boolean button_state[COLS*ROWS];
unsigned long last_row_time = 0;

// Timer
unsigned long now = 0;
unsigned long v_effect_display = 0;
unsigned long udp_int = 0;

// Effects

char e_active = 0;
unsigned long e_serial = 0;
boolean e_changed = true;

// UDP Stuff

const char * udpAddress = "192.168.4.255";
const int udpPort = 3333;
#define UDP_INT 1000

// Generic Send Stuff
char resJSON[400];
size_t resJSONlen;

// General declarations

#define VERSION "0.0.1-dev"

WebServer server(80);
WiFiUDP udp;

void setup(void) {
  Serial.begin(115200);
  Serial.print("ardulumen-control v");
  Serial.println(VERSION);
  // IO Stuff
  initButtons();
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
  // Initialize WiFi AP
  char apName[30] = "ardulumen";
  // prefs.getString("apName","ardulumen").toCharArray(apName, 50);
  Serial.print("Create AP with SSID "); Serial.println(apName);  Serial.println("");
  WiFi.softAP(apName);
  
  if (MDNS.begin("ardulumen")) {
    Serial.println("MDNS responder started");
  }
  // Effects
  setupJSON();
  notify();
  
  // HTTP Routes
  server.on("/", handleRoot);
  server.on("/led", handleLEDconfig);
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  // User-Interface
  
  initDisplay();
  Serial.println("User Interface initialized");

  //OTA

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("OTA: Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("OTA: \nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("OTA: Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("OTA: Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("OTA: Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("OTA: Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA: Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("OTA: End Failed");
    });

  ArduinoOTA.begin();
  udp.begin(WiFi.localIP(),udpPort);
}

void loop(void) {
  now = millis();
  // Handle button press stuff
  handleInputs();
  // View Switching logic
  if(v_current==BOOT) {
    if(now > BOOTSCREEN_DUR) {
      displayHome();
    }
  }
  if(now > (v_effect_display + EFFECT_DISPLAY_DUR)) {
    displayHome();
  }
  // Home/Live
  if(input >=0) {
    displayEffect(input);
    input = -1;
  }
  // Periodical UDP send
  if(now > (udp_int + UDP_INT)) {
    udp_int = now;
    sendUDP();
  }
  display.display();
  server.handleClient();
  ArduinoOTA.handle();
}
