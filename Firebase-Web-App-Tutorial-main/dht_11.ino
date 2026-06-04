#include <ESP8266WiFi.h>
 #include "DHT.h"        // DHT11 temperature and humidity sensor Predefined library
 
#define DHTTYPE DHT11   // DHT 11
#define dht_dpin 0      //GPIO-0 D3 pin of nodemcu

const char* ssid = "Your SSID";
const char* password = "Your Wifi Password";


DHT dht(dht_dpin, DHTTYPE); 
WiFiServer server(80);

void setup(void)
{ 
  dht.begin();
  Serial.begin(9600);
  delay(10);
  
  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);     //Begin WiFi
  
 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address on serial monitor
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");    //URL IP to be typed in mobile/desktop browser
  Serial.print(WiFi.localIP());
  Serial.println("/");
}



void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  } 
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

 float h =0.0;  //Humidity level
 float t =0.0;  //Temperature in celcius 
 float f =0.0;  //Temperature in fahrenheit
 
  // Match the request
  int value = LOW;
  if (request.indexOf("/Up=ON") != -1)  {
     h = dht.readHumidity();    //Read humidity level
     t = dht.readTemperature(); //Read temperature in celcius
     f = (h * 1.8) + 32;        //Temperature converted to Fahrenheit
    value = HIGH;
  }

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<h1 align=center>Temperature & Humidity Monitoring with DHT11</h1><br><br>");
  client.print("Temperature in Celsius =");
  client.println(t);
  client.println("<br>");
  client.print("Temperature in Fahrenheit =");
  client.println(f);
  client.println("<br>");
  client.print("Humidity =");
  client.println(h);
  client.print(" %");   
  client.println("<br>");  
  client.println();
 
  if(value == HIGH) {
    client.println("Updated");
  } else {
    client.print("Not Updated");
  }
  client.println("<br><br>");
  client.println("<a href=\"/Up=ON\"\"><button>Update Temperature & Humidity</button></a><br />");  
  client.println("</html>");
  delay(1);
  Serial.println("Client disconnected");
  Serial.println("");
}
