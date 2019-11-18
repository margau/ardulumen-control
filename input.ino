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
