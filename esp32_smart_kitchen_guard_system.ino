#define BLYNK_TEMPLATE_ID "TMPL6BqnHqspj"
#define BLYNK_TEMPLATE_NAME "Smart Kitchen Guard System With ESP32"
#define BLYNK_AUTH_TOKEN "Ha1bMnJGSAQEoHWs0eO7pUezsnAGz8Jo"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Sensor & Buzzer
#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LDR_PIN 33
#define MQ2_SMOKE_PIN 35 //MQ-2 dekat kompor
#define MQ2_GAS_PIN 34 //MQ-2 dekat tabung gas
#define BUZZER_PIN 25

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Blynk
char auth[] = BLYNK_AUTH_TOKEN;

// WiFi credentials (add your SSID and password)
char ssid[] = "Wokwi-GUEST";
char pass[] = "";
BlynkTimer timer;

// Ambang Batas
float tempLimit = 60.0;      // DHT22
int lightLimit = 355;     // LDR
int gasLimit = 3497;        // MQ-2 dekat tabung gas
int smokeLimit = 3707;       // MQ-2 dekat kompor

String buzzerStatus = "Buzzer OFF";

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LDR_PIN, INPUT);
  pinMode(MQ2_GAS_PIN, INPUT);
  pinMode(MQ2_SMOKE_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting...");
  display.display();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(3000L, sendSensorData);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Connected!");
  display.display();
  
  delay(1000);
}

void loop() {
  Blynk.run();
  timer.run();

  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  int lightValue = analogRead(LDR_PIN);
  int gasValue = analogRead(MQ2_GAS_PIN);
  int smokeValue = analogRead(MQ2_SMOKE_PIN);

  Serial.print("Suhu: "); Serial.print(temp); Serial.print(" °C, ");
  Serial.print("Humidity: "); Serial.print(humid); Serial.print(" %, ");
  Serial.print("Light: "); Serial.print(lightValue); Serial.print(", ");
  Serial.print("Gas: "); Serial.print(gasValue); Serial.print(", ");
  Serial.print("Smoke: "); Serial.println(smokeValue);

  display.clearDisplay();
  display.setCursor(0, 0);
  
  // Set Kondisi
  if (gasValue > gasLimit){
    display.println("Gas Bocor!");
    digitalWrite(BUZZER_PIN, HIGH);
  }else if (smokeValue > smokeLimit){
    display.println("Cek Dapur! Ada Asap!");
    digitalWrite(BUZZER_PIN, HIGH);
  }else if (temp > tempLimit && lightValue > lightLimit){
    display.println("Peringatan Kebakaran!");
    digitalWrite(BUZZER_PIN, HIGH);
  }else{
    display.println("Dapur Aman");
    display.println("Suhu: " + String(temp, 1) + "C");
    display.println("Humid: " + String(humid, 0) + "%");
    digitalWrite(BUZZER_PIN, LOW);
  }

  display.display();
  delay(1000);
}

void sendSensorData(){
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  int lightValue = analogRead(LDR_PIN);
  int gasValue = analogRead(MQ2_GAS_PIN);
  int smokeValue = analogRead(MQ2_SMOKE_PIN);
  String status = "";

  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, humid);
  Blynk.virtualWrite(V2, lightValue);
  Blynk.virtualWrite(V3, gasValue);
  Blynk.virtualWrite(V4, smokeValue);

    // Set Kondisi
  if (gasValue > gasLimit){
  status = "⚠️ Gas Bocor!";
  Blynk.logEvent("kebocoran_gas", "⚠️ Gas Bocor!");
  }else if (smokeValue > smokeLimit) {
  status = "⚠️ Cek Dapur! Ada Asap!";
  Blynk.logEvent("asap", "⚠️ Cek Dapur! Ada Asap!");
  }else if (temp > tempLimit && lightValue > lightLimit) {
  status = "🔥 Peringatan Kebakaran!";
  Blynk.logEvent("kebakaran", "🔥 Peringatan Kebakaran!");
  }else {
  status = "✅ Keadaan Aman =)";
  }

  Blynk.virtualWrite(V5, status);

}
BLYNK_WRITE(V6) {
  int btn = param.asInt();
  digitalWrite(BUZZER_PIN, btn);
  buzzerStatus = btn ? "Buzzer ON" : "Buzzer OFF";
}

