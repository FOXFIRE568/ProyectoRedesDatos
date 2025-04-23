#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuración de la pantalla OLED  
#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  
#define OLED_RESET    -1  
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  

// Pines para el módulo LoRa  
#define SS_PIN    5  
#define RST_PIN   14  
#define DIO0_PIN  2  

// Frecuencia de trabajo  
const long frequency = 915E6;  

// Pin del LED
const int ledPin = 13; // Pin del LED

void setup() {  
  Serial.begin(115200);  
  
  // Inicialización del LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // Asegura que el LED esté apagado al inicio

  // Inicialización de la pantalla  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){  
    Serial.println("Error al iniciar la pantalla OLED");  
    while (true);  
  }  
  display.clearDisplay();  
  display.setTextSize(1);  
  display.setTextColor(SSD1306_WHITE);  
  display.setCursor(0,0);  
  display.println("LoRa Receptor");  
  display.display();  
  
  // Configuración de pines y inicio de LoRa  
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);  
  if (!LoRa.begin(frequency)) {  
    Serial.println("Fallo al iniciar LoRa!");  
    display.println("LoRa fallo!");  
    display.display();  
    while (true);  
  }  
  display.println("LoRa OK!");  
  display.display();  
  delay(2000);  
}  

void loop() {  
  // Revisar si se ha recibido un paquete LoRa  
  int packetSize = LoRa.parsePacket();  
  if (packetSize) {  
    String mensajeRecibido = "";  
    while (LoRa.available()) {  
      mensajeRecibido += (char)LoRa.read();  
    }  

    Serial.print("Mensaje recibido: ");  
    Serial.println(mensajeRecibido);  

    // Mostrar mensaje recibido en la pantalla OLED
    display.clearDisplay();  
    display.setCursor(0,0);  
    display.println("LoRa Receptor");  
    display.println("Msg recibido:");  
    display.println(mensajeRecibido);  
    display.display();  

    // Controlar el LED según el mensaje recibido
    if (mensajeRecibido == "ENCENDER") {
      digitalWrite(ledPin, HIGH);  // Encender el LED
      Serial.println("LED ENCENDIDO");
    } 
    else if (mensajeRecibido == "APAGAR") {
      digitalWrite(ledPin, LOW);  // Apagar el LED
      Serial.println("LED APAGADO");
    }
  }
}