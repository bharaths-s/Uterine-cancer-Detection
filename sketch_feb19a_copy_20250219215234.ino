#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Simulated Analog Inputs (Replace with real sensor pins)
#define ESTROGEN_SENSOR A0
#define PROGESTERONE_SENSOR A1
#define INSULIN_SENSOR A2
#define IGF_SENSOR A3

void setup() {
    Serial.begin(115200);
    
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
    // Simulated sensor readings (use analogRead for real sensors)
    float estrogen = analogRead(ESTROGEN_SENSOR) * (200.0 / 1023.0);  // Scale to 0-200 pg/mL
    float progesterone = analogRead(PROGESTERONE_SENSOR) * (25.0 / 1023.0);  // Scale to 0-25 ng/mL
    float insulin = analogRead(INSULIN_SENSOR) * (30.0 / 1023.0);  // Scale to 0-30 µU/mL
    float igf1 = analogRead(IGF_SENSOR) * (300.0 / 1023.0);  // Scale to 0-300 ng/mL

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
    delay(5000);
}
