StaticJsonDocument<512> compose_json;
JsonArray compose_effects;
boolean compose_stack_new = false;
enum c_effect_type {
  FILL,
  SINE
};
c_effect_type compose_stack_type;
void composeClear() {
  Serial.println("Leave Composing Mode");
  e_compose = false;
  v_current = HOME;
  notify();
}
void composeButton(int i) {
  if(!e_compose) {
    // Init Compose Mode
    e_compose = true;
    v_current = COMPOSE;
    Serial.println("Switch to Composing Mode");
    // Prepare JSON stuff
    compose_json.clear();
    compose_stack = 0;
    compose_effects = compose_json.createNestedArray("effects");
  }
  compose_stack++;
  compose_stack_new = true;
  switch(i) {
    case 13:
    popUp("Fill");
    compose_fader_text[0] = "RED";
    compose_fader_text[1] = "GREEN";
    compose_fader_text[2] = "BLUE";
    compose_fader_text[3] = "MAST";
    compose_stack_type = FILL;
    break;
    case 16:
    popUp("Sine");
    compose_fader_text[0] = "WIDTH";
    compose_fader_text[1] = "PERIOD";
    compose_fader_text[2] = "";
    compose_fader_text[3] = "";
    compose_stack_type = SINE;
    break;
  }
  compose_fader_val[0] = 0;
  compose_fader_val[1] = 0;
  compose_fader_val[2] = 0;
  compose_fader_val[3] = 0;
  Serial.printf("Compose Button %d\n",i);
  composeWorker();  
}
void composeWorker() {
  JsonObject e;
  if(compose_stack_new) {
    // This is a new element, create it
    e = compose_effects.createNestedObject();
    compose_stack_new = false;
  } else {
    // Get Existing JsonObject for this
    e = compose_effects[compose_stack-1];
  }
  // Only Touch the effect on top
  switch(compose_stack_type) {
    case FILL:
    composeFill(e);
    break;
    case SINE:
    composeSine(e);
    break;
  }
  
  compose_last_handle = now;
  // Output
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

/*
 * Implementation per Effect
 */

void composeFill(JsonObject &e) {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  r=map(fade_val[0],0,FADE_MAX,0,255);g=map(fade_val[1],0,FADE_MAX,0,255);b=map(fade_val[2],0,FADE_MAX,0,255);
  compose_fader_val[0] = r; compose_fader_val[1] = g; compose_fader_val[2] = b;
     
  e["type"] = "fill";
  e["color"] = (r<<16)+(g<<8)+b;
}

void composeSine(JsonObject &e) {
  uint16_t p = 0;
  uint8_t w = 0;
  w=map(fade_val[0],0,FADE_MAX,0,255);p=map(fade_val[1],0,FADE_MAX,0,10000);
  compose_fader_val[0] = w; compose_fader_val[1] = p;
     
  e["type"] = "sine";
  e["w"] = w;
  e["p"] = p;
}
