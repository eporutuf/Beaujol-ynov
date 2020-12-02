#include <Arduino.h>
/*LCD*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
/*/ DS18b20*/
#include <OneWire.h>
#include <DallasTemperature.h>
/*WIFI & MQTT*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define LED_VERTE D5 // GPIO LED VERTE D5=14
#define LED_ROUGE D6 // GPIO LED ROUGE D6=
#define SEUIL_HAUT 24
#define SEUIL_BAS 20

#define REFRESH_TIME 3000  //time beetween LCD refresh in mseconds
#define PUBLISH_TIME 15000 //time beetwenn each publish

/*Informations réseau */
const char *ssid = "Msayif";
const char *password = "123456789";
const char *mqtt_server = "51.158.79.224";
/*MQTT: TOPIC = nom/element*/
const char *topic_temperature = "michael/temp";
const char *topic_text = "michael/texte";
const char *topic_led_verte = "michael/ledVerte";
const char *topic_led_rouge= "michael/ledRouge";
/*MQTT*/
WiFiClient espClient;
PubSubClient client(espClient);
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

unsigned long lastMsgTemp = 0;
unsigned long lastLcdRefresh = 0;
String msgInBox = "";

/* sensor & LCD*/
OneWire oneWire(D4);                 // Initialise la communioation 1Wire pour le capteur de temperature DS18b200
DallasTemperature sensors(&oneWire); // Initialise le capteurs DS18b20
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Cf datasheets => 0x27 : Adresse fonctions des jumpers :A0,A1,A2 ; 16colonnesx2Lignes LCD

// Listes Pages
enum
{
    BIENVENUE,
    ACCEUIL,
    ENVOI
};

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

void setup_wifi()
{
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA); //MODE WIFI : Station, Acces-point...
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            client.subscribe(topic_text);
            client.subscribe(topic_led_verte);
            client.subscribe(topic_led_rouge);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(1000);
        }
    }
}

void publish()
{
    snprintf(msg, 50, "%.2f", getTemp());
    client.publish(topic_temperature, msg);
}

void displayPage(uint8_t viewPage)
{
    //lcd.clear();
    switch (viewPage)
    {
    case 0:
        lcd.setCursor(3, 0);
        lcd.print("Bienvenue");
        break;
    case 1:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(msgInBox);
        lcd.setCursor(7, 1);
        lcd.print(getTemp());
        lcd.setCursor(0, 1);
        lcd.print("Temp=  ");
        lcd.setCursor(12, 1);
        lcd.print("degC");
        break;
    case 2:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Envoi en-cours");
        break;
    }
}

void callback(char *inTopic, byte *payload, unsigned int length)
{
    String receivedPayload = "";
    for (unsigned int i = 0; i < length; i++)
    {
        receivedPayload += (char)(payload[i]);
    }
    receivedPayload.toCharArray(msg, receivedPayload.length() + 1);
    
    Serial.print("Topic ============= ");
    Serial.println(inTopic);
    Serial.print("Payload =========== ");
    Serial.println(receivedPayload);

    if (strcmp(inTopic, topic_text) == 0)
    {
        msgInBox = receivedPayload;
        displayPage(ACCEUIL);
    }
    else if (strcmp(inTopic, topic_led_verte) == 0)
        digitalWrite(LED_VERTE,!receivedPayload.toInt());
    else if (strcmp(inTopic, topic_led_rouge) == 0)
        digitalWrite(LED_ROUGE,!receivedPayload.toInt());
    else
        Serial.println("uknown Topic");
    
}

/*Une seule fois*/
void setup()
{
    pinMode(LED_ROUGE, OUTPUT);
    pinMode(LED_VERTE, OUTPUT);
    digitalWrite(LED_VERTE, HIGH);
    digitalWrite(LED_ROUGE, HIGH);
    lcd.init();
    lcd.backlight();
    displayPage(BIENVENUE);
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    lcd.setCursor(3, 1);
    lcd.print("Connected");
    delay(1000);
}

/*Tâches répétitive*/
void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    if ((millis() - lastMsgTemp) > PUBLISH_TIME)
    {
        //displayPage(ENVOI);
        publish();
        lastMsgTemp = millis();
    }
    else if (millis() - lastLcdRefresh > REFRESH_TIME)
    {
        displayPage(ACCEUIL);
        lastLcdRefresh = millis();
    }

    client.loop();
    delay(30);
}