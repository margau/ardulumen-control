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

void displayLoop() {
  // View Switching logic
  if(v_current==BOOT) {
    if(now > BOOTSCREEN_DUR) {
      v_current=HOME;
      d_changed = true;
    }
  }
  // Display normal without "popup"
  if(d_popup && (now > (v_popup_display + EFFECT_DISPLAY_DUR))) {
    d_popup = false;
    d_changed = true;
  }

  // Build new view if necessary
  // Differntiate between views
  if(d_changed) {
    display.clearDisplay();
    switch(v_current) {
      case HOME:
      displayHome();
      break;
      case COMPOSE:
        display.setTextSize(2); // Draw 2X-scale text
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(10, 0);
        display.println("COMPOSE");
        display.setTextSize(1); // Draw 2X-scale text
        display.setCursor(10, 20);
        display.printf("Stack: %d ",compose_stack);
        // Output Fader stuff 0
        display.setCursor(0,44);
        display.print(compose_fader_text[0]);
        display.setCursor(0,54);
        display.print(compose_fader_val[0]);
        // Output Fader stuff 1
        display.setCursor(32,44);
        display.print(compose_fader_text[1]);
        display.setCursor(32,54);
        display.print(compose_fader_val[1]);
        // Output Fader stuff 2
        display.setCursor(64,44);
        display.print(compose_fader_text[2]);
        display.setCursor(64,54);
        display.print(compose_fader_val[2]);
        // Output Fader stuff 3
        display.setCursor(96,44);
        display.print(compose_fader_text[3]);
        display.setCursor(96,54);
        display.print(compose_fader_val[3]);
      break;
    }

    //Overlay if applicable
    if(d_popup) {
      Serial.println("PopUp: "+d_popup_string);
      display.setTextSize(2); // Draw 2X-scale text
      display.fillRect(0, 32, display.width(), 32, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); 
      display.setCursor(10, 36);
      display.println(d_popup_string);
    }

    display.display();
    Serial.println("refresh display");
    d_changed = false;
  }
}
void displayHome() {
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println("LIVE MODE");
  display.setTextSize(1); // Draw 2X-scale text
  display.setCursor(10, 20);
  display.printf("Effect.: %d ",e_active);
}
void popUp(String s) {
  d_popup_string = s;
  d_popup = true;
  d_changed = true;
  v_popup_display = now;
}
