#include <LiquidCrystal_I2C.h>


#define TdsSensorPin 34
#define ButtonPin 4
#define LedPin 2

#define VREF 5.0              // Voltaje de referencia (Volt) del ADC
#define SCOUNT  30            // Suma de puntos de muestra

int analogBuffer[SCOUNT];     // Almacena los valores analógicos en el arreglo, leídos desde el ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;       // Temperatura actual para la compensación
// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// Algoritmo de filtrado mediano
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (int i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  
  return bTemp;
}

void setup() {
  Serial.begin(115200);
  pinMode(TdsSensorPin, INPUT);
  pinMode(ButtonPin, INPUT_PULLUP);
  pinMode(LedPin, OUTPUT);
  lcd.init();
  lcd.backlight();
}

void loop() {
  // Verificar si el botón ha sido presionado
  int buttonState = digitalRead(ButtonPin);
  
  if (buttonState == LOW) {
    // El botón ha sido presionado, realizar la lectura del sensor
    static unsigned long analogSampleTimepoint = millis();
    
    if (millis() - analogSampleTimepoint > 40U) {
      analogSampleTimepoint = millis();
      analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
      analogBufferIndex++;
      
      if (analogBufferIndex == SCOUNT) {
        analogBufferIndex = 0;
      }
    }
    
    static unsigned long printTimepoint = millis();
    
    if (millis() - printTimepoint > 800U) {
      printTimepoint = millis();
      
      for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
        analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
        averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float) VREF / 1024.0;
        float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
        float compensationVoltage = averageVoltage / compensationCoefficient;
        tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;
       
        Serial.print("TDS Value: ");
        Serial.print(tdsValue, 0);
        Serial.println("ppm");
        lcd.setCursor(0, 0);
         
         lcd.print(tdsValue);
         
         
      }
    }
    
    // Encender el LED
    digitalWrite(LedPin, LOW);
   
  } else {
    // El botón no está presionado, apagar el LED
    digitalWrite(LedPin, HIGH);
    
     lcd.setCursor(0, 0);
      lcd.print("Presione el");
      lcd.setCursor(1, 1);
      lcd.print("boton");
      delay(1000);
      lcd.clear();
  }
}
