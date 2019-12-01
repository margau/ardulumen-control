StaticJsonDocument<512> compose_json;
void composeClear() {
  e_compose = false;
  v_current = HOME;
}
void composeButton(int i) {
  if(!e_compose) {
    e_compose = true;
    v_current = COMPOSE;
    Serial.println("Switch to Composing Mode");
  }
  compose_stack++;
  switch(i) {
    case 13:
    popUp("Fill");
    compose_fader_text[0] = "RED";
    compose_fader_text[1] = "GREEN";
    compose_fader_text[2] = "BLUE";
    compose_fader_text[3] = "MAST";
    break;
  }
  Serial.printf("Compose Button %d\n",i);
  composeWorker();  
}
void composeWorker() {
  compose_json.clear();
  JsonArray effects = compose_json.createNestedArray("effects");
  JsonObject e = effects.createNestedObject();
  // For now assume a static fill effect, ToDo: Loop over effect stack
  composeFill(e);
  compose_last_handle = now;
  // Output
  e_serial++;
  e_changed = true;
  notify();
}
// Runtime stuff, e.g. adjusting colors
void composeHandle() {
  if(e_compose) {
    if(now>(compose_last_handle+COMPOSE_INT)) {
      composeWorker();
    }
  }
}
void composeFill(JsonObject &e) {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  r=map(fade_val[0],0,4095,0,255);g=map(fade_val[1],0,4095,0,255);b=map(fade_val[2],0,4095,0,255);
  compose_fader_val[0] = r; compose_fader_val[1] = g; compose_fader_val[2] = b;
     
  e["type"] = "fill";
  e["color"] = (r<<16)+(g<<8)+b;
}
