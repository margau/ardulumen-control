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
void printEffect(String s) {
  
  Serial.println("Display Effect "+s);
  display.setTextSize(2); // Draw 2X-scale text
  display.fillRect(0, 32, display.width(), 32, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); 
  display.setCursor(10, 36);
  display.println(s);
}
void displayEffect(int input) {
    displayHome();
    char effectname [10];
    switch(input) {
      case 0:
      printEffect(F("BLACKOUT"));
      break;
      case 1 ... 9:
        snprintf(effectname, 10, "EFFECT %d",input);
        printEffect(effectname);
        e_active = input;
        e_serial++;
        e_changed = true;
        notify();
      break;
      case 10:
      printEffect("DELETE");
      break;
      case 11:
      printEffect("STORE");
      break;      
      case 12:
      printEffect("PARAM");
      break;
    }
    v_effect_display = now;
}
