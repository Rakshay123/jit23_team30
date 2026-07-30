#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

char ssid[] ="" ;   // your network SSID (name) 
char pass[] = "";   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = ID;
const char * myWriteAPIKey = "API";

#define ONE_WIRE_BUS 4

#define PH_PIN 34
#define TDS_PIN 35
#define TURBIDITY_PIN 32
#define WATER_LEVEL_PIN 33

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  sensors.begin();
   WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
// Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
   
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }


  // Temperature
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  // pH
  int phRaw = analogRead(PH_PIN);
  float voltagePH = phRaw * (3.3 / 4095.0);
  float pH = 7 + ((2.5 - voltagePH) / 0.18);

  // TDS
  int tdsRaw = analogRead(TDS_PIN);
  float voltageTDS = tdsRaw * (3.3 / 4095.0);
  float tds = (133.42 * voltageTDS * voltageTDS * voltageTDS
             - 255.86 * voltageTDS * voltageTDS
             + 857.39 * voltageTDS) * 0.5;

  // Turbidity
  int turbidity = analogRead(TURBIDITY_PIN);

  // Water Level
  int waterLevel = analogRead(WATER_LEVEL_PIN);

  Serial.println("==========================");

  Serial.print("Temperature : ");
  Serial.print(temp);
  Serial.println(" °C");

  Serial.print("pH : ");
  Serial.println(pH);

  Serial.print("TDS : ");
  Serial.print(tds);
  Serial.println(" ppm");

  Serial.print("Turbidity : ");
  Serial.println(turbidity);

  Serial.print("Water Level : ");
  Serial.println(waterLevel);

  Serial.println("==========================");
    // set the fields with the values
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, pH);
  ThingSpeak.setField(3, tds);
  ThingSpeak.setField(4, turbidity);
  ThingSpeak.setField(5, waterLevel);

   // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }


  delay(2000);
}