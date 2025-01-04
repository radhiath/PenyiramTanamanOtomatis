#include <Wire.h>
#include "DHT11.h"
#include "LiquidCrystal_I2C.h"

#define SOIL_MOISTURE_PIN A3 
#define DHT_PIN A0   // dipake sbg pin digital biar konfigurasinya enak aja
#define PUMP_PIN 8   // Pin untuk pompa

#define LOWEST_VAL 270  // nilai sensor soil moisture di aer (~udh disesuaiin)
#define HIGHEST_VAL 575 // nilai sensor soil mositure di udara (~udh disesuaiin)

#define UPDATE_DELAY 5000 // cek dan update 5 detik sekali
#define WATERING_DURATION 5000 // lama nyiram 10 detik

float soilMoist = 0;                  // nilai dari soil moisture (set ke 0 biar ga diisi garbage val)
int temperature = 0, airHumidity = 0; // nilai dari DHT (set ke 0 biar ga diisi garbage val)

unsigned long lastChecking = 0; // waktu mulai cek
unsigned long wateringStartTime = 0;  // waktu mulai penyiraman
bool isWatering = false;

DHT11 DHT(DHT_PIN); // instantiate DHT
LiquidCrystal_I2C LCD(0x27, 20, 4); //instantiate LCD

void setup() {
    Serial.begin(9600);
    LCD.init();
    LCD.backlight();
    LCD.clear();

    LCD.setCursor(0, 0);
    LCD.print("Initializing...");
    delay(1000);
    LCD.clear();

    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW);
}

void loop() {
    unsigned long now = millis();
    if (now - lastChecking >= UPDATE_DELAY) {
        lastChecking = now;

        // baca kelembapan tanah
        getSoilMoist();

        // baca suhu dan kelembapan udara
        getTempHumidity();

        // cek perlu nyiram atau ngga
        checkEnvironment();

        // tampilin semuanya di LCD
        showInLCD();
    }

}

// fungsi buat gantiin fungsi map()
float mapFloat(float x, float fromLow, float fromHigh, float toLow, float toHigh) {
    return (x - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

// fungsi buat ngambil data kelembapan tanah
void getSoilMoist() {
    int rawSoilMoist = analogRead(SOIL_MOISTURE_PIN);
    soilMoist = mapFloat(
        constrain(rawSoilMoist, LOWEST_VAL, HIGHEST_VAL), 
        LOWEST_VAL, HIGHEST_VAL, 
        100.00, 0.00
    );
}

// fungsi buat ngambil data suhu dan kelembapa udara
void getTempHumidity() {
    static int lastTemperature = temperature;
    static int lastHumidity = airHumidity;
    int result = DHT.readTemperatureHumidity(temperature, airHumidity);

    // kl error pake nilai sebelumya (liat dokum)
    if (result != 0) {
        temperature = lastTemperature;
        airHumidity = lastHumidity;
    }
}

void checkEnvironment() {
    if (soilMoist < 50.0 || airHumidity < 50.0 || temperature > 35) {
        if (!isWatering) {
            wateringStartTime = millis();  // set waktu mulai
        }
        digitalWrite(PUMP_PIN, HIGH);
        isWatering = true;
    } else {
        digitalWrite(PUMP_PIN, LOW);
        isWatering = false;
    }

    // matiin pompa kalo udh lewat durasi
    if (isWatering && millis() - wateringStartTime >= WATERING_DURATION) {
        digitalWrite(PUMP_PIN, LOW);
        isWatering = false;
    }
}

// fungsi buat nampilin semua data ke LCD
void showInLCD() {
    // nampilin data di LCD
    LCD.setCursor(0, 0);
    LCD.print("Soil Moist: ");
    LCD.print(soilMoist);
    LCD.print("%  ");

    LCD.setCursor(0, 1);
    LCD.print("Air Humidity: ");
    LCD.print(airHumidity);
    LCD.print("%");

    LCD.setCursor(0, 2);
    LCD.print("Temperature: ");
    LCD.print(temperature);
    LCD.print((char)223);
    LCD.print("C");

    LCD.setCursor(0, 3);
    LCD.print(isWatering? "Status: Watering " : "Status: Idle      ");
}