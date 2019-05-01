#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

#include <ArduinoJson.h>
#include <EEPROM.h>

#define RST_PIN         5           // Configurable, see typical pin layout above
#define SS_PIN          4          // Configurable, see typical pin layout above




// Update these with values suitable for your network.

const char* ssid = "WR-Sydor5";
const char* password = "IRENA1978";
const char* mqtt_server = "stag.track-debts.com";

WiFiClient espClient;
ESP8266WiFiClass wifi;
PubSubClient client(espClient);
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
long lastMsg = 0;
char msg[50];
int value = 0;
uint addr = 0;
char* uuid;

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
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String JSONMessage;
  for (int i = 0; i < length; i++) {
      JSONMessage = (String)payload[i];

  }
 
 // char JSONMessage[] = "{\"macAddress\":\"5G-67-NJ-L51\",\"uuid\":\"bfc8b00b-7de7-4ce0-8719-1d63d2773b04\",\"createdAt\":1556232469976,\"title\":\"Test #2\"}";
  StaticJsonDocument <200> doc;

  DeserializationError error = deserializeJson(doc, JSONMessage);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
 String macad =doc["macAddress"];
  String title =doc["title"];
  String uud =doc["uuid"];

  Serial.println(macad);
  Serial.println(title);
  Serial.println(uud);
  


   String mac = wifi.macAddress();
	   String macAddress = doc["macAddress"];
  if(mac == macAddress){
    uuid == doc["uuid"];
    Serial.println(uuid);
    EEPROM.commit();

  }
  else
  {
    Serial.println("mac's is not equals");
    Serial.println(mac);
    Serial.println(macAddress);
  }
  

 /* StaticJsonBuffer<300> JSONBuffer;
  Object&  parsed= JSONBuffer.parseObject(JSONMessage);
  if (!parsed.success()) { 
    Serial.println("Parsing failed");
    return; 
  }
  else {
	  String mac = wifi.macAddress();
	  const char * macAddress = parsed["macAddress"];
	  if (macAddress == mac) {
	 	 uuid = parsed["uuid"];
	     
		 
		 // replace values in byte-array cache with modified data
	     // no changes made to flash, all in local byte-array cache
	     EEPROM.put(addr,data);

	     // actually write the content of byte-array cache to
	     // hardware flash.  flash write occurs if and only if one or more byte
	     // in byte-array cache has been changed, but if so, ALL 512 bytes are 
	     // written to flash
	       
		 Serial.println("uuid for this device - " + String(uuid));
      }
  }*/
  
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  

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
     // client.publish("esp/","2");
      // ... and resubscribe
      //client.subscribe("esp/");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
void read_card(){
  if ( ! mfrc522.PICC_IsNewCardPresent())
        return;

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;

    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
    mfrc522.PICC_HaltA();       // Halt PICC
    mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}*/
void regist() {

     // read bytes (i.e. sizeof(data) from "EEPROM"),
     // in reality, reads from byte-array cache
     // cast bytes into structure called data

   EEPROM.get(addr,uuid);
   if (uuid == NULL) { // uuid isEmpty	
   	 	String mac = wifi.macAddress();
   		String data = "{\"macAddress\":\"";
   		data += mac;
   		data += "\",\"title\": \"rc522\"}";
   		Serial.println(data);
   		Serial.println(client.subscribe("esp/token")); 
   		client.publish("esp/12/registration",data.c_str());
  }
}

void connect(){

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  /*long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 50, wifi.macAddress().c_str(), value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("esp/", msg);
    
  }*/
 

 }

void setup() {
  
  //pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  
  // commit 512 bytes of ESP8266 flash (for "EEPROM" emulation)
  // this step actually loads the content (512 bytes) of flash into 
  // a 512-byte-array cache in RAM
  EEPROM.begin(512);
  
  
  setup_wifi();
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);
   while (!Serial);            // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();                // Init SPI bus
    mfrc522.PCD_Init();         // Init MFRC522 card
    Serial.println(F("Try the most used default keys to print block 0 of a MIFARE PICC."));
}

int i = 0;

void loop() {

 connect();
 regist();
 //read_card();
/* char JSONMessage[] = "{\"macAddress\":\"5G-67-NJ-L51\",\"uuid\":\"bfc8b00b-7de7-4ce0-8719-1d63d2773b04\",\"createdAt\":1556232469976,\"title\":\"Test #2\"}";
  StaticJsonDocument <200> doc;

  DeserializationError error = deserializeJson(doc, JSONMessage);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
   String mac = wifi.macAddress();
	  String macAddress = doc["macAddress"];
  if(mac == macAddress){
    uuid == doc["uuid"];

  }
  else
  {
    Serial.println("mac's is not equals");
    Serial.println(mac);
    Serial.println(macAddress);
  }*/
}