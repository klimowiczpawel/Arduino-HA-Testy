/*
 * Płytka Arduino Mega opis podłączeń
 * 
 * Ethernet:
 * 5V - 5V
 * GND - GND
 * SCLK - AM-52
 * SCS - AM-10
 * MOSI - AM-51
 * MISO - AM-50
 * 
 * Pin sterujący 1 - 22
 * Przełącznik 1 - 23
 * Pin sterujący 2 - 24
 * Przełącznik 2 - 25
 * 
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Bounce2.h>

// Ustawienia sieci:
byte mac[]    = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE }; // Arduino MAC adress
IPAddress ip(192, 168, 40, 231); // Arduino IP Adress
IPAddress server(192, 168, 40, 221); // Home Assistant IP Adress

// Ustawienia pinów:
const int pinSterujacy_1 = 22;
const int pinPrzycisk_1 = 23;
const char temat_1[] = "A-231/Swiatlo_1";
const char tematSub_1[] = "A-231/Swiatlo_1/command";
boolean stanSterujacy_1 = 0;
boolean stanPrzycisk_1 = 0;

const int pinSterujacy_2 = 24;
const int pinPrzycisk_2 = 25;
const char temat_2[] = "A-231/Swiatlo_2";
const char tematSub_2[] = "A-231/Swiatlo_2/command";
boolean stanSterujacy_2 = 0;
boolean stanPrzycisk_2 = 0;

EthernetClient ethClient;
PubSubClient client(ethClient);

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  //int test = atoi((char *)payload);
  if(strcmp(topic, tematSub_1)==0){
    digitalWrite(pinSterujacy_1, atoi((char *)payload));
    client.publish(temat_1, String(digitalRead(pinSterujacy_1)).c_str(), true);
  }
  if(strcmp(topic, tematSub_2)==0){
    digitalWrite(pinSterujacy_2, atoi((char *)payload));
    client.publish(temat_2, String(digitalRead(pinSterujacy_2)).c_str(), true);
  }
  
  //Serial.print("Payload: ");
  //Serial.println(String(test).c_str());
}

//PubSubClient client(server, 1883, callback, ethClient);

long lastReconnectAttempt = 0;

boolean reconnect() {
  if (client.connect("Ard-231", "mqtt_broker", "kCi6NO8mH8QS87Cu1msA")) {
    client.publish(temat_1, String(digitalRead(pinSterujacy_1)).c_str(), true);
    client.publish(temat_2, String(digitalRead(pinSterujacy_2)).c_str(), true);
    delay(150);
    client.subscribe(tematSub_1);
    client.subscribe(tematSub_2);
    Serial.println("MQTT connected");
  }
  return client.connected();
}

void setup(){
  pinMode(pinSterujacy_1, OUTPUT);
  pinMode(pinPrzycisk_1, INPUT);
  pinMode(pinSterujacy_2, OUTPUT);
  pinMode(pinPrzycisk_2, INPUT);
  Serial.begin(9600);
  client.setServer(server,1883);
  client.setCallback(callback);
  Ethernet.begin(mac, ip);
  client.setBufferSize(255);
  delay(1500);
  lastReconnectAttempt = 0;
}

void loop(){
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      Serial.println("MQTT disconected");
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    client.loop();
  }
  if(digitalRead(pinPrzycisk_1) != stanPrzycisk_1){
    stanPrzycisk_1 = !stanPrzycisk_1;
    if(stanPrzycisk_1){
      digitalWrite(pinSterujacy_1, !digitalRead(pinSterujacy_1));
      if (client.connected()) {
        client.publish(temat_1, String(digitalRead(pinSterujacy_1)).c_str(), true);
      }
    }
  }
  if(digitalRead(pinPrzycisk_2) != stanPrzycisk_2){
    stanPrzycisk_2 = !stanPrzycisk_2;
    if(stanPrzycisk_2){
      digitalWrite(pinSterujacy_2, !digitalRead(pinSterujacy_2));
      if (client.connected()) {
        client.publish(temat_2, String(digitalRead(pinSterujacy_2)).c_str(), true);
      }
    }
  }
  delay(10);
}
