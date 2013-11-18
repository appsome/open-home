/*
  Basic serial bridge between computer and transmitter.
  Version fot Arduino Yun.
  
  Requires rc-switch library: http://code.google.com/p/rc-switch/

  Part of Open Home https://github.com/appsome/open-home
*/

#include <RCSwitch.h>
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

// Transmitter is connected to ATTiny Pin #2
#define TRANSMIT_PIN 2

RCSwitch mySwitch = RCSwitch();
YunServer server;

void setup() {
  // Optional set protocol (default is 1, will work for most outlets)
  mySwitch.setProtocol(1);
  
  // Optional set number of transmission repetitions.
  mySwitch.setRepeatTransmit(10);
  
  pinMode(TRANSMIT_PIN, OUTPUT);  
  mySwitch.enableTransmit(TRANSMIT_PIN);
  
  // Initiate Yun bridge
  Bridge.begin();
  
  // Start web server
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  YunClient client = server.accept();

  if (client) {
    process(client);
    client.stop();
  }

  delay(50);
}

void process(YunClient client) {
  String command = client.readStringUntil('/');

  if (command == "433") {
    parse433(client);
  }
}

void parse433(YunClient client) {
  int pulseLength;
  String triStateString;
  char triStateCode[13];
  triStateCode[12] = '\0';
  
  pulseLength = client.parseInt();
  mySwitch.setPulseLength(pulseLength);
  
  if (client.read() == '/') {
    triStateString = client.readStringUntil('/');
    triStateString.toCharArray(triStateCode, 12);
    mySwitch.sendTriState(triStateCode);
    client.print(F("{ sent: \""));
    client.print(triStateString);
    client.print(F("\" }"));
  } else {
    client.print(F("{ error: \"Unsufficient paramenters\" }"));  
  }
}
