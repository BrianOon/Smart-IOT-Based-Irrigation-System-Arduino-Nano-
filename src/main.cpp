// Define module that is used.
#define TINY_GSM_MODEM_SIM900

// define Blynk template.
#define BLYNK_TEMPLATE_ID "TMPL68a2_SKi3"
#define BLYNK_TEMPLATE_NAME "Smart IOT Based Irrigation Arduino"
#define BLYNK_AUTH_TOKEN "xmOiSCfd2WRfOyK94jKNGJZ1D3YE14JC"

// Headers.
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGsmClient.h> // https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpClient/HttpClient.ino
#include <ArduinoJson.h>
#include <BlynkSimpleTinyGSM.h> // https://github.com/Tech-Trends-Shameer/Arduino-GSM-Projects/blob/main/Home-Automation-Using-Arduino-GSM-Blynk-IOT/home-automation-using-arduino-gms-blynk-iot.ino

///////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************************************************************************


Smart IOT-Based Irrigation System (Arduino Nano)
---------------------------------
A smart iot-based irrigation system that determines whether to water plants based on soil moisture, air humidity,
surrounding temperature, and precipitation probability. The data is sent to Blynk for users to view. 
This version is made for the Arduino Nano ATmega328 microcontroller with the SIM900 GSSM Module.

Author: BrianOon (Bren)
Latest Revision: 15/3/2026

To-do List:

- Test location system, Pirate Weather API and weather check
- Implement sensor readings
- Connect GSM to Blynk app

For any inquiries, contact me at brianoon07+fypPTSS@gmail.com


***************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////////


// Declare pins.
const int
RX_pin = 2, // Pin that receives external data.
TX_pin = 3, // Pin that transmits data externally.
pump_relay_pin = 19; 

// Declare variables of parameters.
float
  soil_moist = 20,
  air_humid = 20,
  temperature = 20;
String
  prob1 = "none",
  prob2 = "none";

// Declare constants which is used for condition checks.
const float 
  max_moist = 60, 
  max_humid = 70, 
  min_temp = 4;

// Sets serial ports for communication with SIM900 module.
SoftwareSerial serialAT(RX_pin, TX_pin); 
TinyGsm modem(serialAT);
TinyGsmClient client(modem);

// Declare Blynk's built in timer.
BlynkTimer timer;

// Blynk cloud server.
const char*
  host = "blynk.cloud";

// Declare constants relating to the API host.
const String
  api_key = "QvSwH3jHrwxiY2tnvDaTGrOxvbeLQIqb",
  host_site = "api.pirateweather.net";
const int 
  port = 80; // Default network port used for HTTP.

// Boolean that determines whether weather is good.
bool
  good_weather = false;

// Declare longitude and latitude.
  // The default location is Tuanku Syed Sirajuddin Polytechnic.
String
  latitude = "6.450",
  longitude = "100.344";

// GPRS credentials. GPRS is a mobile data standard that is part of the 2G cellular network.
const char
  apn[] = "YourAPN", 
  gprsUser[] = "",
  gprsPass[] = "";

// Values for timer for environment check.
const unsigned int
    weather_interval = 10000, // Checks every 10s. Need to switch to 300000s (5 mins).
    max_checks = 12;
int
    num_rain_checks = 0; // Failsafe value for false rain predictions.
unsigned long
  last_weather_check = 0;

// Function declaration.
bool 
  env_condition_check(), 
  soil_moist_check(),
  air_humid_check(),
  temperature_check(),
  weather_check();
void 
  set_location(),
  blynk_func();


///////////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {

  randomSeed(millis()); // Generate random seed for random temporary values.

  Serial.begin(9600);
  serialAT.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, modem, apn, gprsUser, gprsPass);

  // Set pin mode.
  pinMode(pump_relay_pin, OUTPUT);

  // GPRS setup.
  Serial.println("Initialising modem.");
  modem.restart(); 
  Serial.print("Waiting for network...");
  modem.waitForNetwork();
  Serial.println("Connecting to GPRS...");
  modem.gprsConnect(apn, gprsUser, gprsPass);

  timer.setInterval(1000L, blynk_func); 

}

void loop() {

  delay(1000); // 1s delay.

  // Run Blynk.
  Blynk.run();
  timer.run();

  // Checks if network is connected. Reconnects and restarts the loop if not.
  if (!modem.isNetworkConnected()) {
    Serial.println("Network lost. Reconnecting...");
    modem.waitForNetwork();
    return;
  }

  // Checks if GPRS is connected. Reconnects and restarts the loop if not.
  if (!modem.isGprsConnected()) {
    Serial.println("GPRS lost. Reconnecting...");
    modem.gprsConnect(apn, gprsUser, gprsPass);
    return;
  }

  // Checks if a weather check is needed.
    // Also checks for millis() overflow.
  if(millis() - last_weather_check >= weather_interval) { 

    set_location();
    good_weather = weather_check();
    last_weather_check = millis();
  
  } 

  // Random temporary values are used for now.
  soil_moist = random() % 101, 
  air_humid = random() % 101, 
  temperature = random() % 101;

  if (env_condition_check()) {

    Serial.println("Plant watered");
    Serial.println("_________________________________________");
    digitalWrite(pump_relay_pin, HIGH);
    delay(10000); // Waters for 10s
    digitalWrite(pump_relay_pin, LOW);

  }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////


// Function Definition.
  // Function that checks every environment condition.
bool env_condition_check() {

  if (soil_moist_check() && air_humid_check() && temperature_check()) {
    if (good_weather) {

      return true;

    }
    else {

      Serial.println("Rain predicted.");
      Serial.println("_________________________________________");
      return false;

    }
  }
  else {
    return false;
  }

}

  // Function that checks soil moisture.
bool soil_moist_check() {
  
  if (soil_moist <= max_moist) {
    Serial.println("Soil moisture passed the check");
    return true;
  }
  else {
    Serial.println("Soil moisture failed the check");
    Serial.println("_________________________________________");
    return false;
  }

}

  // Function that checks air humidity.
bool air_humid_check() {

  if (air_humid <= max_humid) {
    Serial.println("Air humidity passed the check");
    return true;
  }
  else {
    Serial.println("Air humidity failed the check");
    Serial.println("_________________________________________");
    return false;
  }

}

  // Function that checks surrounding temperature.
bool temperature_check() {

  if (temperature >= min_temp) {
    Serial.println("Temperature passed the check");
    return true;
  }
  else {
    Serial.println("Temperature failed the check");
    Serial.println("_________________________________________");
    return false;
  }

}

  // Function updates the location through Blynk
BLYNK_WRITE(V5) {
    latitude = param.asStr();
    Serial.print("Latitude changed to ");
    Serial.println(latitude);
}
BLYNK_WRITE(V6) {
    longitude = param.asStr();
    Serial.print("Longitude changed to ");
    Serial.println(longitude);
}

/* Unused.
  // Function updates the location of the device in longitude and latitude based on the nearby cell providers.
    // Unconfirmed if this funtion works.
void set_location() {


      Note:
      serialAT.println() in this case outputs a message to the GSM module, instead of the computer like Serial.println().
      Thus, this does not print a message on the computer.
      The message is an attention (AT) command, which is used for communication equipment such as GSM modules.
      the command AT+CIPGSMLOC gets location in the form of string "<locationcode>,<longitude>,<latitude>,<date>,<time>"


  serialAT.println("AT+CIPGSMLOC=1,1");
  delay(2000);

    // Checks if there is data in serialAT. Updates location if so.
  if(serialAT.available()) {

    // Reads the string received from GSM after the AT command.
    String response = serialAT.readString();

    // Gets the index of the first three commas.
    int 
      firstcomma = response.indexOf(","),
      secondcomma = response.indexOf(",", firstcomma + 1),
      thirdcomma = response.indexOf(",", secondcomma + 1);

    // Splits out longitude and latitude using index of commas.
      // Longitude is between first and second comma, latitude between second and third.
    longitude = response.substring(firstcomma + 1, secondcomma);
    latitude = response.substring(secondcomma + 1, thirdcomma);

  }
  else {
    Serial.println("Error in set_location() function.");
  }
}

*/

  // Function that checks local weather forcasts.
bool weather_check() {

    // Pirate Api has a free 10000 monthly request limit,
    // thus a request is made every 5 minutes, with around 8928 requests made in a 31 day month.

    // Call API through GSM module.
      // Unconfirmed if this part works.
  String 
    url = "/forecast/" + api_key + "/" + latitude + "," + longitude + "?exclude=currently,minutely,daily,alerts,flags&units=si",
    httpResponse;
  
  Serial.println("Connecting to weather API...");

  if (!client.connect(host_site.c_str(), port)) {
    Serial.println("Connection failed");
    return false;
  }

  Serial.println("Sending HTTP request");
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host_site + "\r\n" + "Connection: close\r\n\r\n");

  delay(1000);

  while(client.available()){
    httpResponse += client.readString();
    int bodyStart = httpResponse.indexOf("\r\n\r\n");
    if (bodyStart != -1) {
      httpResponse = httpResponse.substring(bodyStart + 4);
    }
  }

  if (httpResponse.length() == 0) {
      Serial.println("No data received from API");
      return false;
  }
  client.stop();

    // Parse JSON received from API.
  JsonDocument filter;
  filter["hourly"]["data"][0]["icon"] = true;
  filter["hourly"]["data"][1]["icon"] = true;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, httpResponse, DeserializationOption::Filter(filter));

  if (!error) {
    
    prob1 = doc["hourly"]["data"][0]["icon"] | "none", 
    prob2 = doc["hourly"]["data"][1]["icon"] | "none";

    Serial.print("Weather Forecast (Hour 1): "); 
    Serial.println(prob1);
    Serial.print("Weather Forecast (Hour 2): "); 
    Serial.println(prob2);

    if ((prob1 == "rain" || prob2 == "rain") && num_rain_checks <= max_checks) {
      Serial.println("Rain likely in this hour or next hour, irrigation cancelled.");
      Serial.println("_________________________________________");
      num_rain_checks += 1;
      return false;
    }

    Serial.println("Rain unlikely in this hour or next hour, irrigation continues.");
    Serial.println("_________________________________________");
    num_rain_checks = 0;
    return true;
  }

  Serial.println("Error in weather check function.");
  Serial.println("_________________________________________");
  num_rain_checks = 0;
  return true;
  
}

// Function to send values to Blynk 
void blynk_func() {
  Blynk.virtualWrite(V0, soil_moist);
  Blynk.virtualWrite(V1, air_humid);
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, prob1);
  Blynk.virtualWrite(V4, prob2);
}