#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

// OLED Display Config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi Credentials (Update these)
const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";

// ThingSpeak API Credentials (Update with your API Key & Channel ID)
const char* THINGSPEAK_API_KEY = "YOUR_API_KEY";
unsigned long THINGSPEAK_CHANNEL_ID = YOUR_CHANNEL_ID;  // Replace with your channel ID

WiFiClient client;

// Simulated Analog Inputs (Replace with real sensor pins)
#define ESTROGEN_SENSOR A0
#define PROGESTERONE_SENSOR A1
#define INSULIN_SENSOR A2
#define IGF_SENSOR A3

void setup() {
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Connect to WiFi
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nConnected to WiFi!");

    ThingSpeak.begin(client);

    // Initialize OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Biosensor: Starting...");
    display.display();
    delay(2000);
}

void loop() {
    // Simulated sensor readings (Replace analogRead with actual sensor readings)
    float estrogen = analogRead(ESTROGEN_SENSOR) * (200.0 / 1023.0);
    float progesterone = analogRead(PROGESTERONE_SENSOR) * (25.0 / 1023.0);
    float insulin = analogRead(INSULIN_SENSOR) * (30.0 / 1023.0);
    float igf1 = analogRead(IGF_SENSOR) * (300.0 / 1023.0);

    Serial.print("Estrogen: "); Serial.print(estrogen); Serial.println(" pg/mL");
    Serial.print("Progesterone: "); Serial.print(progesterone); Serial.println(" ng/mL");
    Serial.print("Insulin: "); Serial.print(insulin); Serial.println(" µU/mL");
    Serial.print("IGF-1: "); Serial.print(igf1); Serial.println(" ng/mL");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Hormone Levels:");
    display.print("E: "); display.println(estrogen);
    display.print("P: "); display.println(progesterone);
    display.print("I: "); display.println(insulin);
    display.print("IGF: "); display.println(igf1);
    
    // Risk analysis
    if (estrogen > 100 && progesterone < 5) {
        display.println("⚠️ High Estrogen, Low P!");
        Serial.println("Warning: High Estrogen, Low Progesterone (Potential Risk)");
    }
    if (insulin > 20) {
        display.println("⚠️ High Insulin!");
        Serial.println("Warning: High Insulin Levels (Potential Risk)");
    }
    
    display.display();

    // Send data to ThingSpeak
    ThingSpeak.setField(1, estrogen);
    ThingSpeak.setField(2, progesterone);
    ThingSpeak.setField(3, insulin);
    ThingSpeak.setField(4, igf1);

    int response = ThingSpeak.writeFields(THINGSPEAK_CHANNEL_ID, THINGSPEAK_API_KEY);
    if (response == 200) {
        Serial.println("Data sent to ThingSpeak successfully!");
    } else {
        Serial.print("Error sending data to ThingSpeak. HTTP error code: ");
        Serial.println(response);
    }

    delay(15000);  // Send data every 15 seconds
}
