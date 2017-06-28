#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>  
       
static int alive_timer = 0;
long lastReconnectAttempt = 0;

// Pin Definitions //
const int LED_PIN = 2; // Sparkfun's green LED
const int LIGHT_PIN = 0; // D3 on WeMos

WiFiClientSecure wifiClient; // Use for secure TLS MQTT

// MQTT Connection info
char server[] = "mqtt.mediumone.com";
int port = 61620;
char pub_topic[]="0/<project mqtt>/<user mqtt>/esp8266/";
char sub_topic[]="1/<project mqtt>/<user mqtt>/esp8266/event";
char mqtt_username[]="<project mqtt>/<user mqtt>";
char mqtt_password[]="<api key>/<user password>";

void setup() {
  // init hardware
  Serial.begin(9600);
  while(!Serial){}
  
  delay(2000); // give device time to start

  // wifi setup
  WiFi.mode(WIFI_STA);
  WiFi.begin("<wifi ssid>", "<wifi password>");
  delay(3000);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("failed to connect, we should reset and see if it connects");
    ESP.reset();
  }
  // if you get here you have connected to the WiFi
  Serial.println("connected to Wifi!");

  Serial.println();
  Serial.println("Init hardware");
  pinMode(LED_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
 
  // connect to MQTT broker to send board reset msg
  connectMQTT();
  Serial.println("Complete Setup");
}

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  int i = 0;
  char message_buff[length + 1];
  for(i=0; i < length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  Serial.print("Received some data: ");
  Serial.println(String(message_buff));
  
  // Process message to turn lights on and off
  if (String(message_buff[0]) == "0") { // 0 = Turn off
    if (String(message_buff[1]) == "0") digitalWrite(LED_PIN, HIGH); // 0 = LED
    else digitalWrite(LIGHT_PIN, LOW); // 1 = PIN
  } else if (String(message_buff[0]) == "1") { // 1 = Turn on
    if (String(message_buff[1]) == "0") digitalWrite(LED_PIN, LOW);
    else digitalWrite(LIGHT_PIN, HIGH);
  }
}

PubSubClient client(server, port, callback, wifiClient);
boolean connectMQTT()
{    
  if (client.connect((char*) mqtt_username,(char*) mqtt_username, (char*) mqtt_password)) {
    Serial.println("Connected to MQTT broker");
    if (client.publish((char*) pub_topic, "{\"event_data\":{\"mqtt_connected\":true}}")) {
      Serial.println("Publish connected message ok");
    } else {
      Serial.print("Publish connected message failed: ");
      Serial.println(String(client.state()));
    }
    
    if (client.subscribe((char *)sub_topic,1)){
      Serial.println("Successfully subscribed");
    } else {
      Serial.print("Subscribed failed: ");
      Serial.println(String(client.state()));
    }
  } else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }
  return client.connected();
}

void loop() {
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 1000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (connectMQTT()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    client.loop();
  }
}

