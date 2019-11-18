
void handleRoot() {
  server.send(200, "text/plain", "ardulumen-control");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleLEDconfig() {
  StaticJsonDocument<400> response;
  response["instance"] = 0;
  response["serial"] = 0;
  response["effect"] = e_active;
  JsonArray effects = response.createNestedArray("effects");
  JsonObject e1 = effects.createNestedObject();
  e1["type"] = "fill";
  e1["color"] = 0xFF0000;
  JsonObject e2 = effects.createNestedObject();
  e2["type"] = "sine";
  e2["w"] = 25;
  e2["p"] = 2000;
  JsonObject e3 = effects.createNestedObject();
  e3["type"] = "pix";
  e3["color"] = 0xFF0000;
  e3["f"] = 200;
  e3["c"] = 1;
  char temp[400];
  serializeJson(response,temp);
  server.send(200, "application/json",temp);
}
