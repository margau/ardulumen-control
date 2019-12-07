StaticJsonDocument<1024> compose_json;
JsonArray compose_effects;
boolean compose_stack_new = false;
uint32_t compose_hash = 0;
uint32_t compose_hash_temp = 0;
uint8_t compose_save_step = 0;
uint8_t compose_save_num = 0;
#define HSV_SECTION_3 (0x40)
enum c_effect_type {
  FILL,
  SINE,
  PIX,
  HSV,
  SAW
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
void composeSave() {
  char tmp[20];
  switch(compose_save_step) {
    case 1:
    popUp("SELECT");
    compose_save_step = 2;
    Serial.println("Save: Select Cue");
    break;
    case 3:
    snprintf(tmp, 20, "%d - Sure?",compose_save_num);
    popUp(tmp);
    Serial.println("Save: Confirm");
    break;
    case 4:
    snprintf(tmp, 20, "/effect%d.json",compose_save_num);
    char cJSON[1000];
    serializeJson(compose_json,cJSON);
    // Save
    File file = SPIFFS.open(tmp, FILE_WRITE);
    if(file.print(cJSON)){
        Serial.println("file written");
        popUp("Saved");
        file.close();
    } else {
        Serial.println("write failed");
        popUp("Failed");
    }
    compose_save_step = 0;
    // Exit Compose
    runEffect(compose_save_num);
    break;
  }
}
void composeButton(int i) {
  if(e_compose && compose_stack >= 6) {
    popUp("Max Stack!");
    notify();
    return;
  }
  switch(i) {
    case 13:
    popUp("Fill");
    compose_fader_text[0] = "RED";
    compose_fader_text[1] = "GREEN";
    compose_fader_text[2] = "BLUE";
    compose_stack_type = FILL;
    break;
    case 14:
    popUp("Fill HSV");
    compose_fader_text[0] = "HUE";
    compose_fader_text[1] = "SAT";
    compose_fader_text[2] = "VAL";
    compose_stack_type = HSV;
    break;
    case 16:
    popUp("Sine");
    compose_fader_text[0] = "WIDTH";
    compose_fader_text[1] = "PERIOD";
    compose_fader_text[2] = "";
    compose_stack_type = SINE;
    break;
    case 17:
    popUp("Sawtooth");
    compose_fader_text[0] = "WIDTH";
    compose_fader_text[1] = "PERIOD";
    compose_fader_text[2] = "";
    compose_stack_type = SAW;
    break;
    case 19:
    popUp("Fad. Pix");
    compose_fader_text[0] = "RED";
    compose_fader_text[1] = "GREEN";
    compose_fader_text[2] = "BLUE";
    compose_stack_type = PIX;
    break;
    default:
    popUp("NO EFFECT");
    notify();
    return;
  }
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
  compose_fader_val[0] = 0;
  compose_fader_val[1] = 0;
  compose_fader_val[2] = 0;
  compose_param = 0;
  compose_stack++;
  compose_stack_new = true;
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
    case HSV:
    composeFillHSV(e);
    break;
    case SINE:
    composeSine(e);
    break;
    case PIX:
    composePix(e);
    break;
    case SAW:
    composeSaw(e);
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
  +(fade_val_8[2]<<16); 
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
void composeFillHSV(JsonObject &e) {
    uint8_t r,g,b,h,saturation,value;
    compose_hash_temp = composeHash();
    h=map(fade_val[0],0,FADE_MAX,0,191);saturation=map(fade_val[1],0,FADE_MAX,0,255);value=map(fade_val[2],0,FADE_MAX,0,255);
    compose_fader_val[0] = h; compose_fader_val[1] = saturation; compose_fader_val[2] = value;
    
    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = 255 - saturation;
    uint8_t brightness_floor = value * invsat / 256;

    // The color amplitude is the maximum amount of R, G, and B
    // that will be added on top of the brightness_floor to
    // create the specific hue desired.
    uint8_t color_amplitude = value - brightness_floor;

    // Figure out which section of the hue wheel we're in,
    // and how far offset we are withing that section
    uint8_t section = h / HSV_SECTION_3; // 0..2
    uint8_t offset = h % HSV_SECTION_3;  // 0..63

    uint8_t rampup = offset; // 0..63
    uint8_t rampdown = (HSV_SECTION_3 - 1) - offset; // 63..0

    // compute color-amplitude-scaled-down versions of rampup and rampdown
    uint8_t rampup_amp_adj   = (rampup   * color_amplitude) / HSV_SECTION_3;
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / HSV_SECTION_3;

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor   = rampup_amp_adj   + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;


    if( section ) {
        if( section == 1) {
            // section 1: 0x40..0x7F
            r = brightness_floor;
            g = rampdown_adj_with_floor;
            b = rampup_adj_with_floor;
        } else {
            // section 2; 0x80..0xBF
            r = rampup_adj_with_floor;
            g = brightness_floor;
            b = rampdown_adj_with_floor;
        }
    } else {
        // section 0: 0x00..0x3F
        r = rampdown_adj_with_floor;
        g = rampup_adj_with_floor;
        b = brightness_floor;
    }
    e["type"] = "fill";
    e["color"] = (r<<16)+(g<<8)+b;
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

void composeSaw(JsonObject &e) {
  uint16_t p = 0;
  uint8_t w = 0;
  w=map(fade_val[0],0,FADE_MAX,1,250);p=map(fade_val[1],0,FADE_MAX,1,10000);
  compose_fader_val[0] = w; compose_fader_val[1] = p;
     
  e["type"] = "sawtooth";
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
    compose_fader_val[0] = c; compose_fader_val[1] = f; compose_fader_val[2] = 0;
    e["c"] = c;
    e["f"] = f;
  } else {
    // Page 0, Set RGB
    r=map(fade_val[0],0,FADE_MAX,0,255);g=map(fade_val[1],0,FADE_MAX,0,255);b=map(fade_val[2],0,FADE_MAX,0,255);
    compose_fader_val[0] = r; compose_fader_val[1] = g; compose_fader_val[2] = b;
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
  } else {
    compose_param = 0;
    compose_fader_text[0] = "RED";
    compose_fader_text[1] = "GREEN";
    compose_fader_text[2] = "BLUE";
  }
}
