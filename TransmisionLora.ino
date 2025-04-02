#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================== Configuración de LoRa ====================
#define LORA_SS    18   // Chip Select
#define LORA_RST   14   // Reset
#define LORA_DIO0  26   // DIO0 (Interrupción)

// ==================== Configuración del Display OLED ====================
#define SCREEN_WIDTH 128 // Ancho del display en píxeles
#define SCREEN_HEIGHT 64 // Alto del display en píxeles
#define OLED_RESET    -1 // Pin de reset (algunos módulos lo fijan internamente)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // Inicializa el monitor serial
  Serial.begin(115200);
  while (!Serial);

  // Inicializa el display OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Dirección I2C por defecto 0x3C
    Serial.println("Error al iniciar el display SSD1306");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Iniciando display...");
  display.display();

  // Configura los pines para el módulo LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  // Muestra mensaje en el display
  display.println("Iniciando LoRa...");
  display.display();

  // Inicializa LoRa en 915 MHz
  if (!LoRa.begin(915E6)) {
    Serial.println("Error al iniciar LoRa");
    display.println("Error al iniciar LoRa");
    display.display();
    while (1);
  }
  
  Serial.println("LoRa iniciado correctamente");
  display.println("LoRa iniciado correctamente");
  display.display();
  delay(2000); // Pausa para leer el mensaje
  display.clearDisplay();
}

void loop() {
  // Genera el mensaje a enviar
  unsigned long tiempoActual = millis();
  String mensaje = "Hola mundo " + String(tiempoActual);

  // Muestra información en el monitor serial y en el display
  Serial.print("Enviando paquete: ");
  Serial.println(mensaje);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Enviando paquete:");
  display.println(mensaje);
  display.display();

  // Envía el paquete LoRa
  LoRa.beginPacket();
  LoRa.print(mensaje);
  LoRa.endPacket();

  delay(1000); // Envía cada 1 segundo
}