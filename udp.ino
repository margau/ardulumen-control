void sendUDP() {
    //Send a packet from global JSON
    udp.beginPacket(udpAddress,udpPort);
    udp.write((uint8_t*)resJSON,resJSONlen);
    udp.endPacket();
}
