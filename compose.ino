StaticJsonDocument<512> compose_json;
JsonArray compose_effects;
boolean compose_stack_new = false;
uint32_t compose_hash = 0;
uint32_t compose_hash_temp = 0;
enum c_effect_type {
  FILL,
  SINE,
  PIX
};
c_effect_type compose_stack_type;
uint8_t compose_param = 0;
boolean compose_param_change = 0;
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
  compose_param = 0;
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
    case 19:
    popUp("Fading Pix");
    compose_fader_text[0] = "RED";
    compose_fader_text[1] = "GREEN";
    compose_fader_text[2] = "BLUE";
    compose_fader_text[3] = "";
    compose_stack_type = PIX;
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
    case PIX:
    composePix(e);
    break;
  }

  compose_last_handle = now;
  // Output only if JSON has changed
  if(compose_hash != compose_hash_temp or compose_stack_new or compose_param_change) {
    Serial.println("Compose Changed");
    notify();
    compose_hash = compose_hash_temp;
  }  
  compose_param_change = false;
  compose_stack_new = false;
}
// Runtime stuff, e.g. adjusting colors
void composeHandle() {
  if(e_compose) {
    if(now>(compose_last_handle+COMPOSE_INT)) {
      composeWorker();
    }
  }
}

void composeParam() {
  compose_param_change = true;
  switch(compose_stack_type) {
    case PIX:
      composeParamPix();
    break;
  }
  notify();
}
uint64_t composeHash() {
  return fade_val_8[0]+(fade_val_8[1]<<8)
  +(fade_val_8[2]<<16)+(fade_val_8[3]<<24); 
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
  compose_hash_temp = composeHash();
}

void composeSine(JsonObject &e) {
  uint16_t p = 0;
  uint8_t w = 0;
  w=map(fade_val[0],0,FADE_MAX,1,250);p=map(fade_val[1],0,FADE_MAX,1,10000);
  compose_fader_val[0] = w; compose_fader_val[1] = p;
     
  e["type"] = "sine";
  e["w"] = w;
  e["p"] = p;
  compose_hash_temp = composeHash();
}

void composePix(JsonObject &e) {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t c = 0;
  if(compose_stack_new) {
    // Everything new, set all
      e["type"] = "pix";
      e["color"] = 0;
      e["c"] = 1;
      e["f"] = 200;
  }
  if(compose_param>0) {
    // Page 1, Set C, F
    uint8_t c;
    uint16_t f;
    c=map(fade_val[0],0,FADE_MAX,1,10);f=map(fade_val[1],0,FADE_MAX,1,2000);
    compose_fader_val[0] = c; compose_fader_val[1] = f; compose_fader_val[2] = 0; compose_fader_val[3] = 0;
    e["c"] = c;
    e["f"] = f;
  } else {
    // Page 0, Set RGB
    r=map(fade_val[0],0,FADE_MAX,0,255);g=map(fade_val[1],0,FADE_MAX,0,255);b=map(fade_val[2],0,FADE_MAX,0,255);
    compose_fader_val[0] = r; compose_fader_val[1] = g; compose_fader_val[2] = b; compose_fader_val[3] =0;
    e["color"] = (r<<16)+(g<<8)+b;
  }
  compose_hash_temp = composeHash();
}
void composeParamPix() {
  if(compose_param == 0) {
    compose_param = 1;
    compose_fader_text[0] = "COUNT";
    compose_fader_text[1] = "FADE";
    compose_fader_text[2] = "";
    compose_fader_text[3] = "";
  } else {
    compose_param = 0;
    compose_fader_text[0] = "RED";
    compose_fader_text[1] = "GREEN";
    compose_fader_text[2] = "BLUE";
    compose_fader_text[3] = "";
  }
}
