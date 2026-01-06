// ========== ARDUINO PRINCIPAL - LÓGICA DE LA CASA (MAESTRO I2C) ==========

#include <Wire.h>

class Componente {
protected:
  const char* Nombre;
  bool Estado;
  int Pin;

public:
  virtual void iniciar() = 0;
  virtual int leer() { return -1; }
  virtual void escribir(int v) {}
  virtual ~Componente() {}
};

class Led : public Componente {
private:
  bool esPWM; // NUEVO: indica si el LED soporta PWM
  
public:
  Led(const char* n, int p, bool pwm = true) {
    Nombre = n;
    Pin = p;
    Estado = false;
    esPWM = pwm;
  }

  void iniciar() override {
    pinMode(Pin, OUTPUT);
    digitalWrite(Pin, LOW);
  }

  void escribir(int pwm) override {
    if (esPWM) {
      // Para LEDs con PWM (LED_AUTO y LED_MANUAL)
      pwm = constrain(pwm, 0, 255);
      analogWrite(Pin, pwm);
      Estado = (pwm > 0);
    } else {
      // Para LEDs digitales de habitaciones (solo HIGH/LOW)
      if (pwm > 0) {
        digitalWrite(Pin, HIGH);
        Estado = true;
      } else {
        digitalWrite(Pin, LOW);
        Estado = false;
      }
    }
  }
  
  // NUEVO: método para saber si está encendido
  bool estaEncendido() {
    return Estado;
  }
};

class Boton : public Componente {
public:
  Boton(const char* n, int p) {
    Nombre = n;
    Pin = p;
    Estado = false;
  }

  void iniciar() override {
    pinMode(Pin, INPUT_PULLUP);
  }

  bool presionado() {
    return digitalRead(Pin) == LOW;
  }
};

class SensorLDR : public Componente {
public:
  SensorLDR(const char* n, int p) {
    Nombre = n;
    Pin = p;
  }

  void iniciar() override {}

  int leer() override {
    return analogRead(Pin); 
  }
};

const int PIN_LDR = A0;
const int PIN_LED_AUTO = 5;
const int PIN_LED_MANUAL = 3;
const int PIN_BOTON_MODO = 2;
const int PIN_BOTON_INTENSIDAD = 4;

// NUEVO: Pines de los LEDs de cada habitación (del 6 al 13)
const int PIN_JARDIN = 6;
const int PIN_SALA = 7;
const int PIN_COCINA = 8;
const int PIN_C3 = 9;
const int PIN_C2 = 10;
const int PIN_BANO = 11;
const int PIN_PATIO = 12;
const int PIN_C1 = 13;

// NUEVO: Dirección I2C del Arduino esclavo (pantalla)
const int DIRECCION_ESCLAVO = 8;

Led ledAuto("LED_AUTO", PIN_LED_AUTO);
Led ledManual("LED_MANUAL", PIN_LED_MANUAL);
SensorLDR ldr("LDR", PIN_LDR);
Boton botonModo("BOTON_MODO", PIN_BOTON_MODO);
Boton botonIntensidad("BOTON_INTENSIDAD", PIN_BOTON_INTENSIDAD);

// NUEVO: Pines para leer/controlar LEDs de habitaciones
const int pinesHabitaciones[8] = {PIN_JARDIN, PIN_SALA, PIN_COCINA, PIN_C3, PIN_C2, PIN_BANO, PIN_PATIO, PIN_C1};
bool modoBloqueo = false; // Cuando está true, forzamos apagado

bool modoAutomatico = true;
int nivelLuz = 0; 

const int nivelesPWM[3] = { 10, 30, 200 };
const int umbralLuz = 120;

bool lastBotonModo = false;
bool lastBotonIntensidad = false;

void leerCambioModo() {
  bool estadoActual = botonModo.presionado();

  if (estadoActual && !lastBotonModo) {
    modoAutomatico = !modoAutomatico;
  }

  lastBotonModo = estadoActual;
}

void leerCambioIntensidad() {
  bool estadoActual = botonIntensidad.presionado();

  if (estadoActual && !lastBotonIntensidad) {
    nivelLuz++;
    if (nivelLuz > 3) {
      nivelLuz = 0;
    }
  }

  lastBotonIntensidad = estadoActual;
}

int calcularIntensidad() {
  if (nivelLuz < 3) {
    return nivelesPWM[nivelLuz];
  } else {
    return (millis() / 200) % 2 ? 255 : 0;
  }
}

// NUEVO: Función para contar LEDs encendidos leyendo los pines
int contarLedsEncendidos() {
  if (modoBloqueo) {
    return 0; // Si están bloqueados, cuenta 0
  }
  
  int cuenta = 0;
  for (int i = 0; i < 8; i++) {
    if (digitalRead(pinesHabitaciones[i]) == HIGH) {
      cuenta++;
    }
  }
  return cuenta;
}

// NUEVO: Función para configurar pines según modo
void configurarPines(bool bloquear) {
  modoBloqueo = bloquear;
  
  if (bloquear) {
    // Modo OUTPUT para forzar LOW (apagar)
    for (int i = 0; i < 8; i++) {
      pinMode(pinesHabitaciones[i], OUTPUT);
      digitalWrite(pinesHabitaciones[i], LOW);
    }
  } else {
    // Modo INPUT para leer switches
    for (int i = 0; i < 8; i++) {
      pinMode(pinesHabitaciones[i], INPUT);
    }
  }
}

// NUEVO: Función para enviar datos al Arduino de la pantalla por I2C
void enviarDatosAPantalla() {
  Wire.beginTransmission(DIRECCION_ESCLAVO);
  
  Wire.write(modoAutomatico ? 1 : 0);  // Modo: 1=AUTO, 0=MANUAL
  Wire.write(nivelLuz);                 // Nivel de luz: 0-3
  Wire.write(contarLedsEncendidos()); 
  
  int valorLDR = ldr.leer();
  Wire.write(highByte(valorLDR));       
  Wire.write(lowByte(valorLDR));        
  
  Wire.endTransmission();
}

void setup() {
  Serial.begin(9600);
  Wire.begin(); 

  ledAuto.iniciar();
  ledManual.iniciar();
  botonModo.iniciar();
  botonIntensidad.iniciar();
  ldr.iniciar();
  
  configurarPines(false);
}

void loop() {
  leerCambioModo();
  leerCambioIntensidad();

  int intensidad = calcularIntensidad();

  if (modoAutomatico) {
    int luz = ldr.leer();
    bool oscuro = luz < umbralLuz;

    ledManual.escribir(0);
    ledAuto.escribir(oscuro ? intensidad : 0);

    bool debeBloquear = !oscuro || (nivelLuz == 3 && intensidad == 0);
    configurarPines(debeBloquear);
  }
  else {
    ledAuto.escribir(0);
    ledManual.escribir(intensidad);
    
  
    bool debeBloquear = (nivelLuz == 3 && intensidad == 0);
    configurarPines(debeBloquear);
  }

  static unsigned long ultimoEnvio = 0;
  if (millis() - ultimoEnvio > 200) {
    enviarDatosAPantalla();
    ultimoEnvio = millis();
  }

  delay(50);
}