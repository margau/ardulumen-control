void initButtons() {
  for(int i=0; i<ROWS; i++) {
    pinMode(pin_rows[i],OUTPUT_OPEN_DRAIN);
    digitalWrite(pin_rows[i], HIGH);
  }
  for(int i=0; i<COLS; i++) {
    pinMode(pin_cols[i],INPUT_PULLUP);
  }
  row = 0;
  row_read = true;
  digitalWrite(pin_rows[row],LOW);
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
  if(now>(last_row_time+2)) {
    if(row_read) {
      // Read out all columns of row
      for(int col=0;col<COLS;col++) {
        boolean r = (digitalRead(pin_cols[col])==0);
        uint8_t num_button = col+row*COLS;
        if(r!=button_state[num_button]) {
          button_state[num_button] = r;
          if(r) {
            Serial.println(num_button+1);
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
    last_row_time=now;
  }
}
