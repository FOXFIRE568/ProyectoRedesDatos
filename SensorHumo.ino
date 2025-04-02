// Definir los pines analógicos para los sensores de gas y humo
const int smokeSensorPin = 34;  // Pin analógico para el sensor de humo (GPIO34)
const int gasSensorPin = 35;    // Pin analógico para el sensor de gas (GPIO35)

// Variables para almacenar los valores de los sensores
int smokeLevel = 0;
int gasLevel = 0;

void setup() {
  // Inicializamos la comunicación serial (solo para monitoreo, puedes eliminarla si no la necesitas)
  Serial.begin(115200);
  delay(1000);

  // Configuramos los pines de los sensores como entradas analógicas
  pinMode(smokeSensorPin, INPUT);
  pinMode(gasSensorPin, INPUT);
}

void loop() {
  // Leemos los valores de los sensores
  smokeLevel = analogRead(smokeSensorPin);  // Lectura del sensor de humo
  gasLevel = analogRead(gasSensorPin);      // Lectura del sensor de gas

  // Imprimimos los valores en el monitor serial (solo si lo necesitas)
  Serial.print("Nivel de humo: ");
  Serial.print(smokeLevel);  // Imprimir valor del sensor de humo
  Serial.print(", Nivel de gas: ");
  Serial.println(gasLevel);  // Imprimir valor del sensor de gas

  // Esperamos un segundo antes de realizar otra lectura
  delay(1000);
}