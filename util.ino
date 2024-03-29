StaticJsonDocument<1024> doc;
StaticJsonDocument<100> blackout;

// Notify: Wrapper for display & output refresh
void notify() {
  d_changed = true;
  e_serial++;
  e_changed = true;
  buildJSON();
  sendUDP();
}
// Build response JSON
int buildJSON() {
  char filename[20];
  snprintf(filename, 20, "/effect%d.json",e_active);
  StaticJsonDocument<1024> response;
  response["instance"] = 0;
  response["serial"] = e_serial;
  response["effect"] = e_active;
  response["filename"] = filename;
  response["runtime"] = now;
  if(e_changed) {
    if(e_compose) {
      // If we're in compose mode, just use their JSON directly
      doc.clear();
      combine(response,compose_json);
    } else {
      // If we're in effect mode, try to load JSON from SPIFFS
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
    // Reset changed flag
    e_changed = false;    
  }

  combine(response,doc);

  dimJSON(response);

  resJSONlen = serializeJson(response,resJSON);
}
// Append dimmer to all outputs
void dimJSON(JsonDocument& d) {
  JsonArray effects = d["effects"];
  JsonObject e_dim = effects.createNestedObject();
  e_dim["type"] = "dim";
  e_dim["value"] = dim_value;
}
// Start with Blackout
void setupJSON() {
  JsonArray effects = blackout.createNestedArray("effects");
  JsonObject e1 = effects.createNestedObject();
  e1["type"] = "fill";
  e1["color"] = 0x000000;
}
// Merge two documents
void combine(JsonDocument& dst, const JsonDocument& src) {
    for (auto p : src.as<JsonObject>())
        dst[p.key()] = p.value();
}
