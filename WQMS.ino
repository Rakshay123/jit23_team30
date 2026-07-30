#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include "ThingSpeak.h"   // always include thingspeak header file after other header files and custom macros

char ssid[] = "SSID";   // your network SSID (name)
char pass[] = "PASSWORD";           // your network password
int keyIndex = 0;                     // your network key Index number (needed only for WEP)
WiFiClient client;

unsigned long myChannelNumber = ID ;
const char *myWriteAPIKey = "API ";

#define ONE_WIRE_BUS 4

#define PH_PIN 34
#define TDS_PIN 35
#define TURBIDITY_PIN 32
#define WATER_LEVEL_PIN 33

// Solenoid Valve Pins
#define CLEAN_VALVE 26
#define MUDDY_VALVE 27

// ---------------- Threshold Values ----------------
const float PH_MIN = 6.5;
const float PH_MAX = 8.5;

const float TDS_MAX = 500.0;        // ppm
const int TURBIDITY_MAX = 1500;     // Adjust after calibration
const float TEMP_MIN = 20.0;        // °C
const float TEMP_MAX = 35.0;        // °C
const int WATER_LEVEL_MIN = 1000;   // Adjust according to your sensor
// ---------------------------------------------------

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {

  Serial.begin(115200);

  sensors.begin();

  pinMode(CLEAN_VALVE, OUTPUT);
  pinMode(MUDDY_VALVE, OUTPUT);

  digitalWrite(CLEAN_VALVE, LOW);
  digitalWrite(MUDDY_VALVE, LOW);

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
}

void loop() {

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {

    Serial.print("Attempting to connect to SSID: ");

    while (WiFi.status() != WL_CONNECTED) {

      WiFi.begin(ssid, pass);

      Serial.print(".");

      delay(5000);
    }

    Serial.println("\nConnected.");
  }

  // ---------------- Temperature ----------------
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  // ---------------- pH ----------------
  int phRaw = analogRead(PH_PIN);
  float voltagePH = phRaw * (3.3 / 4095.0);
  float pH = 7 + ((2.5 - voltagePH) / 0.18);

  // ---------------- TDS ----------------
  int tdsRaw = analogRead(TDS_PIN);
  float voltageTDS = tdsRaw * (3.3 / 4095.0);

  float tds = (133.42 * voltageTDS * voltageTDS * voltageTDS
               - 255.86 * voltageTDS * voltageTDS
               + 857.39 * voltageTDS) * 0.5;

  // ---------------- Turbidity ----------------
  int turbidity = analogRead(TURBIDITY_PIN);

  // ---------------- Water Level ----------------
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

  // ---------------- Decision Making ----------------
  bool cleanWater = true;

  if (pH < PH_MIN || pH > PH_MAX)
    cleanWater = false;

  if (tds > TDS_MAX)
    cleanWater = false;

  if (turbidity > TURBIDITY_MAX)
    cleanWater = false;

  if (temp < TEMP_MIN || temp > TEMP_MAX)
    cleanWater = false;

  if (waterLevel < WATER_LEVEL_MIN)
    cleanWater = false;

  // ---------------- Solenoid Control ----------------
  if (cleanWater) {

    digitalWrite(CLEAN_VALVE, HIGH);
    digitalWrite(MUDDY_VALVE, LOW);

    Serial.println("STATUS : CLEAN WATER");
    Serial.println("Clean Water Valve : ON");
    Serial.println("Muddy Water Valve : OFF");

  } else {

    digitalWrite(CLEAN_VALVE, LOW);
    digitalWrite(MUDDY_VALVE, HIGH);

    Serial.println("STATUS : MUDDY WATER");
    Serial.println("Clean Water Valve : OFF");
    Serial.println("Muddy Water Valve : ON");
  }

  Serial.println("==========================");

  // ---------------- ThingSpeak ----------------
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, pH);
  ThingSpeak.setField(3, tds);
  ThingSpeak.setField(4, turbidity);
  ThingSpeak.setField(5, waterLevel);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (x == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  delay(2000);
}