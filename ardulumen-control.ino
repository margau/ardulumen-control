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

// General declarations

#define VERSION "0.0.1-dev"

WebServer server(80);

void initButtons() {
  pinMode(pin_rows[0],OUTPUT_OPEN_DRAIN);
  pinMode(pin_rows[1],OUTPUT_OPEN_DRAIN);
  pinMode(pin_rows[2],OUTPUT_OPEN_DRAIN);
  digitalWrite(pin_rows[0], HIGH);
  digitalWrite(pin_rows[1], HIGH);
  digitalWrite(pin_rows[2], HIGH);
  pinMode(pin_cols[0],INPUT_PULLUP);
  pinMode(pin_cols[1],INPUT_PULLUP);
  pinMode(pin_cols[2],INPUT_PULLUP);
}

void initDisplay() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
  }
  
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("ardulumen"));
  display.setTextSize(1);
  display.setCursor(40, 25);
  display.println(VERSION);
  display.display();      // Show initial text
}

void displayHome() {
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println("LIVE MODE");
  v_current = HOME;
}
void displayEffect(String s) {
  Serial.println("Display Effect "+s);
  display.setTextSize(2); // Draw 2X-scale text
  display.fillRect(0, 32, display.width(), 32, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); 
  display.setCursor(10, 36);
  display.println(s);
}

void buttonClick(char i) {
  input = i;
  Serial.print("Button pressed: ");
  Serial.println(input, HEX);
}

void handleInputs() {
  // First: Serial inputs
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();
    if(incomingByte >= '0' && incomingByte <= '9') {
      incomingByte -= 48;
      buttonClick(incomingByte);
    }
  }
  if(now>(last_col_time+2)) {
    if(col_read) {
      for(int i=0;i<3;i++) {
        boolean r = (digitalRead(pin_cols[i])==0);
        if(r!=button_state[i+col*3]) {
          button_state[i+col*3] = r;
          if(r) {
            short clicked = i+col*3+1;
            Serial.println(clicked);
            buttonClick(clicked);
          }
        }
      }    
      digitalWrite(pin_rows[col],HIGH);
      col_read = false;
     } else {
      if(col>=COLS-1) {
        col = 0;
      } else {
        col++;
      }
      col_read = true;
      digitalWrite(pin_rows[col],LOW);
    }
    last_col_time=now;
  }
}

void handleRoot() {
  server.send(200, "text/plain", "ardulumen-control");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleLEDconfig() {
  StaticJsonDocument<400> response;
  response["instance"] = 0;
  response["serial"] = 0;
  JsonArray effects = response.createNestedArray("effects");
  JsonObject e1 = effects.createNestedObject();
  e1["type"] = "fill";
  e1["color"] = 0xFF0000;
  JsonObject e2 = effects.createNestedObject();
  e2["type"] = "sine";
  e2["w"] = 25;
  e2["p"] = 2000;
  JsonObject e3 = effects.createNestedObject();
  e3["type"] = "pix";
  e3["color"] = 0xFF0000;
  e3["f"] = 200;
  e3["c"] = 1;
  char temp[400];
  serializeJson(response,temp);
  server.send(200, "application/json",temp);
}

void setup(void) {
  Serial.begin(115200);
  Serial.print("ardulumen-control v");
  Serial.println(VERSION);
  // IO Stuff
  initButtons();
  // Initialize Preferences
  // prefs.begin("ardulumen");
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
    displayHome();
    char effectname [10];
    switch(input) {
      case 0:
      displayEffect(F("BLACKOUT"));
      break;
      case 1 ... 9:
      snprintf(effectname, 10, "EFFECT %d",input);
      displayEffect(effectname);
      break;
    }
    v_effect_display = now;
    input = -1;
  }
  display.display();
  server.handleClient();
}
