#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "BarkKlaMI";
const char* password = "12345678";

const char* mqtt_server = "192.168.43.109";

WiFiClient espClient;
PubSubClient client(espClient);
int BUTT = 2;
int BUTTSTATE = 0;

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
  client.subscribe("count");
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
  
  pinMode(BUTT,INPUT);
  BUTTSTATE = digitalRead(BUTT);
  Serial.begin(115200);
  setup_wifi();
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


  
  button();
 
  

}
