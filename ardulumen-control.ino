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
char pin_rows[] = {33, 25, 26};
char pin_cols[] = {27, 14, 12};
char col = 0;
boolean col_read = false;
boolean button_state[9];
unsigned long last_col_time = 0;

// Timer
unsigned long now = 0;
unsigned long v_effect_display = 0;

// Effects

char e_active = 0;
boolean e_changed = true;

// General declarations

#define VERSION "0.0.1-dev"

WebServer server(80);

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

  // HTTP Routes
  server.on("/", handleRoot);
  server.on("/led", handleLEDconfig);
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  // User-Interface
  
  initDisplay();
  Serial.println("User Interface initialized");
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
  display.display();
  server.handleClient();
}
