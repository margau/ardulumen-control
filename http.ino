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
StaticJsonDocument<512> doc;
void handleLEDconfig() {
  char temp[400];
  char filename[20];
  snprintf(filename, 20, "/effect%d.json",e_active);
  StaticJsonDocument<400> response;
  response["instance"] = 0;
  response["serial"] = 0;
  response["effect"] = e_active;
  response["filename"] = filename;
  if(e_changed) {
    Serial.println("Loading new JSON");
    if(!SPIFFS.exists(filename)) {
      response["error"] = F("effect-json does not exists!");
      Serial.println(F("effect-json does not exists!"));
      serializeJson(response,temp);
      server.send(500, "application/json",temp);
      return;
    }
    File file = SPIFFS.open(filename, FILE_READ);
   
    if (!file) {
      response["error"] = F("effect-json not readable");
      Serial.println(F("effect-json not readable"));
      serializeJson(response,temp);
      server.send(500, "application/json",temp);
      return;
    }
    // Read effects from JSON
    
    DeserializationError error = deserializeJson(doc, file);
    if (error){
      response["error"] = F("effect-json not valid");
      Serial.println(F("effect-json not valid"));
      serializeJson(response,temp);
      server.send(500, "application/json",temp);
      return;
    }
    e_changed = false;    
    file.close();
  }

  combine(response,doc);

  serializeJson(response,temp);
  server.send(200, "application/json",temp);
}
