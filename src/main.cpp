#include <Arduino.h>
#include <ArduinoOTA.h>

#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include "PwmControl.h"

#include <WebServer.h>
#include <ArduinoJson.h>

#include <dmx.h>

int readcycle = 0;

WiFiManager wifiManager;
void checkButton();

WebServer server(800);

// JSON data buffer
StaticJsonDocument<250> jsonDocument;
char buffer[250];

PwmControl pwm(0x40, 1.05 * 600); // actual frequency is 5% below this number

const uint8_t BUTTON_PIN = 0;

void handlePost()
{
  if (server.hasArg("plain") == false)
  {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  const uint8_t red = jsonDocument["red"];
  const uint8_t green = jsonDocument["green"];
  const uint8_t blue = jsonDocument["blue"];
  const uint8_t white = jsonDocument["white"];
  pwm.SetColor(0, red, green, blue, white);
  printf("%i, %i, %i, %i \n", red, green, blue, white);

  // Respond to the client
  server.send(200, "application/json", "{}");
}

void setup_routing()
{
  // server.on("/temperature", getTemperature);
  // server.on("/pressure", getPressure);
  // server.on("/humidity", getHumidity);
  // server.on("/env", getEnv);
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
  pwm.begin();
  pwm.SetColor(0, 0, 0, 0, 0);
  delay(10);
}

void loop()
{
  server.handleClient();
  checkButton();
  ArduinoOTA.handle();
  // pwm.setPin(0, 0 * 4095);
  // pwm.setPin(1, TEST_LEVEL * 4095);
  // pwm.setPin(2, TEST_LEVEL * 4095);
  // pwm.setPin(3, 0 * 4095);
  // delay(1000 * 5);
}

// void loop1()
// {
//   // put your main code here, to run repeatedly:
//   pwm.setPin(3, 0);
//   pwm.setPin(0, TEST_LEVEL * 4095);
//   delay(1000 * 5);

//   pwm.setPin(0, 0);
//   pwm.setPin(1, TEST_LEVEL * 4095);
//   delay(1000 * 5);

//   pwm.setPin(1, 0);
//   pwm.setPin(2, TEST_LEVEL * 4095);
//   delay(1000 * 5);

//   pwm.setPin(2, 0);
//   pwm.setPin(3, TEST_LEVEL * 4095);
//   delay(1000 * 5);
// }

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
    delay(50);
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
