#include <Arduino.h>
#include <WiFi.h>


struct varName {
    String name;
    int* var;
};


class telemetryHandler
{
public:
    telemetryHandler(WiFiClient* _telemetryServer) {
        telemetryServer = _telemetryServer;
    }
    ~telemetryHandler() {}

    void connect(const char* host, const uint16_t port) {
        while (!telemetryServer->connect(host, port)) {
            Serial.println("connection to server failed");
            delay(30000);
        }
    }
    void init() {
        if (telemetryServer->connected()) {
            String buff = "";
            for (int i = 0; i < receiveVarCount; i++)
            {
                buff += receiveVar[i].name;
                buff += ',';
                buff += String(*receiveVar[i].var);
                buff += ',';
            }
            telemetryServer->println(buff);
        }
    }

    void addTelemetryVar(const char* name, int* var) { telemetryVar[telemetryVarCount++] = { name, var }; }
    void addReceiveVar(const char* name, int* var) { receiveVar[receiveVarCount++] = { name, var }; }

    void sendVars() {
        if (telemetryServer->connected())
        {
            String buff = "";
            for (int i = 0; i < telemetryVarCount; i++)
            {
                buff += telemetryVar[i].name;
                buff += ',';
                buff += String(*telemetryVar[i].var);
                buff += ',';
            }
            telemetryServer->println(buff);
        }
    }

    void sendRaw(char buffer[])
    {
        if (telemetryServer->connected())
        {
            String buff;
            buff += char(2);
            buff += buffer;
            telemetryServer->println(buff);
        }
    }

    void receiveVars()
    {
        if (telemetryServer->connected())
        {
            if (toReceive == -1 && telemetryServer->available()) toReceive = telemetryServer->read();
            else {
                if (telemetryServer->available() == toReceive)
                {
                    for (int i = 0; i < receiveVarCount; i++)
                        *receiveVar[i].var = telemetryServer->parseInt();
                }
            }
        }
    }

private:
    varName telemetryVar[1023];
    uint16_t telemetryVarCount = 0;
    varName receiveVar[1023];
    uint16_t receiveVarCount = 0;

    int16_t toReceive;

    WiFiClient* telemetryServer;
};