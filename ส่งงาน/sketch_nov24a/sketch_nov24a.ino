#include <Wire.h>
#include "dw_font.h"
#include "SSD1306.h"

#include <PubSubClient.h>
#include <WiFi.h>
#include "DHT.h"
const char* ssid = "BarkKlaMI";
const char* password = "12345678";

#define mqtt_server "192.168.43.109"
#define mqtt_port 1883
#define mqtt_user "TEST"
#define mqtt_password "12345"
#define DHTTYPE DHT22
WiFiClient espClient;
PubSubClient client(espClient);

SSD1306   display(0x3c, 21, 22);

extern dw_font_info_t font_th_sarabunpsk_regular40;
dw_font_t myfont;

const int LED = 25;
const int LED2 = 26;
const int DHTPIN = 4;
 int BUTT = 2;

const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

int BUTTSTATE = 0;
static char celsiusTemp[7];
static char humidity[7];
static char fahrenheitTemp[7];
static char NUMB[7];

unsigned long currenttime2 = millis();

DHT dht(DHTPIN, DHTTYPE);


long now = millis();
long lastMeasure = 0;

void draw_pixel(int16_t x, int16_t y)
{
  display.setColor(WHITE);
  display.setPixel(x, y);
}

void clear_pixel(int16_t x, int16_t y)
{
  display.setColor(BLACK);
  display.setPixel(x, y);
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg = "";
  
  int i = 0;
  
  while (i < length) msg += (char)payload[i++];

  Serial.println(msg);
  //  client.publish("temp","asdfa");
   if (msg == "on") {
    digitalWrite(LED, HIGH);
  }
  if (msg == "off") {
    digitalWrite(LED, LOW);
  }
  
  if (isNumeric(msg)) {
    ledcWrite(ledChannel, msg.toInt());
  }

  if (not strcmp(topic, "oled")) {
    Serial.println(msg);

      display.clear();
      dw_font_setfont(&myfont, &font_th_sarabunpsk_regular40);
      dw_font_goto(&myfont, 10, 40);
      dw_font_print(&myfont,(char*) msg.c_str());
      display.display();
    
//    display.clearDisplay();
//    display.setTextSize(2);
//    display.setTextColor(WHITE);
//    display.setCursor(0, 0);
//    display.print(msg);
//    display.display();
  }



}

void runmqtt() {

  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
//      client.subscribe("led");
//      client.subscribe("run");
      client.subscribe("oled");
      client.subscribe("LED/ON-OFF");
      client.subscribe("DHT");
      client.subscribe("count");

    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      return;
    }
  }

}




boolean isNumeric(String str) {
  unsigned int stringLength = str.length();

  if (stringLength == 0) {
    return false;
  }

  boolean seenDecimal = false;

  for (unsigned int i = 0; i < stringLength; ++i) {
    if (isDigit(str.charAt(i))) {
      continue;
    }

    if (str.charAt(i) == '.') {
      if (seenDecimal) {
        return false;
      }
      seenDecimal = true;
      continue;
    }
    return false;
  }
  return true;
}

void DHT_READ(){

  now = millis();

  if(now - lastMeasure > 3000){
    lastMeasure = now;

   float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hic = dht.computeHeatIndex(f, h);
  dtostrf(hic, 6, 2, fahrenheitTemp);
  // Compute heat index in Celsius (isFahreheit = false)
  float hif = dht.computeHeatIndex(t, h, false);
  dtostrf(hif, 6, 2, celsiusTemp);
  dtostrf(h, 6, 2, humidity);
 
  // Publishes Temperature and Humidity values
  client.publish("room/temperature", celsiusTemp);
  
//  Serial.println(Temperature);
  client.publish("room/humidity", humidity);
  client.publish("room/fahrenheit", fahrenheitTemp);
 
  }
}
void button(){
  int newest = digitalRead(BUTT);
  if(BUTTSTATE != newest){
    client.publish("sw",newest?"ON":"OFF");
    Serial.println(BUTTSTATE);
    BUTTSTATE = newest;
  }
}


void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUTT,INPUT);
  BUTTSTATE = digitalRead(BUTT);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(LED2, ledChannel);
  Serial.begin(115200);
 
  dht.begin();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
  
  uint16_t width = 0;
 
  Serial.begin(115200);
  
  display.init();
  display.flipScreenVertically();

  dw_font_init(&myfont,
               128,
               64,
               draw_pixel,
               clear_pixel);

  dw_font_setfont(&myfont, &font_th_sarabunpsk_regular40);
  dw_font_goto(&myfont, 10, 40);
//    dw_font_goto(&myfont, 10, 60);
  dw_font_print(&myfont, "สวัสดีAB");
  display.display();


  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
}

void loop() {
  // put your main code here, to run repeatedly:

    runmqtt();
  client.loop();
DHT_READ();
  
  button();
}
