/*
Stolen from the "mqtt_esp8266" example from PubSubClient library,
so probably some vestigial stuff like publishing on "outTopic".
Lots of room for improvement, esp power usage!

For each song receiver module the appropriate song must be
hardcoded in char song AND client name changed on line 153.
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

Servo myservo;

// Update these with values suitable for your network.
const char* ssid = "YOUR WIFI NETWORK NAME";
const char* password = "YOUR WIFI PASSWORD";
const char* mqtt_server = "YOUR PI IP ADDRESS";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

/*////////////////////////////////////////////CHOOSE WHICH SONG RECEIVER HERE!!!
'1' = Sun
'2' = Time
'3' = Storm//consolidated with '1'
'4' = Forest//consolidated with '1'
'5' = Saria//un-needed!
'6' = Fire
'7' = Epona
'8' = Zelda*/
char song = '2';
/*Sun's Song, Song of Storms, and Minuet of Forest are
 * all executed by a single esp8266 controlling multiple relays
 */
int outPin = 2;
int stormPin = 4;
int forestPin = 5;
int pause;
int stormPause;
int forestPause;
int angle;
int neutral;
int bounce;
boolean toggle = true;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);// Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (song=='8') {//zelda
    myservo.attach(outPin);
    angle = 180;
    neutral = 30;
    bounce = 15;
    myservo.write(angle);
  }
  if (song=='7') {//epona
    myservo.attach(outPin);
    angle = 99;
    neutral = 0;
    pause = 0;
    myservo.write(neutral);
  }
  if (song=='6') {//fire
    myservo.attach(outPin);
    angle = 170;
    neutral = 70;
    bounce = 5;
    myservo.write(120);
  }
  if (song=='2') {//time
    pinMode(outPin, OUTPUT);
    digitalWrite(outPin, LOW);
    pause = 100;
  } 
  if (song=='1') {//sun, storm, and forest
    pinMode(outPin, OUTPUT);
    pinMode(stormPin, OUTPUT);
    pinMode(forestPin, OUTPUT);
    digitalWrite(outPin, HIGH);
    digitalWrite(stormPin, HIGH);
    digitalWrite(forestPin, HIGH);
    stormPause = 10000;
    forestPause = 5000;
  }   
}

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
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Do the thing if your song plays
  if ((char)payload[0]=='3'&&song=='1') {
    pinToggle(stormPin);//storms
  }
  if ((char)payload[0]=='4'&&song=='1') {
    pinHold(forestPause, forestPin);//forest
  }
  if ((char)payload[0] == song) {
    if (song=='8'||song=='6') {//Zelda or fire
      servoToggle();
    }
    if (song=='7') {//Epona
      servoHold();
    }
    if (song=='2') {//time
      pinHold(pause, outPin);
    }
    if (song=='1') {
      pinToggle(outPin);//sun
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Time")) {///////////////THIS MUST BE A UNIQUE NAME FOR EACH ESP8266
      //Time
      //Epona
      //SunStormForest
      //Fire
      //Zelda
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("songID");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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
    snprintf (msg, 75, "hello world #%ld", song);
    Serial.print("Publish message: ");
    Serial.println(msg);
    //client.publish("outTopic", msg);
  }
}

void pinToggle(int pin) {//toggle pin high/low whenever song is heard
  digitalWrite(pin, !digitalRead(pin));
}
void pinHold(int holdTime, int pin) {//hold pin low for some time when song is heard, then go back high
  digitalWrite(pin, LOW);
  delay(holdTime);
  digitalWrite(pin, HIGH);
}
void servoToggle() {//bring servo to position, bounce back a bit, hold until song plays again then move back+bounce
  int pos;
  if (toggle) {
    for(pos = angle-bounce; pos>=neutral; pos-=1)    
    {                                
      myservo.write(pos);             
      delay(10);                       
    }
    for(pos = neutral; pos<=neutral+bounce; pos +=1)
    {
      myservo.write(pos);
      delay(10);
    }
  } else {
    for(pos = neutral+bounce; pos <= angle; pos += 1)
    {                                  
      myservo.write(pos);              
      delay(10);                     
    }
    for(pos = angle; pos >= angle-bounce; pos -=1)
    {
      myservo.write(pos);
      delay(10);
    }    
  }
  toggle = !toggle; 
}
void servoHold() {//when song is heard move servo to some position then move it back to neutral
  int pos;
  for(pos = neutral; pos <= angle; pos += 1) 
  {                                 
    myservo.write(pos);              
    delay(5);                      
  } 
  //delay(pause);
  for(pos = angle; pos>=neutral; pos-=1)     
  {                                
    myservo.write(pos);            
    delay(5);                       
  } 
}

