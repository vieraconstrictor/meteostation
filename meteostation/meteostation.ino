#define _DEBUG_
#define _DISABLE_TLS_

#include <Wire.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ThingerWifi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

// MQ Sensor constants
const int MQ_PIN = A0;
const int RL_VALUE = 5;      // RL resistance in kiloohms
const int R0 = 10;          // R0 resistance in kiloohms
const int READ_SAMPLE_INTERVAL = 100;    // Time between samples
const int READ_SAMPLE_TIMES = 5;       // Amount of samples

// Configuration
#define THINGER_USER "user"
#define THINGER_DEVID "id"
#define THINGER_PASS "pass"
#define WIFI_SSID "ssid"
#define WIFI_PASS "ssid_pass"

struct GasProfile
{
  float P1[2];
  float P2[2];

  float scope;
  float coord;

  GasProfile(float X0, float Y0, float X1, float Y1)
  {
    P1[0] = log10(X0); P1[1] = log10(Y0);
    P2[0] = log10(X1); P2[1] = log10(Y1);

    scope = (P2[1] - P1[1]) / (P2[0] - P1[0]);
    coord = P1[1] - P1[0] * scope;
  }
};

// BMP180 object
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

// Thinger instance
ThingerWifi meteo(THINGER_USER, THINGER_DEVID, THINGER_PASS);

// BMP180 sensor values
float bmp_pres, bmp_temp, bmp_alti;

// MQ sensor values
float mq_co2, mq_co, mq_ch4, mq_no2;

// Gas profiles
GasProfile MQ_135_CO2(10, 2.36, 100, 1);
GasProfile MQ_135_CO(10, 2.85, 100, 1.6);

void setup() 
{
  Serial.begin(9600);
  Serial.println("MeteoStation v1.0.0"); Serial.println("");

  Serial.println("Estableciendo conexión wifi...");
  meteo.add_wifi(WIFI_SSID, WIFI_PASS);
  
  // Initialize the sensor
  if(!bmp.begin())
  {
    Serial.println("ERROR: BMP180 no detectado.");
  } else {
    Serial.println("BMP180 iniciado correctamente.");
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
  Serial.println("==========");
  Serial.print("Tiempo:      ");
  Serial.print(millis());
  Serial.println(" ms");
  
  readBMP();
  readGas();

  Serial.println("==========");
  meteo.handle();
  
  delay(1000);
}

// Obtener la resistencia promedio en N muestras
float readMQ(int mq_pin)
{
   float rs = 0;
   for (int i = 0;i<READ_SAMPLE_TIMES;i++) {
      rs += getMQResistance(analogRead(mq_pin));
      delay(READ_SAMPLE_INTERVAL);
   }
   return rs / READ_SAMPLE_TIMES;
}
 
// Obtener resistencia a partir de la lectura analogica
float getMQResistance(int raw_adc)
{
   return (((float)RL_VALUE / 1000.0*(1023 - raw_adc) / raw_adc));
}
 
// Obtener concentracion 10^(coord + scope * log (rs/r0)
float getConcentration(GasProfile profile, float rs_ro_ratio)
{
   return pow(10, profile.coord + profile.scope * log(rs_ro_ratio));
}

void readBMP()
{
  sensors_event_t event;
  bmp.getEvent(&event);

  if (event.pressure)
  {
    /* Display atmospheric pressure in hPa */
    Serial.print("Presión:     ");
    Serial.print(event.pressure);
    Serial.println(" hPa");
    bmp_pres = event.pressure;

    /* Display temperature in Celsius */
    bmp.getTemperature(&bmp_temp);
    Serial.print("Temperatura: ");
    Serial.print(bmp_temp);
    Serial.println(" C");

    /* Get altitude in meters */
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    bmp_alti = bmp.pressureToAltitude(seaLevelPressure, event.pressure);
    Serial.print("Altitud:     "); 
    Serial.print(bmp_alti); 
    Serial.println(" m");
  }
  else
  {
    Serial.println("Error de lectura del BMP180");
  }
}

void readGas()
{
  float rs_med;
  
  rs_med = readMQ(MQ_PIN);
  mq_co2 = getConcentration(MQ_135_CO2, rs_med/R0);
  Serial.print("CO2:         ");
  Serial.print(mq_co2);
  Serial.println(" ppm");

  rs_med = readMQ(MQ_PIN);
  mq_co = getConcentration(MQ_135_CO, rs_med/R0);
  Serial.print("CO:          ");
  Serial.print(mq_co);
  Serial.println(" ppm");
}
