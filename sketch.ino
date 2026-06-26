#include <WiFi.h>
#include <HTTPClient.h>

// --- WiFi (Wokwi simulation) ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- ThingSpeak ---
String apiKey = "94FRHFL9JJ14OFM2";
const char* server = "http://api.thingspeak.com/update";

// --- Pins ---
#define LDR_PIN 34
#define PIR_PIN 27
#define TRIG_PIN 5
#define ECHO_PIN 18
#define RELAY_PIN 26
#define BUZZER_PIN 19
#define LED_R 32
#define LED_G 33
#define LED_B 25

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connecte !");
}

long readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

void loop() {
  int ldrValue = analogRead(LDR_PIN);
  int motion = digitalRead(PIR_PIN);
  long distance = readDistance();

  bool nightTime = ldrValue < 1500;
  bool vehicleDetected = distance < 50;

  digitalWrite(RELAY_PIN, nightTime ? HIGH : LOW);

  if (vehicleDetected) {
    digitalWrite(LED_R, HIGH); digitalWrite(LED_G, LOW); digitalWrite(LED_B, LOW);
    tone(BUZZER_PIN, 1000, 200);
  } else if (motion) {
    digitalWrite(LED_R, LOW); digitalWrite(LED_G, LOW); digitalWrite(LED_B, HIGH);
  } else {
    digitalWrite(LED_R, LOW); digitalWrite(LED_G, HIGH); digitalWrite(LED_B, LOW);
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey +
                 "&field1=" + String(ldrValue) +
                 "&field2=" + String(distance) +
                 "&field3=" + String(motion) +
                 "&field4=" + String(nightTime ? 1 : 0);
    http.begin(url);
    int httpCode = http.GET();
    Serial.println("ThingSpeak code: " + String(httpCode));
    http.end();
  }

  delay(16000);
}
