#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ArduinoMqttClient.h>
#include <RCSwitch.h>
#include <ArduinoOTA.h>
#include <Ticker.h>

#define DISABLE_RFRECEIVE   // receiving is useful for debugging but not necessary for this
#define DEBUG true          // set to true for debug output, false for no serial output
#define DEBUG_SERIAL \
  if (DEBUG) Serial
#define DEBUG_SPRINTF \
  if (DEBUG) sprintf

WiFiClient wifiClient;
WiFiManager wm;
MqttClient mqttClient(wifiClient);

const char broker[] = "10.0.1.171";
int        port     = 1883;
const char topic[]  = "rf433/control";

unsigned long remoteCodes[5][2] =
{     // ON     // OFF
    { 0x115533, 0x11553C }, // Button 1
    { 0x1155C3, 0x1155CC }, // Button 2
    { 0x115703, 0x11570C }, // Button 3
    { 0x115D03, 0x115D0C }, // Button 4
    { 0x117503, 0x11750C }  // Button 5
};

#define SEND_PIN D1
RCSwitch sendSwitch = RCSwitch();

#if not defined(DISABLE_RFRECEIVE)
#define RECEIVE_PIN D3
RCSwitch receiveSwitch = RCSwitch();
#endif

// Check to see if there's an MQTT message telling us to transmit...
void checkTransmit()
{
    int messageSize = mqttClient.parseMessage();
    if (messageSize)
    {   // we received a message...
        DEBUG_SERIAL.print("Received a message with topic '");
        DEBUG_SERIAL.print(mqttClient.messageTopic());
        DEBUG_SERIAL.print("', length ");
        DEBUG_SERIAL.print(messageSize);
        DEBUG_SERIAL.println(" bytes:");

        static char message[100], serialStr[100];

        // use the Stream interface to get the message
        for (int i = 0; i < messageSize; i++)
            message[i] = (char)mqttClient.read();
        message[messageSize] = '\0';
        DEBUG_SERIAL.println(message);

        if ((strlen(message) > 10) && (strncmp(message, "button", 6) == 0))
        {
            int buttonNum = message[7] - '0';
            if ((strlen(message) == 11) && (strcmp(message + 9, "ON") == 0))
            {   // ON
                DEBUG_SPRINTF(serialStr, "Remote ON, button number %d", buttonNum);
                sendSwitch.send(remoteCodes[buttonNum - 1][0], 24);
            }
            else if ((strlen(message) == 12) && (strcmp(message + 9, "OFF") == 0))
            {   // OFF
                DEBUG_SPRINTF(serialStr, "Remote OFF, button number %d", buttonNum);
                sendSwitch.send(remoteCodes[buttonNum - 1][1], 24);
            }
            else
                DEBUG_SPRINTF(serialStr, "Unknown command for button number %d", buttonNum);
        }
        else
            DEBUG_SPRINTF(serialStr, "Unknown command");

        DEBUG_SERIAL.println(serialStr);            
        DEBUG_SERIAL.println();
    }
}
Ticker checkTransmitTicker(checkTransmit, 100, 0, MILLIS);

#if not defined(DISABLE_RFRECEIVE)
void checkReceive()
{
    if (receiveSwitch.available())
    {   // something's there...
        DEBUG_SERIAL.print("Received 0x");
        DEBUG_SERIAL.print(receiveSwitch.getReceivedValue(), HEX);
        DEBUG_SERIAL.print(" / ");
        DEBUG_SERIAL.print(receiveSwitch.getReceivedBitlength());
        DEBUG_SERIAL.print("bit ");
        DEBUG_SERIAL.print("Protocol: ");
        DEBUG_SERIAL.println(receiveSwitch.getReceivedProtocol());
        receiveSwitch.resetAvailable();
    }
}
Ticker checkReceiveTicker(checkReceive, 100, 0, MILLIS);
#endif

// Transmit alternating ON/OFF signals (for debugging)
static bool onOffToggleState = false;
void toggleOnOff()
{
    onOffToggleState = !onOffToggleState;
    if (onOffToggleState)
    {
        DEBUG_SERIAL.println("Remote ON, button number 1");
        sendSwitch.send(remoteCodes[0][0], 24);
    }
    else
    {
        DEBUG_SERIAL.println("Remote OFF, button number 1");
        sendSwitch.send(remoteCodes[0][1], 24);
    }
}
Ticker toggleOnOffTicker(toggleOnOff, 2000, 0, MILLIS);

void setup()
{
    if (DEBUG)
    {
        Serial.begin(115200);
        while (!Serial) delay(100);
    }

    // try to connect to wifi...
    if (!wm.autoConnect())
    {
        DEBUG_SERIAL.println("WiFi not connected");
        // try to start the portal...
        if (!wm.startConfigPortal())
            DEBUG_SERIAL.println("Failed to start portal");
    }

    DEBUG_SERIAL.print("Wifi connected, IP address: ");
    DEBUG_SERIAL.println(WiFi.localIP());

    DEBUG_SERIAL.print("Attempting to connect to the MQTT broker: ");
    DEBUG_SERIAL.println(broker);
    
    short notConnectedCounter = 0;
    while (!mqttClient.connect(broker, port))
    {
        DEBUG_SERIAL.print("MQTT connection failed! Error code = ");
        DEBUG_SERIAL.println(mqttClient.connectError());
        delay(1000);
        notConnectedCounter++;
        if(notConnectedCounter > 20)
        {   // Reset board if not connected after 20s...
            DEBUG_SERIAL.println("Resetting due to MQTT not connecting...");
            ESP.restart();
        }
    }
    
    DEBUG_SERIAL.println("Connected to MQTT broker");
    DEBUG_SERIAL.print("Subscribing to topic: ");
    DEBUG_SERIAL.println(topic);
    mqttClient.subscribe(topic);    
    DEBUG_SERIAL.print("Waiting for messages on topic: ");
    DEBUG_SERIAL.println(topic);

    // enable transmitter
    sendSwitch.enableTransmit(SEND_PIN);
    sendSwitch.setProtocol(1);          // optional (default is 1)
    sendSwitch.setPulseLength(190);     // optional
    sendSwitch.setRepeatTransmit(10);   // optional
    checkTransmitTicker.start();

#if not defined(DISABLE_RFRECEIVE)
    // enable receiver
    receiveSwitch.enableReceive(RECEIVE_PIN);
    checkReceiveTicker.start();
#endif

    // toggleOnOffTicker.start();

    ArduinoOTA.begin();
}

void loop()
{
    checkTransmitTicker.update();

#if not defined(DISABLE_RFRECEIVE)
    checkReceiveTicker.update();
#endif

    //toggleOnOffTicker.update();

    ArduinoOTA.handle();
}
