#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "DHT.h"
#include "MQ135.h"
#define DHTPIN D2
#define DHTTYPE DHT22
#define LED D7
#define relay D1
DHT dht(DHTPIN, DHTTYPE);
//const int LED = D7;
//const int relay = ;
const int mq135Pin = 0;
MQ135 gasSensor = MQ135(mq135Pin);

const int temperature = 1;
const int humidity = 1;
const int ir = 1;
const int voc = 1;
const int hp_control = 1;
int r_time = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  pinMode(relay, OUTPUT);
  Serial.setTimeout(2000);
  if (voc == 1)
  {
    Serial.println("MQ135 Running!");
    float rzero = gasSensor.getRZero();
    Serial.print("R0: ");
    Serial.println(rzero);
  }
  if (temperature == 1 || humidity == 1)
  {
    while (!Serial)  {}
    Serial.println("DHT22 Running!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (hp_control == 1)
  {
    digitalWrite(relay, HIGH);
    delay(5000);
    digitalWrite(relay, LOW);
    delay(1000);
  }
  if (ir == 1)
  {
    digitalWrite(LED, HIGH);
    delay(5000);
    digitalWrite(LED, LOW);
   delay(1000);
 }
  if (r_time > 20000)
  {
    if (voc == 1)
    {
      //float ppm = gasSensor.getPPM();
      Serial.print("A0: ");
      int gas = analogRead(mq135Pin);
      Serial.println(gas);
      //Serial.print(" ppm CO2: ");
      //Serial.println(ppm);


    }

    if (temperature == 1)
    {
      float temperature = dht.readTemperature();
      if (isnan(temperature))
      {
        Serial.println("Failed to read temperature from DHT sensor!");
        r_time = 0;
        return;
      }
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println("*C");

    }

    if (humidity == 1)
    {
      float humidity = dht.readHumidity();

      if (isnan(humidity))
      {
        Serial.println("Failed to read humidity from DHT sensor!");
        r_time = 0;
        return;
      }

      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println("%\t");


    }

    r_time = 0;
  }

  delay(100);
  r_time += 12100;
}

