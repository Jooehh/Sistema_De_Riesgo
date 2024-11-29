#include <Wire.h>
  #include <LiquidCrystal_PCF8574.h>

  // Define pines
  #define SOIL_SENSOR_PIN A0  // Pin analógico al que está conectado el sensor YL-69
  #define RELAY_PIN 7         // Pin digital al que está conectado el relé
  #define LCD_SDA_PIN A4      // Pin SDA del LCD
  #define LCD_SCL_PIN A5      // Pin SCL del LCD

  // Configura el tipo de re
  #define ACTIVE_HIGH true  // Cambia a false si tu relé es activo en bajo

  // Umbrales de humedad
  const int DRY_THRESHOLD = 600;  // Suelo seco
  const int WET_THRESHOLD = 400;  // Suelo húmedo

  // Inicializa la pantalla LCD (dirección I2C por defecto 0x27)
  LiquidCrystal_PCF8574 lcd(0x27);

  // Variables para estabilizar el estado de la bomba
  bool pumpState = false;
  unsigned long lastPumpChange = 0;
  const unsigned long pumpDelay = 5000; // 5 segundos de retardo entre cambios

  void setup() {
    // Configuración de los pines
    pinMode(RELAY_PIN, OUTPUT);
    setRelayState(false);  // Apaga la bomba por defecto

    // Inicializa la comunicación serie
    Serial.begin(9600);

    // Inicializa la pantalla LCD
    lcd.begin(16, 2);
    lcd.setBacklight(255); // Enciende la retroiluminación
    lcd.print("Riego Autom.");
    
    delay(2000); // Espera 2 segundos para mostrar el mensaje
    lcd.clear();
  }

  void loop() {
    // Lee y filtra el valor del sensor de humedad
    int soilMoistureValue = getAverageMoisture();

    // Mostrar el valor en el monitor serial y LCD
    Serial.println(soilMoistureValue);
    lcd.setCursor(0, 0);
    lcd.print("Humedad: ");
    lcd.print(soilMoistureValue);
    lcd.print("    "); // Espacios para borrar valores anteriores

    // Control de la bomba con retardo de seguridad
    unsigned long currentTime = millis();
    if (soilMoistureValue > DRY_THRESHOLD && !pumpState && currentTime - lastPumpChange > pumpDelay) {
      // Suelo seco: Enciende la bomba
      setRelayState(true);
      pumpState = true;
      lastPumpChange = currentTime;
      lcd.setCursor(0, 1);
      lcd.print("Bomba: ON ");
    } else if (soilMoistureValue < WET_THRESHOLD && pumpState && currentTime - lastPumpChange > pumpDelay) {
      // Suelo húmedo: Apaga la bomba
      setRelayState(false);
      pumpState = false;
      lastPumpChange = currentTime;
      lcd.setCursor(0, 1);
      lcd.print("Bomba: OFF");
    }

    // Espera 1 segundo antes de la próxima lectura
    delay(1000);
  }

  // Función para configurar el estado del relé
  void setRelayState(bool state) {
    if (ACTIVE_HIGH) {
      digitalWrite(RELAY_PIN, state ? HIGH : LOW); // Activo en alto
    } else {
      digitalWrite(RELAY_PIN, state ? LOW : HIGH); // Activo en bajo
    }
  }

  // Función para promediar varias lecturas del sensor
  int getAverageMoisture() {
    int total = 0;
    for (int i = 0; i < 10; i++) {
      total += analogRead(SOIL_SENSOR_PIN);
      delay(10); // Pequeño retardo para lecturas estables
    }
    return total / 10;
  }