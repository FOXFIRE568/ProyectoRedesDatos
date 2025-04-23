#include <WiFi.h>
#include <PubSubClient.h>
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
#define OLED_RESET   -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------------------
// Configuración de pines para LoRa (TTGO LORA32)
// ---------------------------
#define SS_PIN    18
#define RST_PIN   14
#define DIO0_PIN  26

const long frequency = 915E6;

// ---------------------------
// Configuración WiFi y MQTT
// ---------------------------
const char* ssid = "SALON_A";
const char* password = "unives12345";
const char* mqttServer = "3.148.250.52";
const int mqttPort = 1883;
const char* mqttUser = "admin";
const char* mqttPassword = "UNIVES123";

WiFiClient espClient;
PubSubClient client(espClient);

// ---------------------------
// Callback para mensajes MQTT
// ---------------------------
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Mensaje MQTT recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  if (String(topic) == "led") {
    message.trim();
    message.toLowerCase();
    if (message == "on" || message == "off") {
      String comandoLoRa = "LED:" + message;
      Serial.print("Reenviando por LoRa: ");
      Serial.println(comandoLoRa);
      LoRa.beginPacket();
      LoRa.print(comandoLoRa);
      LoRa.endPacket();
    } else {
      Serial.println("⚠ Comando LED MQTT no válido (usa 'on' o 'off')");
    }
  }
}

// ---------------------------
// Función para reconectar al broker MQTT
// ---------------------------
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando a MQTT...");
    if (client.connect("GatewayClient", mqttUser, mqttPassword)) {
      Serial.println("Conectado a MQTT");
      client.subscribe("led");
    } else {
      Serial.print("Falló la conexión, rc=");
      Serial.print(client.state());
      Serial.println(" - Reintentando en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Inicialización OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error al iniciar la pantalla OLED");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Gateway Starting...");
  display.display();

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("WiFi conectado");

  // Configuración MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);

  // Configuración LoRa
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
  // Asegurar conexión MQTT
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Comprobar entrada manual desde monitor serial
  if (Serial.available()) {
    String comandoManual = Serial.readStringUntil('\n');
    comandoManual.trim();
    comandoManual.toLowerCase();
    if (comandoManual == "on" || comandoManual == "off") {
      String comandoLoRaManual = "LED:" + comandoManual;
      Serial.print("Enviando manualmente por LoRa: ");
      Serial.println(comandoLoRaManual);
      LoRa.beginPacket();
      LoRa.print(comandoLoRaManual);
      LoRa.endPacket();
    } else {
      Serial.println("⚠ Comando manual inválido (usa 'on' o 'off')");
    }
  }

  // Procesar datos recibidos por LoRa
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    Serial.print("Paquete LoRa recibido (sensor): ");
    Serial.println(incoming);

    String smokeValue = "";
    String gasValue = "";

    // Extraer humo
    if (incoming.indexOf("H:") != -1) {
      int start = incoming.indexOf("H:") + 2;
      int end = incoming.indexOf(",", start);
      if (end == -1) end = incoming.length();
      smokeValue = incoming.substring(start, end);
      smokeValue.trim();
      if (smokeValue.length() > 0) {
        Serial.print("Valor de Humo recibido: ");
        Serial.println(smokeValue);
        client.publish("humo", smokeValue.c_str());
      }
    }

    // Extraer gas
    if (incoming.indexOf("G:") != -1) {
      int start = incoming.indexOf("G:") + 2;
      int end = incoming.indexOf(",", start);
      if (end == -1) end = incoming.length();
      gasValue = incoming.substring(start, end);
      gasValue.trim();
      if (gasValue.length() > 0) {
        Serial.print("Valor de Gas recibido: ");
        Serial.println(gasValue);
        client.publish("gas", gasValue.c_str());
      }
    }

    // Mostrar estado en OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Gateway MQTT & LoRa");
    display.print("WiFi: ");
    display.println(WiFi.isConnected() ? "OK" : "Falló");
    display.print("MQTT: ");
    display.println(client.connected() ? "OK" : "Falló");
    display.print("Humo: ");
    display.println(smokeValue);
    display.print("Gas: ");
    display.println(gasValue);
    display.display();
  }

  delay(100);
}