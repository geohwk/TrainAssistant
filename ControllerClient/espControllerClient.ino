#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <vector>

// WiFi credentials
const char * wifi_ssid = "TrainControl";
const char * wifi_password = "ringousel";

//MQTT Broker
const char* mqtt_broker = "192.168.1.7";
const int mqtt_port = 1883;
const char* brokerUser = "";  // exp: myemail@mail.com
const char* brokerPass = "";

// MQTT topics
std::vector<String> topics;
int currentTopicIndex = 0; // Index of the currently selected topic

// GPIO pins for buttons

const int removePin = D2; //Remove client from list
const int swapPin = D3; //Change selected client
const int rfPin = D4;
const int light1Pin = D5;
const int light2Pin = D6;

// Potentiometer pin
const int potentiometerPin = A0;

// Initialize the WiFi and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

//Switch States
bool RFcurrentState = false;
bool lights1State = false;
bool lights2State = false;

void setup() 
{
    Serial.begin(115200);

    // Initialize buttons as inputs
    pinMode(removePin, INPUT);
    pinMode(swapPin, INPUT);
    pinMode(rfPin, INPUT);
    pinMode(rfPin, INPUT);
    pinMode(light1Pin, INPUT);
    pinMode(light2Pin, INPUT);

    // Connect to Pi Access Point
    connectToWiFi();
    
    //Connect to MQTT broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);

    while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), "", "")) {
          Serial.println("Public emqx mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
    }
    client.subscribe("newClient");
}

void connectToWiFi() {
  Serial.printf("Connecting to '%s'\n", wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Connection Failed!");
  }
}

void removeEntry() {
  if (!topics.empty()) {
    topics.erase(topics.begin() + currentTopicIndex);
    if (currentTopicIndex >= topics.size()) {
      currentTopicIndex = topics.size() - 1;
    }
  }
  Serial.println("Removed Client from Controller");
}

void swapEntry() {
  currentTopicIndex++;
  if (currentTopicIndex >= topics.size()) {
    currentTopicIndex = 0;
  }
  Serial.println(("Swapped to new topic: " + topics[currentTopicIndex]).c_str());
}

void setToForward() {
  Serial.println("Train command: Forward");
}

void setToReverse() {
  Serial.println("Train command: Reverse");
}

void lights1On(){
  Serial.println("Train command: Lights 1 On");
}

void lights1Off(){
  Serial.println("Train command: Lights 1 Off");
}

void lights2On(){
  Serial.println("Train command: Lights 2 On");
}

void lights2Off(){
  Serial.println("Train command: Lights 2 Off");
}


void readSpeed(){
  int potValue = analogRead(potentiometerPin);
  float speedValue = (potValue/1023)*100;
  Serial.println(speedValue);

  // Publish potentiometer value to the currently selected speed subtopic
  //client.publish((topics[currentTopicIndex] + "/speed").c_str(), String(speedValue).c_str());
}

//New Client connected
void callback(char *topic, byte *payload, unsigned int length){
  Serial.println("New Client Added to network:");
  String str;
  for (int i = 0; i < length; i++) {
      str += byteArray[i];
  }
  Serial.println(str);
  //Adding client to array
  topics.push_back(str);
}

void loop() 
{
  client.loop();

  readSpeed();

  // Check button presses and perform corresponding actions
  if (digitalRead(removePin) == HIGH) {
    //Remove Entry
    removeEntry();
  }
  if (digitalRead(swapPin) == HIGH) {
    //Swap Selected Entry
    swapEntry();
  }
  if (digitalRead(rfPin) == HIGH) && (RFcurrentState == false) {
    //Reverse-Forward
    setToForward();
    RFcurrentState = HIGH
  }
  if (digitalRead(rfPin) == LOW) && (RFcurrentState == true) {
    //Reverse-Forward
    setToReverse();
    RFcurrentState = LOW
  }

  //Lights
  if (digitalRead(light1Pin) == HIGH) && (lights1State == false) {
    lights1On();
    lights1State = true
  }
  if (digitalRead(light1Pin) == LOW) && (lights1State == true){
    lights1Off();
    lights1State = false
  }
  if (digitalRead(light2Pin) == HIGH) && (lights2State == false){
    lights2On();
    lights2State = true
  }
  if (digitalRead(light2Pin) == LOW) && (lights2State == true){
    lights2Off();
    lights2State = false
  }

}