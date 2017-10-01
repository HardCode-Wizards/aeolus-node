#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "DHT.h"
#include "MQ135.h"

#define DHTPIN D2
#define DHTTYPE DHT22
#define LED D7  //
#define relay D1 //
DHT dht(DHTPIN, DHTTYPE);

const int mq135Pin = 0;
MQ135 gasSensor = MQ135(mq135Pin);

const int temperature = 1;
const int humidity = 1;
const int ir = 0;
const int voc = 1;
const int hp_control = 0;

String APIkey;
String password;
String ssid;
int r_time = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  pinMode(relay, OUTPUT);
  Serial.println(password);
  Serial.println(ssid);
  
  SPIFFS.begin();
  if (!SPIFFS.begin()) 
  {
    Serial.println("Failed to mount file system");
  }
   if (SPIFFS.exists("/pass.txt"))
   {
    Serial.println("1");
      File pass = SPIFFS.open("/pass.txt", "r");
      if (!pass) 
      {
          Serial.println("Failed to open config file");
       }
             while (pass.available())
             {
                  password += char(pass.read()); //
                  Serial.println(password);
             }
             pass.close();
   }
   if (SPIFFS.exists("/ssidf.txt"))
   {
    Serial.println("2");
      File ssidf = SPIFFS.open("/ssidf.txt","r");
      if (!ssidf) {
          Serial.println("Failed to open config file");
       }
             while (ssidf.available())
             {
                  ssid += char(ssidf.read()); //
             }
             ssidf.close();
   }
   if (SPIFFS.exists("/key.txt"))
   {
    Serial.println("3");
      File key = SPIFFS.open("/key.txt","r");
      if (!key) {
          Serial.println("Failed to open config file");
       }
             while (key.available())
             {
                  APIkey += char(key.read());
             }
             key.close();
   }
   SPIFFS.end();
   
   Serial.setTimeout(2000);
   if (voc == 1)  {
   Serial.println("MQ135 Running!");
   float rzero = gasSensor.getRZero();
   Serial.print("R0: ");
   Serial.println(rzero);
   }
   if (temperature == 1 || humidity == 1) {
   while(!Serial)  {}
   Serial.println("DHT22 Running!");
   }
}

void loop() {
    SPIFFS.begin();
  // put your main code here, to run repeatedly:
        String incomingBytes;
        String firstString, secondString;

        if (Serial.available() > 0) 
        {
                // read the incoming bytes:
                incomingBytes = Serial.readString();

                for (int i = 0; i < incomingBytes.length(); i++) 
                {
                      if (incomingBytes.substring(i, i+1) == " ") 
                      {
                        firstString = incomingBytes.substring(0, i);
                        Serial.println(firstString);
                        secondString = incomingBytes.substring(i+1);
                        Serial.println(secondString);
                        break;
                      }
                }
        }
        
        if (firstString == "set_password")
        {
                File pass = SPIFFS.open("/pass.txt","w");
                if(!pass)
                {
                  Serial.println("Could not open file for writing.");
                }
                pass.print(secondString);
                pass.close();
                password = secondString;
                Serial.println("pass ok");
        }
        if (firstString == "set_ssid")
        {
                File ssidf = SPIFFS.open("/ssidf.txt","w");
                if(!ssidf)
                {
                  Serial.println("Could not open file for writing.");
                }
                ssidf.print(secondString);
                ssidf.close();
                ssid = secondString;
                Serial.println("ssid ok");
        }
        if (firstString == "set_api_key")
        {
                File key = SPIFFS.open("/key.txt","w");
                if(!key)
                {
                  Serial.println("Could not open file for writing.");
                }
                key.print(secondString);
                key.close();
                Serial.println("key ok");
        }
        if (incomingBytes == "get_capabilities")
        {
          int capabilities = 1*temperature + 2*humidity + 4*ir + 8*voc + 16*hp_control;
          Serial.println(capabilities);
        }

        SPIFFS.end();
        if(WiFi.status() != WL_CONNECTED && password[0] != '\0' && ssid[0] != '\0')
        {
           WiFiConnection(password, ssid);
        }
        if(WiFi.status() == WL_CONNECTED)
        {
          String status_c1, status_ir1;
                    if (r_time > 20000) 
                    {
                      if (ir == 1 || hp_control == 1)
                      {
                       String url = "http://172.24.1.1:3000/nodes/" + APIkey;
                       HTTPClient get_http;
                       get_http.begin(url);
                       int httpCode = get_http.GET();
                       if (httpCode == 200) 
                       {
                          WiFiClient *stream = get_http.getStreamPtr();
                          DynamicJsonBuffer jsonBuffer_get;

                          while (get_http.connected())  
                          {
                              if (stream->available())  
                              {
                                  String response = stream->readStringUntil('\r');
                                  response.trim();
                                  Serial.println("response");
                                  Serial.println(response);
                                  
                                  JsonObject& root = jsonBuffer_get.parseObject(response.c_str());
                                  
                                  
                                   char jsonMessageBuffer[300];
                                   root.prettyPrintTo(jsonMessageBuffer, sizeof(jsonMessageBuffer));
                                   Serial.println(jsonMessageBuffer);
                                  
                                  if (root.success()) 
                                  {
                                      String status_c = root["control"];
                                      String status_ir = root["ir"];
                                      status_c1 = status_c;
                                      status_ir1 = status_ir;
                                      Serial.println(status_c1);
                                      Serial.println(status_ir1);                         
                                     String payload6 = get_http.getString(); //Get the response payload
                  
                                     Serial.println(httpCode); //Print HTTP return code
                                     Serial.println(payload6); //Print request response payload
      
                                  }
                              }
                          }
                       }
                       }
                        if (hp_control == 1 && status_c1 == "on")   //BEGIN EDIT
                        {
                          digitalWrite(relay, HIGH);
                          Serial.println(digitalRead(relay));
                        }
                        else
                        {
                          digitalWrite(relay, LOW);
                        }
                        if (ir == 1 && status_ir1 == "on")
                        {
                          digitalWrite(LED, HIGH);
                          Serial.println(digitalRead(LED));
                         } 
                         else 
                         {
                            digitalWrite(LED, LOW);
                         }//LOW EDIT
                       
                       // prop}
                          
                        if (voc == 1)
                        {
                         //float ppm = gasSensor.getPPM();
                         Serial.print("A0: ");
                         int gas = analogRead(mq135Pin);
                         Serial.println(gas);
                         //Serial.print(" ppm CO2: ");
                         //Serial.println(ppm);

                         DynamicJsonBuffer jsonBuffer3;
                         JsonObject& v3 = jsonBuffer3.createObject();
                         v3["api_key"] = APIkey;
                         JsonObject& data3 = v3.createNestedObject("data_point");
                         data3["kind"] = "voc";
                         data3["value"] = gas;
  
                         //Serial.print(v3);
                         Serial.println(sizeof(v3));
                         
                         char jsonMessageBuffer3[300];
                         v3.prettyPrintTo(jsonMessageBuffer3, sizeof(jsonMessageBuffer3));
                         Serial.println(jsonMessageBuffer3);
      
                         HTTPClient http3; //Declare object of class HTTPClient
                         http3.begin("http://172.24.1.1:3000/data_points"); //Specify request destination
                         http3.addHeader("Content-Type", "application/json"); //Specify content-type header
      
                         int httpCode3 = http3.POST(jsonMessageBuffer3); //Send the request
                         String payload3 = http3.getString(); //Get the response payload
      
                         Serial.println(httpCode3); //Print HTTP return code
                         Serial.println(payload3); //Print request response payload
      
                         http3.end(); //Close connection   
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
  
                         DynamicJsonBuffer jsonBuffer1;
                         JsonObject& v1 = jsonBuffer1.createObject();
                         v1["api_key"] = APIkey;
                         JsonObject& data1 = v1.createNestedObject("data_point");
                         data1["kind"] = "temperature";
                         data1["value"] = temperature;
  
                         //Serial.print(v1);
                         Serial.println(sizeof(v1));
                         
                         char jsonMessageBuffer1[300];
                         v1.prettyPrintTo(jsonMessageBuffer1, sizeof(jsonMessageBuffer1));
                         Serial.println(jsonMessageBuffer1);
      
                         HTTPClient http1; //Declare object of class HTTPClient
                         http1.begin("http://172.24.1.1:3000/data_points"); //Specify request destination
                         http1.addHeader("Content-Type", "application/json"); //Specify content-type header
      
                         int httpCode1 = http1.POST(jsonMessageBuffer1); //Send the request
                         String payload1 = http1.getString(); //Get the response payload
      
                         Serial.println(httpCode1); //Print HTTP return code
                         Serial.println(payload1); //Print request response payload
      
                         http1.end(); //Close connection
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

                         DynamicJsonBuffer jsonBuffer2;
                         JsonObject& v2 = jsonBuffer2.createObject();
                         v2["api_key"] = APIkey;
                         JsonObject& data2 = v2.createNestedObject("data_point");
                         data2["kind"] = "humidity";
                         data2["value"] = humidity;
  
                         //Serial.print(v2);
                         Serial.println(sizeof(v2));
                         
                         char jsonMessageBuffer2[300];
                         v2.prettyPrintTo(jsonMessageBuffer2, sizeof(jsonMessageBuffer2));
                         Serial.println(jsonMessageBuffer2);
      
                         HTTPClient http2; //Declare object of class HTTPClient
                         http2.begin("http://172.24.1.1:3000/data_points"); //Specify request destination
                         http2.addHeader("Content-Type", "application/json"); //Specify content-type header
      
                         int httpCode2 = http2.POST(jsonMessageBuffer2); //Send the request
                         String payload2 = http2.getString(); //Get the response payload
      
                         Serial.println(httpCode2); //Print HTTP return code
                         Serial.println(payload2); //Print request response payload
      
                         http2.end(); //Close connection   
                       }
                       
                       r_time = 0;
                     }
                     yield();
                     delay(1000); 
                     r_time += 1000;
                   }              
                  // else
                   //{
                     // Serial.println("Error in WiFi connection");
                   //}
}

void WiFiConnection(String password, String ssid) 
{
  int status;
  status = WiFi.begin(ssid.c_str(), password.c_str());
  status = WiFi.waitForConnectResult();
  
  if (status != WL_CONNECTED) 
  {
    Serial.println("Connection failed.");
  }
  else
  {
    Serial.println("Connected.");
    Serial.println("MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}
