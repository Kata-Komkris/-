#include <WiFi.h>
#include <PubSubClient.h>
const char* ssid = "BarkKlaMI";
const char* password = "12345678";

const char* mqtt_server = "192.168.43.109";

WiFiClient espClient;
PubSubClient client(espClient);

const int LED = 25;

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
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
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

  DHT_READ();
  
  button();
 
  

}
