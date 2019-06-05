#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <Timer.h>

#include <ArduinoJson.h>
#include <EEPROM.h>

// Update these with values suitable for your network.

const char *ssid = "";
const char *password = "";
const char *mqtt_server = "smart-cupboard.kolegran.com";

WiFiClient espClient;
ESP8266WiFiClass wifi;
PubSubClient client(espClient);
SoftwareSerial softSerial(13, 15);

uint addr = 0;
String uuid;
String mac = wifi.macAddress();
Timer t;

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
      Serial.println("uuid for this device - " + String(uuid));
    }
  }

  Serial.println();
}

void reconnect()
{

  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      String successChannel = "esp/success";
      String errorChannel = "esp/error";

      client.subscribe(successChannel.c_str());
      client.subscribe(errorChannel.c_str());
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}


String readFirstItem() {
  String itemValue = "INSERT_REAL_DATA_1";

  //INSERT HERE LOGIC FOR READ ITEM
  return itemValue;
}

String readSecondItem() {
  String itemValue = "INSERT_REAL_DATA_2";

  //INSERT HERE LOGIC FOR READ ITEM
  return itemValue;
}

String readThirdItem() {
  String itemValue = "INSERT_REAL_DATA_3";

  //INSERT HERE LOGIC FOR READ ITEM
  return itemValue;
}


void sendData()
{

  StaticJsonDocument<900> doc;
  String Jsondoc;
  doc["deviceId"] = "80:8D:8A:8E:BE:88";
  JsonArray readers = doc.createNestedArray("readers");

  JsonObject readerFirst = readers.createNestedObject();
  readerFirst["readerId"] = "RC-522-1";
  JsonArray firstItemArray = readerFirst.createNestedArray("items");
  JsonObject innerFirstItem = firstItemArray.createNestedObject();

  JsonObject readerSecond = readers.createNestedObject();
  readerSecond["readerId"] = "RC-522-2";
  JsonArray secondItemArray = readerSecond.createNestedArray("items");
  JsonObject innerSecondaryItem = secondItemArray.createNestedObject();

  JsonObject readerThird = readers.createNestedObject();
  readerThird["readerId"] = "RC-522-3";
  JsonArray thirdItemArray = readerThird.createNestedArray("items");
  JsonObject innerThirdItem = thirdItemArray.createNestedObject();

  boolean debugMode = true;      //USE THIS VALUE FOR DEBUG WITH FAKE DATA
  if (debugMode) {
    innerFirstItem["rfid"] = "A00152045B";
    innerSecondaryItem["rfid"] = "EB2561739S";
    innerThirdItem["rfid"] = "N45651212E";
  }
  else {
    innerFirstItem["rfid"] = readFirstItem();
    innerSecondaryItem["rfid"] = readSecondItem();
    innerThirdItem["rfid"] = readThirdItem();
  }

  serializeJson(doc, Jsondoc);
  Serial.println("---------------------------");
  Serial.println(Jsondoc);
  Serial.println("---------------------------");

  String path = "esp/80:8D:8A:8E:BE:88/event";

  client.publish(path.c_str(), Jsondoc.c_str());
}

void regist()
{
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
}

void setup()
{

  Serial.flush();
  softSerial.begin(9600);
  Serial.begin(9600);

  EEPROM.begin(512);

  t.every(5000, connect);
  t.every(10000, sendData);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("Try some rfid tags");
}

void loop()
{
  t.update();
  client.loop();
}