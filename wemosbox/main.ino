// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DNSServer.h>

#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#define ssid             // WiFi SSID
#define password    // WiFi password


#define mqtt_server       "mqtt.office.talpau"
#define mqtt_port         "12345"
#define mqtt_user         "mqtt_user"
#define mqtt_pass         "mqtt_pass"
#define out_topic    "sensor1/temperature"
#define in_topic     "sensor1/command"
#define lwt_topic    "sensor1/lwt"
#define status_topic "sensor1/status"

// Data wire is plugged into port 2 on the Arduino 
float   t = 0 ;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(D2);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;
//
const int sensorPolling = 30000;
unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
unsigned long previousPoll = 0;   // will store last time the LED was updated

char data[80];


WiFiClient espClient;
PubSubClient client(espClient);
// Web server running on port

ESP8266WebServer server ( 80 );
/*
   Setup function. Here we do the basics
*/
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(D1,OUTPUT);
  digitalWrite(D1, HIGH);
  Serial.println("Dallas Temperature IC Control Library Demo");
  WiFiManager wifiManager;
  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");


  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 10);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();

  wifiManager.autoConnect(ssid, password );
  // Connexion WiFi établie / WiFi connexion is OK
  Serial.println ( "" );
  Serial.print ( "Connected to " ); Serial.println ( ssid );
  Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );

  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);

  // On branche la fonction qui gère la premiere page / link to the function that manage launch page
  server.on ( "/", handleRoot );

  server.begin();
  Serial.println ( "HTTP server started" );

}
String getPage() {
  String page = "";
  page += "<!DOCTYPE html>";
  page += "<html lang='en'>";
  page += "  <head>";
  page += "    <meta charset='utf-8'>";
  page += "    <meta http-equiv='X-UA-Compatible' content='IE=edge'>";
  page += "    <meta name='viewport' content='width=device-width, initial-scale=1'>";
  page += "";
  page += "    <title>Wemos D1</title>";
  page += "";
  page += "    <meta name='description' content='Wemos Test'>";
  page += "    <meta name='author' content='LayoutIt!'>";
  page += "";
  page += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'></link>";
  page += "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script>";
  page += "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script>";
  page += "";
  page += "  </head>";
  page += "  <body>";
  page += "";
  page += "    <div class='container-fluid'>";
  page += "  <div class='row'>";
  page += "   <div class='col-md-12'>";
  page += "     <div class='page-header'>";
  page += "       <h1>";
  page += "         Wemos D1 : Test interface";
  page += "       </h1>";
  page += "     </div>";
  page += "     <ul class='nav nav-pills'>";
  page += "       <li class='active'>";
  page += "          <a href='#'>Température : ";
  page += t;
  page += "</a>";
  page += "       </li>";
  page += "     </ul>";
  page += "   </div>";
  page += " </div>";
  page += "</div>";
  page += "";
  page += "    <script src='js/jquery.min.js'></script>";
  page += "    <script src='js/bootstrap.min.js'></script>";
  page += "    <script src='js/scripts.js'></script>";
  page += "  </body>";
  page += "</html>";
  return page;
}
void handleRoot() {
  server.send ( 200, "text/html", getPage() );
}



void updateTemperature(DeviceAddress deviceAddress) {
  if (currentMillis - previousPoll >= sensorPolling) {
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures(); // Send the command to get temperatures
    Serial.println("DONE");
    t = sensors.getTempC(deviceAddress);
    Serial.print("Temp C: ");
    Serial.print(t);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(t)); // Converts tempC to Fahrenheit
    //update polling
    
    previousPoll += sensorPolling;
     String payload = "{ \"temperature\": ";
     payload+=t;
     payload+=+ " }";
     payload.toCharArray(data, (payload.length() + 1));
    
    client.publish(out_topic, data, true);
  }
}
/*
   Main function. It will request the tempC from the sensors and display on Serial.
*/
void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  currentMillis = millis();
  server.handleClient();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //delay(6000);
  updateTemperature(insideThermometer);

}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(D1, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
    client.publish(status_topic,"1",true);
  } else {
    digitalWrite(D1, HIGH);  // Turn the LED off by making the voltage HIGH
    client.publish(status_topic,"0",true);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-salon";
    clientId += String(random(0xffff), HEX);
    String lwt_msg = "1";
    // Attempt to connect
    if (client.connect(clientId.c_str(),lwt_topic,1,true,lwt_msg.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(out_topic, "{ \"temperature\": 15 }");
         // ... and resubscribe
      client.subscribe(in_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
