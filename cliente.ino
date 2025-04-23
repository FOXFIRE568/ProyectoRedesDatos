#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------------------
// Configuración de la pantalla OLED
// ---------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Espacio corregido
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------------------
// Configuración de pines para LoRa (TTGO LORA32)
// ---------------------------
#define SS_PIN    18
#define RST_PIN   14
#define DIO0_PIN  26

const long frequency = 915E6;

// ---------------------------
// Pines de sensores y actuadores
// ---------------------------
const int smokeSensorPin = 34;  // Sensor de humo
const int gasSensorPin = 35;    // Sensor de gas
const int ledPin = 15;          // LED de salida

String lastCommand = "OFF"; // Guarda el último estado del LED

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Inicialización de la pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error al iniciar la pantalla OLED");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Nodo Sensor Starting...");
  display.display();

  // Inicialización de LoRa
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
  if (!LoRa.begin(frequency)) {
    Serial.println("Fallo al iniciar LoRa!");
    display.println("LoRa fallo!");
    display.display();
    while (true);
  }
  Serial.println("LoRa iniciado correctamente");
  display.println("LoRa iniciado");
  display.display();
}

void loop() {
  // 1. LEER SENSORES Y ENVIAR
  int smokeValue = analogRead(smokeSensorPin);
  int gasValue = analogRead(gasSensorPin);
  String message = "H:" + String(smokeValue) + ",G:" + String(gasValue);

  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  Serial.print("Mensaje enviado: ");
  Serial.println(message);

  // 2. MOSTRAR EN OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Nodo Sensor");
  display.print("Humo: ");
  display.println(smokeValue);
  display.print("Gas: ");
  display.println(gasValue);
  display.print("LED: ");
  display.println(lastCommand);
  display.display();

  // 3. ESPERAR Y ESCUCHAR COMANDO LED
  Serial.println("Esperando comandos LED...");
  unsigned long start = millis();
  bool commandReceived = false;

  while (millis() - start < 3000) { // Esperar hasta 3 segundos por comando
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String incoming = "";
      while (LoRa.available()) {
        incoming += (char)LoRa.read();
      }

      Serial.print("Comando recibido: ");
      Serial.println(incoming);

      if (incoming.startsWith("LED:")) {
        String command = incoming.substring(4);
        command.trim();
        command.toLowerCase();

        if (command == "on") {
          digitalWrite(ledPin, HIGH);
          lastCommand = "ON";
          Serial.println("LED ENCENDIDO");
        } else if (command == "off") {
          digitalWrite(ledPin, LOW);
          lastCommand = "OFF";
          Serial.println("LED APAGADO");
        } else {
          Serial.println("Comando LED no reconocido");
        }

        commandReceived = true;
        break;
      }
    }
  }

  if (!commandReceived) {
    Serial.println("No se recibió comando LED");
  }

  delay(2000); // Tiempo entre ciclos
}