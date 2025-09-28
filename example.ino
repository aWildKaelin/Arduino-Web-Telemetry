#include <WiFi.h>
#include "NetworkHandler.h"


WiFiMulti multi;
WiFiClient telemetryServer;

// you need to pass in the wifi client object so it can actually send and receive data
telemetryHandler telehandle(&telemetryServer);
const char* ssid = "YOUR-SSID";
const char* password = "YOUR-PASS";
const char* host = "10.0.0.1"; // replace with your computer's IP address
const uint16_t port = 8084; // this is the port I have chosen, but you are free to change it

int displayVarA = 0;
int displayVarB = 100;

void setup(){

  // replace this with your board's way of connecting to an AP
  // this is for a Pi Pico W
  multi.addAP(ssid, password);
  if (multi.run() != WL_CONNECTED) {
    Serial.println("Unable to connect to network, rebooting in 10 seconds...");
    delay(10000);
    rp2040.reboot();
  }


  // this is how you connect the board to the server
  // it doen't matter when you call `connect` as long as it takes place before `init`
  telehandle.connect(host, port);

  // the library will automatically track variables, so you need to give them a name and a pointer
  // telemetry variables will be displayed on the web interface
  telehandle.addTelemetryVar("variable 1", &displayVarA);
  telehandle.addTelemetryVar("variable 2", &displayVarB);

  // init tells the server that everything is ready and it can begin displaying
  // MUST be called after adding all variables
  telehandle.init();
}

void loop(){

  // updates vars
  // networking is quite unstable, so I recommend adding a timer of at least 100ms 
  // to ensure all data has been received and processed
  telehandle.sendVars();
  displayVarA++;
  displayVarB--;

  delay(1000);
}
