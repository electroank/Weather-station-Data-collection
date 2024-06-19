#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define DHTTYPE DHT11
const int DHTPin = 14; 
DHT dht(DHTPin, DHTTYPE); 
#define ON_Board_LED 2  

const char* ssid = "your-wifi-ssid";
const char* password = "your-wifi-password"; 

const char* host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client; 
Adafruit_BMP085 bmp;


String GAS_ID = "your script GAS_ID";

void setup() {

  Serial.begin(115200);
  delay(500);

  dht.begin(); 
  delay(500);

  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	while (1) {}
  }
  
  WiFi.begin(ssid, password); 
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT);
  pinMode(13,OUTPUT); 
  digitalWrite(ON_Board_LED, HIGH); 

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
   
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    
  }
  digitalWrite(ON_Board_LED, HIGH); 
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  client.setInsecure();
}

void loop() {
  int h = dht.readHumidity();
  int t_dht = dht.readTemperature();
  float t = bmp.readTemperature();
  float a = bmp.readAltitude();
  float p = bmp.readPressure();
  float ht = dht.computeHeatIndex(t_dht,h,false);
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor !");
    delay(500);
    return;
  }
  String Temp = "Temperature : " + String(t) + " °C";
  String Humi = "Humidity : " + String(h) + " %";
  Serial.println(Temp);
  Serial.println(Humi);

  Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");
    
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());
    Serial.println(" meters");
  
  sendData(t,p,a,h,ht);

}

void sendData(float tem, int pre, int alt , float humi,float heat_index) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String string_temperature =  String(tem,DEC);
  // String string_temperature =  String(tem, DEC); 
  String string_pressure = String(pre);
  String string_altitude = String(alt);
  String string_humidity =  String(humi);
  String heat =  String(heat_index);  
 
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&pressure=" + string_pressure + "&altitude=" + string_altitude + "&humidity=" + string_humidity + "&heat_index=" + heat;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
   
      break;
    }
  }
  String line = client.readStringUntil('\n');

  Serial.print("reply was : ");
  Serial.println(line);
  digitalWrite(13,HIGH);
  delay(100);
  digitalWrite(13,LOW);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
} 
