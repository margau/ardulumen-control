void notify() {
  
}
/*
 * JsonArray effects = response.createNestedArray("effects");
  JsonObject e1 = effects.createNestedObject();
  e1["type"] = "fill";
  e1["color"] = 0x00FF00;
  JsonObject e2 = effects.createNestedObject();
  e2["type"] = "sine";
  e2["w"] = 25;
  e2["p"] = 2000;
  JsonObject e3 = effects.createNestedObject();
  e3["type"] = "pix";
  e3["color"] = 0xFF0000;
  e3["f"] = 200;
  e3["c"] = 1;

 */
 void combine(JsonDocument& dst, const JsonDocument& src) {
    for (auto p : src.as<JsonObject>())
        dst[p.key()] = p.value();
}
