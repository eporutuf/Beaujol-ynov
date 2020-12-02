// /* ========== Directive de préprocesseur =========== */
// #include <Arduino.h>
// #include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define LED_VERTE D5 // GPIO LED VERTE D5=14
#define LED_ROUGE D6 // GPIO LED ROUGE D6=

#define SEUIL_HAUT 24
#define SEUIL_BAS 20

#define HIBERNATION_TIME 2 //Hibernation time in seconds

OneWire oneWire(D4);                 // Initialise la communioation 1Wire pour le capteur de temperature DS18b200
DallasTemperature sensors(&oneWire); // Initialise le capteurs DS18b20
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Cf datasheets => 0x27 : Adresse fonctions des jumpers :A0,A1,A2 ; 16colonnesx2Lignes LCD

float getTemp()
{
  // Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  // Serial.print("DONE \n");
  float tempC = sensors.getTempCByIndex(0);
  if (tempC != DEVICE_DISCONNECTED_C)
  {
    Serial.print("Temperature is: ");
    Serial.println(tempC);
    return tempC;
  }
  else
  {
    // Serial.print("Error: Could not read temperature data \n");
    return -1;
  }
}

/*Une seule fois*/
void setup()
{
  /*Liaison Série*/
  Serial.begin(115200);
  /*Temperature init*/
  sensors.begin();
  /*LCD init*/
  lcd.init();
  lcd.clear();
  lcd.backlight();
  delay(100);
  lcd.setCursor(0,0);
  lcd.print("  Bienvenue");
  /* Digital IO */
  pinMode(LED_VERTE, OUTPUT);
  pinMode(LED_ROUGE, OUTPUT);
  digitalWrite(LED_VERTE, HIGH);
  digitalWrite(LED_ROUGE, HIGH);
}

/*Tâches répétitive*/
void loop()
{
  float temp = getTemp();
  if (temp <= SEUIL_BAS || temp >= SEUIL_HAUT)
  {
    digitalWrite(LED_ROUGE, LOW);
    digitalWrite(LED_VERTE, HIGH);
    lcd.setCursor(0,13);
    lcd.print("Nok");
  }
  else if (temp > SEUIL_BAS && temp < SEUIL_HAUT)
  {
    digitalWrite(LED_ROUGE, HIGH);
    digitalWrite(LED_VERTE, LOW);
    lcd.setCursor(0,14);
    lcd.print("Ok");
  }
  lcd.setCursor(0,1);
  lcd.print("Temperature: "); 
  lcd.setCursor(12,1);
  lcd.print(temp);
  delay(HIBERNATION_TIME);
  }