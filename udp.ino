void sendUDP() {
    //Send a packet
    udp.beginPacket(udpAddress,udpPort);
    udp.write((uint8_t*)resJSON,resJSONlen);
    Serial.println("UDP:");
    Serial.println(udp.endPacket());
}
