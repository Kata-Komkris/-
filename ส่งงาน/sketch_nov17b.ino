
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTTYPE DHT22


const char* ssid = "BarkKlaMI";
const char* password = "12345678";

const char* mqtt_server = "192.168.43.109";

WiFiClient espClient;
PubSubClient client(espClient);

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



void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;


  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();


  if (topic == "LED/ON-OFF") {
    Serial.print("Changing Room lamp to ");
    if (messageTemp == "on") {
      digitalWrite(LED, HIGH);
      Serial.print("On");
    }
    else if (messageTemp == "off") {
      digitalWrite(LED, LOW);
      Serial.print("Off");
    }

    else if (isNumeric(messageTemp)) {
      int x = messageTemp.toInt();
      ledcWrite(ledChannel, x);
    }
    //
    //   if(topic=="LED/ON-OFF2"){
    //
    //  }
    Serial.println();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }


  client.subscribe("LED/ON-OFF");
  client.subscribe("DHT");
  client.subscribe("count");
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
  setup_wifi();
  dht.begin();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }
  if (!client.loop()){
    client.connect("ESP8266Client");
  }

  DHT_READ();
  
  button();
 
  

}
