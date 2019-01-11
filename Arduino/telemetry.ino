void init_telem()
{
  Udp.begin(localUdpPort);
  loaded = true;
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

void recv_tcp()
{
  int i = 0;
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    int len = Udp.read(incomingPacket, 64);
    if (len <= 0)
      return ;
    byte channel = incomingPacket[0];
    if (channel == CALIB_CMD)
      calib();
    if (channel == STATE_CMD)
    {
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      int val = 32;
      Udp.write((uint8_t *)&val, sizeof(int));
      int a[2] = {(int)angle[0], (int)angle[1]};
      Udp.write((uint8_t *)a, sizeof(int) * 2);
      Udp.write((uint8_t *)servo, sizeof(uint16_t) * 4);
      Serial.println("Send gyro");
      Udp.endPacket();
    }
    else if (channel == GET_PID)
    {
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      int id = GET_PID;
      Udp.write((uint8_t *)&id, sizeof(int));
      Udp.write((uint8_t *)&P_PID[0], 3);
      Udp.write((uint8_t *)&I_PID[0], 3);
      Udp.write((uint8_t *)&D_PID[0], 3);
      Udp.endPacket();
    }
    else if (channel == SET_PID)
    {
      P_PID[0] = incomingPacket[1];
      P_PID[1] = incomingPacket[2];
      P_PID[2] = incomingPacket[3];
      I_PID[0] = incomingPacket[4];
      I_PID[1] = incomingPacket[5];
      I_PID[2] = incomingPacket[6];
      D_PID[0] = incomingPacket[7];
      D_PID[1] = incomingPacket[8];
      D_PID[2] = incomingPacket[9];
      PID_write();
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      int id = SET_PID;
      Udp.write((uint8_t *)&id, sizeof(int));
      Udp.endPacket();
    } else if (channel == STATE_ROTORS)
    {
      stateRotor = 1;
    }
    else
    {
      int value = incomingPacket[1] | (incomingPacket[2] << 8) | (incomingPacket[3] << 16) | (incomingPacket[4] << 24);
      Serial.print(channel);
      Serial.print(" = ");
      Serial.println(value);
      rcValue[channel] = value;
    }
  }
}
