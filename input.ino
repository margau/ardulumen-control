void initButtons() {
  for(int i=0; i<ROWS; i++) {
    pinMode(pin_rows[i],OUTPUT_OPEN_DRAIN);
    digitalWrite(pin_rows[i], HIGH);
  }
  for(int i=0; i<FADE; i++) {
    pinMode(pin_fade[i],ANALOG);
  }
  for(int i=0; i<COLS; i++) {
    pinMode(pin_cols[i],INPUT_PULLUP);
  }
  row = 0;
  row_read = true;
  digitalWrite(pin_rows[row],LOW);
  analogReadResolution(10);
}

void buttonClick(char i) {
  input = i;
  Serial.print("Button pressed: ");
  Serial.println(input, HEX);
}
void handleFade() {
  if(fade_multi_pointer >= FADE_MULTI) {
    fade_multi_pointer = 0;
  }
  for(int i=0; i<FADE; i++) {
    // Do Multisampling
    fade_multi[i][fade_multi_pointer] = analogRead(pin_fade[i]);
    uint32_t temp = 0;
    for(int j=0; j<FADE_MULTI; j++) {
      temp+= fade_multi[i][j];
    }
    fade_val[i] = temp/FADE_MULTI;
    fade_val_8[i] = map(fade_val[i],0,FADE_MAX,0,255);
  }
  fade_multi_pointer++;
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
  if(now_micros>(last_row_time+10)) {
    if(row_read) {
      // Read out all columns of row
      for(int col=0;col<COLS;col++) {
        boolean r = (digitalRead(pin_cols[col])==0);
        uint8_t num_button = col+row*COLS;
        if(r!=button_state[num_button]) {
          button_state[num_button] = r;
          if(r) {
            buttonClick(num_button+1);
          }
        }
      }    
      digitalWrite(pin_rows[row],HIGH);
      row_read = false;
     } else {
      if(row>=ROWS-1) {
        row = 0;
      } else {
        row++;
      }
      row_read = true;
      digitalWrite(pin_rows[row],LOW);
    }
    last_row_time=now_micros;
  }
}

void runEffect(int input) {
  char effectname [10];
  composeClear();
  snprintf(effectname, 10, "EFFECT %d",input);
  e_active = input;
  popUp(effectname);
  notify();
}

void buttonClick(int input) {
    Serial.printf("Button %d\n",input);
    switch(input) {
      case 0:
      popUp(F("BLACKOUT"));
      break;
      case 1 ... 9:
        if(compose_save_step!=2) {
          runEffect(input);
        } else {
          compose_save_num = input;  
          compose_save_step = 3;
          composeSave();
        }
      break;
      case 10:
      popUp("CLEAR");
      if(e_compose) {
        composeClear();
        compose_save_step = 0;
      }
      break;
      case 11:
      if(e_compose) {
        switch(compose_save_step) {
          case 0:
          compose_save_step = 1;
          composeSave();
          break;
          case 3:
          compose_save_step = 4;
          composeSave();
          break;          
        }
      }
      break;      
      case 12:
      popUp("PARAM");
      if(e_compose) {
        composeParam();
      }
      break;
      case 13 ... 21:
        composeButton(input);
      break;
    }
}
