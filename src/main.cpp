#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <Ticker.h>

#include <ArduinoJson.h>
#include <EEPROM.h>

// Update these with values suitable for your network.

const char *ssid = "WR-Sydor5";
const char *password = "IRENA1978";
const char *mqtt_server = "stag.track-debts.com";

WiFiClient espClient;
ESP8266WiFiClass wifi;
PubSubClient client(espClient);
SoftwareSerial softSerial(13, 15);

uint addr = 0;
String uuid;
String mac = wifi.macAddress();

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void writeString(char add, String data)
{
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  EEPROM.commit();
}

String read_String(char add)
{
  Serial.println("Start reading EEPROM");
  int len = EEPROM.read(add);
  if (len == 255)
  {
    //Empty
    return "";
  }
  Serial.println("Length: " + String(len));
  char *buf = new char[len + 1];
  add++;
  for (int i = 0; i < len; ++i)
  {
    char c = char(EEPROM.read(add + i));
    Serial.print(c);
    buf[i] = c;
  }
  Serial.println();
  EEPROM.end();
  buf[len] = '\x0';
  return String(buf);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }

  char *cPayload = (char *)payload;
  //String JSONMessage[] = "{\"macAddress\":\"5G-67-NJ-L51\",\"uuid\":\"bfc8b00b-7de7-4ce0-8719-1d63d2773b04\",\"createdAt\":1556232469976,\"title\":\"Test #2\"}";
  StaticJsonDocument<300> JSONBuffer;
  //JsonObject  parsed= JSONBuffer.parseObject(JSONMessage);
  auto error = deserializeJson(JSONBuffer, cPayload);
  if (error)
  {
    Serial.println("Parsing failed");
    return;
  }
  else
  {
    String mac = wifi.macAddress();
    const char *macAddress = JSONBuffer["macAddress"].as<char *>();
    if ((String)macAddress == mac)
    {
      uuid = JSONBuffer["uuid"].as<String>();
      Serial.println(macAddress);
      Serial.println(uuid);
      writeString(addr, uuid);

      // replace values in byte-array cache with modified data
      // no changes made to flash, all in local byte-array cache

      // actually write the content of byte-array cache to
      // hardware flash.  flash write occurs if and only if one or more byte
      // in byte-array cache has been changed, but if so, ALL 512 bytes are
      // written to flash
      Serial.println("uuid for this device - " + String(uuid));
    }
  }

  Serial.println();

  // Switch on the LED if an 1 was received as first character

  // Switch on the LED if an 1 was received as first character
}

void reconnect()
{

  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("esp/","2");
      // ... and resubscribe
      //client.subscribe("esp/");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void sendData()
{
  StaticJsonDocument<900> doc;
  
  String Jsondoc;
  doc["deviceId"] = "80:8D:8A:8E:BE:88";
  JsonArray readers = doc.createNestedArray("readers");


  JsonObject readerFirst = readers.createNestedObject();

  JsonArray firstItemArray = readerFirst.createNestedArray("items");
  JsonObject innerFirstItem = firstItemArray.createNestedObject();
  innerFirstItem["rfid"] = "A00152045B";
  readerFirst["readerId"] =  "RC-522-1";

  JsonObject readerSecond = readers.createNestedObject();

  JsonArray secondItemArray = readerSecond.createNestedArray("items");
  JsonObject innerSecondaryItem = secondItemArray.createNestedObject();
  innerSecondaryItem["rfid"] = "EB2561739S";
  readerSecond["readerId"] = "RC-522-2";



  JsonObject readerThird = readers.createNestedObject();
  JsonArray thirdItemArray = readerThird.createNestedArray("items");
  JsonObject innerThirdItem = thirdItemArray.createNestedObject();
  innerThirdItem["rfid"] = "N45651212E";
  readerThird["readerId"] = "RC-522-3";

  //serializeJson(doc,Jsondoc);
  Serial.println(Jsondoc);
  

    String path = "esp/80:8D:8A:8E:BE:88/event";
  
    
    String message  = "[{\"deviceId\":\"80:8D:8A:8E:BE:88\",\"readers\": [\"items\": [{\"rfid\":\"A00152045B\"}],{\"readerId\": \"RC-522-1\"},{\"items\": [{\"rfid\":\"A00152045B\"}],{\"readerId\": \"RC-522-2\"}]}]";
  
    client.publish(path.c_str(),message.c_str());
    client.disconnect();
}

void regist()
{

  // read bytes (i.e. sizeof(data) from "EEPROM"),
  // in reality, reads from byte-array cache
  // cast bytes into structure called data
  String uuid = read_String(addr);

  Serial.print("UUID is: ");
  Serial.println(uuid);
  if (uuid == "")
  { // uuid isEmpty
    Serial.println("No UUID, starting registration process....");

    String mac = wifi.macAddress();
    StaticJsonDocument<200> doc;
    doc["macAddress"] = mac;
    doc["title"] = "rc522";
    String jsonInString;
    serializeJson(doc, jsonInString);
    Serial.println(client.subscribe("esp/token"));
    Serial.println(jsonInString);
    client.publish("esp/12/registration", jsonInString.c_str());
  }
}

void connect()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}
Ticker data(sendData, 1000, 1);
Ticker registration(regist, 1000, 1);

void setup()
{

  // Serial.flush();
  softSerial.begin(9600);
  Serial.begin(9600);

  data.start();
  registration.start();

  // commit 512 bytes of ESP8266 flash (for "EEPROM" emulation)
  // this step actually loads the content (512 bytes) of flash into
  // a 512-byte-array cache in RAM
  EEPROM.begin(512);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //while (!Serial);            // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U
  Serial.println("Try some rfid tags");
}

void loop()
{

  connect();
  data.update();
}