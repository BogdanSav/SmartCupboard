#include <ESP8266WiFi.h>

const char* ssid = "WR-Sydor5";
const char* password = "IRENA1978";

const char* host = "stag.track-debts.com";


void setup()
{
  Serial.begin(115200);
  Serial.println();

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
}


void loop()
{
  WiFiClient client;

  Serial.printf("\n[Connecting to %s ... ", host);
  if (client.connect(host, 8080))
  {
	
    Serial.println("connected]");

    Serial.println("[Sending a request]");

    String data = "[{\"createdAt\": 0,\"itemId\": \"esp\",\"weight\": 0}]"; // ok
	    
    client.println("POST /api/devices/deviceF123/events HTTP/1.1"); // ok
    client.println("Host: stag.track-debts.com"); // ok
    client.println("Accept: */*"); // ok
    client.println("Content-Type: application/json"); // ok
    
	client.print("Content-Length: "); // ok
    client.println(data.length()); // ok
	
    client.println(); // ok
    client.println(data); // ok
                

    Serial.println("[Response:]");
    while (client.connected() || client.available())
    {
      if (client.available())
      {
// /       String line = client.readStringUntil('\n');
//        Serial.println(line);

		  char str=client.read();
		  Serial.print(str);
      }
    }
    client.stop();
    Serial.println("\n[Disconnected]");
  }
  else
  {
    Serial.println("connection failed!]");
    client.stop();
  }
  delay(5000);
}
