/*
Basic MQTT Client
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "vincents";
const char* password = "2174309188";
const char* mqtt_server = "192.168.88.45";
const char* mqtt_server_topic = "shopMainControlESP";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

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

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  
  //Convert payload into char array for later conversion
  char data[length+1];
  for (int i = 0; i < length; i++) {
    data[i]=payload[i];
  }
  data[length] = '\0';

  Serial.print("New MQTT Msg [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(data);

  // Split the complet topic into the subtopic array
  char * subtopic[10]; /* allow up to 10 levels */
  int topicLevel = 0; /* begin subtopic level at 0 */
  char *token; /* location of current isolated subtopic segmenet */
  token = strtok(topic, "/"); /* attemp to find first subtopic */
  while( token != NULL ) /* loop until no more subtopics */
  {
    subtopic[topicLevel] = (char*)malloc(strlen(token) + 1); /* allocate mem space subtopic element */
    strcpy(subtopic[topicLevel],token); /* copy token into subtopic array */
    
    // log the subtopic to serial console
    Serial.print(topicLevel); 
    Serial.print(": ");
    Serial.println(subtopic[topicLevel]);
    
    topicLevel ++; /* increment topic level */
    token = strtok(NULL, "/"); /* attemp to find next subtopic */
  }

  //handle the digital topic and set outputs
  if (strcmp(subtopic[1],"digital") == 0) {
    int ioPin = atoi(subtopic[2]);
    int ioPinState = -1;
    
    //ensure setting is 0.0 to 100.0
    if (strcmp(data,"1") == 0) {
      ioPinState = LOW;
    } else if (strcmp(data,"0") == 0) {
      ioPinState = HIGH;
    } else {
      Serial.print("Something went wrong while setting pin: ");
      Serial.print(ioPin);
      Serial.print(" to ");
      Serial.println(data);
      return;
    }
    Serial.print("Setting GPIO pin ");
    Serial.print(ioPin);
    Serial.print(" to ");
    Serial.println(ioPinState);
    digitalWrite(ioPin, ioPinState);  // Turn the LED off by making the voltage HIGH
  }
    
  //handle the pwm topic and set outputs
  if (strcmp(subtopic[1],"pwm") == 0) {
    bool valid = false;
    int ioPin = atoi(subtopic[2]);
    float dutyCycle = atof(data);

    //ensure setting is between 0 to 255 and write to the pin
    if ((dutyCycle >= 0) && (dutyCycle <= 255)) {
      analogWrite(ioPin, dutyCycle);  // Turn the LED off by making the voltage HIGH
      Serial.print("Setting PWM on pin ");
      Serial.print(ioPin);
      Serial.print(" to ");
      Serial.println(dutyCycle);
      return;
    } else
    //Something went wrong with PWM data
    Serial.print("Issue setting PWM on pin ");
    Serial.print(ioPin);
    Serial.print(" to ");
    Serial.println(dutyCycle);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("shopMainControlESP/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 50, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}
