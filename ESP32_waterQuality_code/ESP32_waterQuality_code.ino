#define USE_WIFI true // True means to use wifi and false means to use GPRS Network
#define TINY_GSM_MODEM_SIM7000

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
// #define TINY_GSM_YIELD() { delay(2); }

// Define how you're planning to connect to the internet
// These defines are only for this example; they are not needed in other code.
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false


// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[]      = "web.vodafone.de";
const char gprsUser[] = "";
const char gprsPass[] = "";
//MQTT Client credential
const char* mqttusername = "username";
const char* mqttpassword = "password";
// Your WiFi connection credentials, if applicable
const char* ssid     = "ssid";
const char* password = "password";

// MQTT details
const char *broker = "gate.bigdata.fh-aachen.de";

const char *topic       = "waterquality/sensor";
#include <ArduinoJson.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <WiFi.h>
// Just in case someone defined the wrong thing..
#if TINY_GSM_USE_GPRS && not defined TINY_GSM_MODEM_HAS_GPRS
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI true
#endif
#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI true
#endif
//#if !USE_WIFI
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif
TinyGsmClient client(modem);
//#endif
#if !USE_WIFI
PubSubClient  mqtt(client);
#else
WiFiClient wifiClient;
PubSubClient  mqtt(wifiClient);
#endif
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  60          // Time ESP32 will go to sleep (in seconds)

#define UART_BAUD   115200
#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4

#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13
#define LED_PIN     12

bool gpsSuccess = false;
int ledStatus = LOW;
bool retry=true;
uint32_t lastReconnectAttempt = 0;
float lat = 0, lng = 0;
unsigned long millis_time;
const int PH_SENSOR_PIN = 32;        // Update with the actual pin number
const int ORP_SENSOR_PIN = 33; // Update with the actual pin number
const int TDS_SENSOR_PIN = 34;         // Update with the actual pin number
const int TURBIDITY_SENSOR_PIN = 35;   // Update with the actual pin number
float ph,orp,tds,turbidity;

const float ORP_OFFSET = 0.7;    // To adjust based on ORP sensor's datasheet
const float ORP_SCALE_FACTOR = 200.0;  // To adjust based on your ORP sensor's datasheet

const float PH_OFFSET = 0.0;  
const float PH_SCALE = 14.0 / 3.3;  // Assuming a 0-3.3V output corresponds to a pH range of 0-14
float calculateAverageVoltage(int sensorPin) {
    float totalVoltage = 0.0;
    for (int i = 0; i < 100; i++) {
        float analogValue = analogRead(sensorPin);
        float voltage = (analogValue / 4095.0) * 3.3;
        totalVoltage += voltage;
    }
    return totalVoltage / 100.0;
}

float calculatePH() {
    float averageVoltage = calculateAverageVoltage(PH_SENSOR_PIN);
    float pH = averageVoltage * PH_SCALE + PH_OFFSET;
    return pH;
}

float calculateORP() {
    float averageVoltage = calculateAverageVoltage(ORP_SENSOR_PIN);
    float orp = averageVoltage * ORP_SCALE_FACTOR + ORP_OFFSET;
    return orp;
}

float calculateTDS() {
    float averageVoltage = calculateAverageVoltage(TDS_SENSOR_PIN);
    float tds = (133.42 * averageVoltage * averageVoltage * averageVoltage - 255.86 * averageVoltage * averageVoltage + 857.39 * averageVoltage) * 0.5;
    return tds;
}

float calculateTurbidity() {
    float averageVoltage = calculateAverageVoltage(TURBIDITY_SENSOR_PIN);
    float turbidity = averageVoltage; // Assuming 0-3.3V maps to 0-330 NTU
    return turbidity;
}



void collectData(){
  ph = calculatePH();
  orp = calculateORP();
  tds = calculateTDS();
  turbidity = calculateTurbidity();
  Serial.print("pH: ");
  Serial.println(ph);
  Serial.print("ORP: ");
  Serial.println(orp);
  Serial.print("TDS: ");
  Serial.println(tds);
  Serial.print("Turbidity: ");
  Serial.println(turbidity);
  
}

void disableGPS(void)
{
    // Set Modem GPS Power Control Pin to LOW ,turn off GPS power
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+CGPIO=0,48,1,0");
    if (modem.waitResponse(10000L) != 1) {
        DBG("Set GPS Power LOW Failed");
    }
    modem.disableGPS();
}
void enableGPS(void)
{
    // Set Modem GPS Power Control Pin to HIGH ,turn on GPS power
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+CGPIO=0,48,1,1");
    if (modem.waitResponse(10000L) != 1) {
        DBG("Set GPS Power HIGH Failed");
    }
    modem.enableGPS();
}

void sendDataRow() { 
  StaticJsonDocument<200> doc;
  doc["p"] = ph;//random(500, 900) / 100.0;  // pH-Wert
  doc["o"] = orp;//random(1000, 2000);        // ORP-Wert
  doc["td"] = tds;//random(100, 300) / 10.0;  // TDS-Wert
  doc["tu"] = turbidity;//random(10, 50);           // Trübungswert
  if (gpsSuccess) {
          doc["l"]["X"] = lat;
          doc["l"]["Y"] = lng;
      } else {
          doc["l"]["X"] = 0; // or error value
          doc["l"]["Y"] = 0; // or error value
      }

  char buffer[256];
  serializeJson(doc, buffer);
  mqtt.publish("waterquality/sensor", buffer);//todo change this
  Serial.println(buffer);
}

void mqttCallback(char *topic, byte *payload, unsigned int len)
{

    SerialMon.print("Message arrived [");
    SerialMon.print(topic);
    SerialMon.print("]: ");
    SerialMon.write(payload, len);
    SerialMon.println();
}

boolean mqttConnect()
{
    SerialMon.print("Connecting to ");
    SerialMon.print(broker);

    // Connect to MQTT Broker
    //boolean status = mqtt.connect("GsmClientTest");

    // Or, if you want to authenticate MQTT:
     boolean status = mqtt.connect("GsmClientName", mqttusername, mqttpassword);

    if (status == false) {
        SerialMon.println(" fail");
        return false;
    }
    SerialMon.println(" success");
    //mqtt.publish(topicInit, "GsmClientTest started");
    mqtt.subscribe(topic);
    return mqtt.connected();
}

bool getGpsCoordinates(float &lat, float &lng) {
  Serial.println("Waiting for the loacation to be locked");
  while(1){
    Serial.print(".");
    if (modem.getGPS(&lat, &lng)) {
      Serial.println("The location has been locked, the latitude and longitude are:");
      Serial.print("latitude:"); Serial.println(lat);
      Serial.print("longitude:"); Serial.println(lng);
      return true;
    }
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(2000);
  }
  return false;
  disableGPS();
}
void init_WIFI(){
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}
void init_LTE(){
  // Set LED OFF
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    // Starting the machine requires at least 1 second of low level, and with a level conversion, the levels are opposite
    delay(1000);
    digitalWrite(PWR_PIN, LOW);
    
    Serial.println("\nWait...");
    delay(1000);

    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
    
    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    Serial.println("Initializing modem...");
    if (!modem.restart()) {
        Serial.println("Failed to restart modem, attempting to continue without restarting");
    }



    String name = modem.getModemName();
    DBG("Modem Name:", name);

    String modemInfo = modem.getModemInfo();
    DBG("Modem Info:", modemInfo);

#if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if (GSM_PIN && modem.getSimStatus() != 3) {
        modem.simUnlock(GSM_PIN);
    }
#endif


#if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
    // The XBee must run the gprsConnect function BEFORE waiting for network!
    modem.gprsConnect(apn, gprsUser, gprsPass);
#endif

    SerialMon.print("Waiting for network...");
    if (!modem.waitForNetwork()) {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");

    if (modem.isNetworkConnected()) {
        SerialMon.println("Network connected");
    }

#if TINY_GSM_USE_GPRS
    // GPRS connection parameters are usually set after network registration
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");

    if (modem.isGprsConnected()) {
        SerialMon.println("GPRS connected");
    }
#endif
}
void setup()
{
    // Set console baud rate
    Serial.begin(115200);
    delay(10);
#if USE_WIFI
    init_WIFI();
#else
    init_LTE();
#endif
    //GPS SetUP Here
    delay(3000);
    Serial.println("initializing GPS ....");
    enableGPS();
    
    gpsSuccess = getGpsCoordinates(lat, lng);
    delay(2000);
    // MQTT Broker setup
    mqtt.setServer(broker, 1883);
    mqtt.setCallback(mqttCallback);
    delay(1600);
}

void loop()
{

#if !USE_WIFI
    // Make sure we're still registered on the network
    if (!modem.isNetworkConnected()) {
        SerialMon.println("Network disconnected");
        if (!modem.waitForNetwork(180000L, true)) {
            SerialMon.println(" fail");
            delay(10000);
            return;
        }
        if (modem.isNetworkConnected()) {
            SerialMon.println("Network re-connected");
        }

#if TINY_GSM_USE_GPRS
        // and make sure GPRS/EPS is still connected
        if (!modem.isGprsConnected()) {
            SerialMon.println("GPRS disconnected!");
            SerialMon.print(F("Connecting to "));
            SerialMon.print(apn);
            if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
                SerialMon.println(" fail");
                delay(10000);
                return;
            }
            if (modem.isGprsConnected()) {
                SerialMon.println("GPRS reconnected");
            }
        }
#endif
    }
#endif
    if (!mqtt.connected()) {
        SerialMon.println("=== MQTT NOT CONNECTED ===");
        // Reconnect every 10 seconds
        uint32_t t = millis();
        if (t - lastReconnectAttempt > 10000L) {
            lastReconnectAttempt = t;
            if (mqttConnect()) {
              SerialMon.println("=== MQTT CONNECTED ===");
                lastReconnectAttempt = 0;
                
            }
        }
        delay(100);
        return;
    }

    mqtt.loop();
    if(retry){ 
     delay(1000);
     collectData();
     sendDataRow();
     //retry=false; 
    }
    delay(6000);
}
