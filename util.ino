StaticJsonDocument<1024> doc;
StaticJsonDocument<100> blackout;

void notify() {
  d_changed = true;
  e_serial++;
  e_changed = true;
  buildJSON();
  sendUDP();
}
int buildJSON() {
  char filename[20];
  snprintf(filename, 20, "/effect%d.json",e_active);
  StaticJsonDocument<400> response;
  response["instance"] = 0;
  response["serial"] = e_serial;
  response["effect"] = e_active;
  response["filename"] = filename;
  response["runtime"] = now;
  if(e_changed) {
    if(e_compose) {
      doc.clear();
      combine(response,compose_json);
    } else {
      Serial.println("Loading new JSON");
      if(!SPIFFS.exists(filename)) {
        response["error"] = F("effect-json does not exists!");
        Serial.println(F("effect-json does not exists!"));
        combine(response,blackout);
        resJSONlen = serializeJson(response,resJSON);
        return 1;
      }
      File file = SPIFFS.open(filename, FILE_READ);
     
      if (!file) {
        response["error"] = F("effect-json not readable");
        Serial.println(F("effect-json not readable"));
        combine(response,blackout);
        resJSONlen = serializeJson(response,resJSON);
        return 2;
      }
      // Read effects from JSON
      
      DeserializationError error = deserializeJson(doc, file);
      if (error){
        response["error"] = F("effect-json not valid");
        Serial.println(F("effect-json not valid"));
        combine(response,blackout);
        resJSONlen = serializeJson(response,resJSON);
        return 3;
      }
      
      file.close();
    }
    e_changed = false;    
  }

  combine(response,doc);

  resJSONlen = serializeJson(response,resJSON);
}
void setupJSON() {
  JsonArray effects = blackout.createNestedArray("effects");
  JsonObject e1 = effects.createNestedObject();
  e1["type"] = "fill";
  e1["color"] = 0x000000;
}
void combine(JsonDocument& dst, const JsonDocument& src) {
    for (auto p : src.as<JsonObject>())
        dst[p.key()] = p.value();
}
