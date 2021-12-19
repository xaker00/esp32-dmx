#include <Arduino.h>
#include <ArduinoOTA.h>

#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include "PwmControl.h"

#include <WebServer.h>
#include <ArduinoJson.h>

#include <dmx.h>

WiFiManager wifiManager;
void checkButton();

WebServer server(800);

char buffer[250];

#define CONTROLLER_COUNT 4
#define CONTROLLER_FIRST_ADDRESS 0x40
#define CONTROLLER_FREQ 600
//* pwm controllers
PwmControl *pwmControllers[CONTROLLER_COUNT];

const uint8_t BUTTON_PIN = 0;

void handlePost()
{
  if (server.hasArg("plain") == false)
  {
    //handle error here
  }
  String body = server.arg("plain");
  // JSON data buffer
  StaticJsonDocument<250> jsonDocument;
  deserializeJson(jsonDocument, body);

  const uint8_t channel = jsonDocument["channel"];
  // Get RGB components
  const uint8_t red = jsonDocument["red"];
  const uint8_t green = jsonDocument["green"];
  const uint8_t blue = jsonDocument["blue"];
  const uint8_t white = jsonDocument["white"];

  uint8_t controllerNum = channel / 16;
  if (controllerNum < CONTROLLER_COUNT)
  {
    pwmControllers[channel / 16]->SetColor(channel % 16, red, green, blue, white);
    printf("%i, %i, %i, %i \n", red, green, blue, white);
    // Respond to the client
    server.send(200, "application/json", "{}");
  }
  else
  {
    server.send(500, "application/json", "{\"message\":\"Invalid channel\"}");
  }
}

void setup_routing()
{
  server.on("/led", HTTP_POST, handlePost);

  // start server
  server.begin();
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(10);
  Serial.println("DMX Light Controller");
  DMX::Initialize(input);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  wifiManager.setConfigPortalTimeout(30);
  wifiManager.autoConnect("DMX_Controller");

  ArduinoOTA
      .onStart([]()
               {
                 String type;
                 if (ArduinoOTA.getCommand() == U_FLASH)
                   type = "sketch";
                 else // U_SPIFFS
                   type = "filesystem";

                 // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                 Serial.println("Start updating " + type);
               })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
                 Serial.printf("Error[%u]: ", error);
                 if (error == OTA_AUTH_ERROR)
                   Serial.println("Auth Failed");
                 else if (error == OTA_BEGIN_ERROR)
                   Serial.println("Begin Failed");
                 else if (error == OTA_CONNECT_ERROR)
                   Serial.println("Connect Failed");
                 else if (error == OTA_RECEIVE_ERROR)
                   Serial.println("Receive Failed");
                 else if (error == OTA_END_ERROR)
                   Serial.println("End Failed");
               });

  ArduinoOTA.begin();

  setup_routing();
  // initialize PWM controllers
  for (int i = 0; i < CONTROLLER_COUNT; i++)
  {
    pwmControllers[i] = new PwmControl(CONTROLLER_FIRST_ADDRESS + i, CONTROLLER_FREQ);
    pwmControllers[i]->begin();
  }

  delay(10);
}

void loop()
{
  server.handleClient();
  checkButton();
  ArduinoOTA.handle();
  delay(250);
}

void checkButton()
{
  static bool portalRunning = false;
  // is auto timeout portal running
  if (portalRunning)
  {
    wifiManager.process();
  }

  // is configuration portal requested?
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    delay(100);
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      if (!portalRunning)
      {
        Serial.println("Button Pressed, Starting Portal");
        wifiManager.startWebPortal();
        portalRunning = true;
      }
      else
      {
        Serial.println("Button Pressed, Stopping Portal");
        wifiManager.stopWebPortal();
        portalRunning = false;
      }
    }
  }
}
