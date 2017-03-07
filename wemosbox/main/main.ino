// Include the libraries we need
#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <DNSServer.h>

#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <ArduinoJson.h>

#define ssid      // WiFi SSID
#define password      // WiFi password


// DHT Config ***************************************************************
#define DHTPIN D3
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// MQTT Config **************************************************************
#define mqtt_server    "192.168.1.10"
#define mqtt_port      1883
// #define mqtt_user   "mqtt_user"
// #define mqtt_pass   "mqtt_pass"
#define out_temp_topic "home/livingroom/temperature"
#define out_hum_topic  "home/livingroom/humidity"

// DS18B20 Sensor Config *****************************************************
// Data wire is plugged into port 2 on the Arduino 
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(D2);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

// To handle time between 2 reads
const int sensorPolling = 30000;
unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
unsigned long previousPoll = 0;   // will store last time the LED was updated

char data[80];

// ESP Config ******************************************************************
WiFiClient espClient;
PubSubClient client(espClient);

// Web server running on port
ESP8266WebServer server (80);

// Measurement *****************************************************************
float t = 0;
float h = 0;


// Setup function. Here we do the basics **************************************
void setup(void) {

  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT);

  Serial.println("Dallas Temperature IC Control Library Demo");
  WiFiManager wifiManager;

  // locate devices on the bus
  Serial.print("Locating devices ... ");
  sensors.begin();
  Serial.print(" Found ");
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

  // Initialize DHT Sensor
  dht.begin();

  wifiManager.autoConnect(ssid, password);
  // Connexion WiFi établie / WiFi connexion is OK
  Serial.println("");
  Serial.print("Connected to "); Serial.println ( ssid );
  Serial.print("IP address: "); Serial.println ( WiFi.localIP() );

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // On branche la fonction qui gère la premiere page / link to the function that manage launch page
  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

// *************************************************************************************************
String getPage() {

  String page = "";
  page += "<!DOCTYPE html>";
  page += "<html lang='en'>";
  page += "  <head>";
  page += "    <meta charset='utf-8'>";
  page += "    <meta http-equiv='X-UA-Compatible' content='IE=edge'>";
  page += "    <meta name='viewport' content='width=device-width, initial-scale=1'>";
  page += "";
  page += "    <title>Wemos D1 Mini Pro</title>";
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
  page += "         Wemos D1  Mini Pro: Test interface";
  page += "       </h1>";
  page += "     </div>";
  page += "     <ul class='nav nav-pills'>";
  page += "       <li class='active'>";
  page += "          <a href='#'>Temperature : ";
  page += t;
  page += "</a>";
  page += "       </li>";
  page += "       <li class='active'>";
  page += "          <a href='#'>Humidity : ";
  page += h;
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

// *******************************************************************************************
void handleRoot() {
  server.send ( 200, "text/html", getPage() );
}

// Publishes temperature to MQTT server *************************************************************  
void publishTemperature(float p_temperature) {

  // create a JSON object
  // doc : https://github.com/bblanchon/ArduinoJson/wiki/API%20Reference
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  // INFO: the data must be converted into a string; a problem occurs when using floats...
  root["temperature"] = (String)p_temperature;
  root.prettyPrintTo(Serial);
  Serial.println("");
  /*
     {
        "temperature": "19.40"
     }
  */
  char data[100];
  root.printTo(data, root.measureLength() + 1);
  client.publish(out_temp_topic, data, true);
}

// Publishes humidity to MQTT server *************************************************************  
void publishHumidity(float p_humidity) {

  // create a JSON object
  // doc : https://github.com/bblanchon/ArduinoJson/wiki/API%20Reference
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  // INFO: the data must be converted into a string; a problem occurs when using floats...
  root["humidity"] = (String)p_humidity;
  root.prettyPrintTo(Serial);
  Serial.println("");
  /*
     {
        "humidity": "19.40"
     }
  */
  char data[100];
  root.printTo(data, root.measureLength() + 1);
  client.publish(out_hum_topic, data, true);
}

// Main function *****************************************************************************
// Handles time between readings and publishes to MQTT
void loop(void) {

  currentMillis = millis();
  server.handleClient();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // if delay expired then read temp and humidity
  if (currentMillis - previousPoll >= sensorPolling) {
    Serial.print("Requesting temperature ... ");
    sensors.requestTemperatures(); // Send the command to get temperatures
    Serial.println("DONE");
    t = sensors.getTempC(insideThermometer);
    Serial.print("Requesting humidity ... ");
    h = dht.readHumidity();
    previousPoll += sensorPolling;
    
    if (isnan(h)) {
      Serial.println("ERROR: Failed to read from DHT sensor!");
      return;
    } else {
      Serial.println("DONE");
      publishTemperature(t);
      publishHumidity(h);
    }    
  }
}

// Prints a device address **************************************************************************
void printAddress(DeviceAddress deviceAddress) {
  
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// Handles received MQTT message ********************************************************************
void callback(char* topic, byte* payload, unsigned int length) {
}

// Reconnects to MQTT server **************************************************************************
void reconnect() {
  
  // Loop until we're reconnected
  while (!client.connected()) {
    
    Serial.print("Attempting MQTT connection ... ");
    // Create a random client ID
    String clientId = "ESP8266Client-Living";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    
    if (client.connect(clientId.c_str())) {
      Serial.println("CONNECTED");
      
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
