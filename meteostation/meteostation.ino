#include <Wire.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ThingerWifi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

// Pins
#define MUX_A 3
#define MUX_B 4

// Configuration
#define THINGER_USER "username"
#define THINGER_DEVID "deviceId"
#define THINGER_PASS "deviceCredential"
#define WIFI_SSID "wifi_ssid"
#define WIFI_PASS "wifi_pass"

// BMP180 object
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

// Thinger instance
ThingerWifi meteo(THINGER_USER, THINGER_DEVID, THINGER_PASS);

// BMP180 sensor values
float bmp_pres, bmp_temp, bmp_alti;

// MQ sensor values
float mq_co2, mq_co, mq_ch4, mq_no2;

void setup() 
{
  Serial.begin(9600);
  Serial.println("MeteoStation v1.0.0"); Serial.println("");

  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);

  Serial.println("Estableciendo conexión wifi...");
  meteo.add_wifi(WIFI_SSID, WIFI_PASS);
  
  // Initialize the sensor
  if(!bmp.begin())
  {
    Serial.print("ERROR: BMP180 no detectado.");
  } else {
    Serial.print("BMP180 iniciado correctamente.");
  }

  Serial.println("Configurando parámetros para thinger.io...");
  meteo["bmp180"] >> [](pson& out){
      out["pressure"] = bmp_pres;
      out["temperature"] = bmp_temp;
      out["altitude"] = bmp_alti;
  };
  
  meteo["gas"] >> [](pson& out){
      out["co2"] = mq_co2;
      out["co"] = mq_co;
      out["ch4"] = mq_ch4;
      out["no2"] = mq_no2;
  };

  Serial.println("Inicio completado.");
}

void loop() 
{
  readBmp();
  readMq7();
  readMq135();

  meteo.handle();
}

void setChannel(byte c)
{
  digitalWrite(MUX_A, bitRead(c, 0));
  digitalWrite(MUX_B, bitRead(c, 1));
}

void readMq7()
{
  setChannel(0);
  int val = analogRead(A0);
}

void readMq135()
{
  setChannel(1);
  int val = analogRead(A0);
}

void readBmp()
{
  sensors_event_t event;
  bmp.getEvent(&event);

  if (event.pressure)
  {
    /* Display atmospheric pressure in hPa */
    Serial.print("Presión:    ");
    Serial.print(event.pressure);
    Serial.println(" hPa");
    bmp_pres = event.pressure;

    /* Display temperature in Celsius */
    bmp.getTemperature(&bmp_temp);
    Serial.print("Temperatura: ");
    Serial.print(bmp_temp);
    Serial.println(" C");

    /* Then convert the atmospheric pressure, and SLP to altitude         */
    /* Update this next line with the current SLP for better results      */
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    bmp_alti = bmp.pressureToAltitude(seaLevelPressure, event.pressure);
    Serial.print("Altitud:    "); 
    Serial.print(bmp_alti); 
    Serial.println(" m");
    Serial.println("");
  }
  else
  {
    Serial.println("Error de lectura del BMP180");
  }
}
