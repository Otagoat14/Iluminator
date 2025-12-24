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
public:
  Led(const char* n, int p) {
    Nombre = n;
    Pin = p;
    Estado = false;
  }

  void iniciar() override {
    pinMode(Pin, OUTPUT);
    digitalWrite(Pin, LOW);
  }

  void escribir(int pwm) override {
    pwm = constrain(pwm, 0, 255);
    analogWrite(Pin, pwm);
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
const int PIN_LED_AUTO = 6;
const int PIN_LED_MANUAL = 3;
const int PIN_BOTON_MODO = 2;
const int PIN_BOTON_INTENSIDAD = 8;

Led ledAuto("LED_AUTO", PIN_LED_AUTO);
Led ledManual("LED_MANUAL", PIN_LED_MANUAL);
SensorLDR ldr("LDR", PIN_LDR);
Boton botonModo("BOTON_MODO", PIN_BOTON_MODO);
Boton botonIntensidad("BOTON_INTENSIDAD", PIN_BOTON_INTENSIDAD);

bool modoAutomatico = true;
int nivelLuz = 0; 

const int nivelesPWM[4] = { 50, 120, 200, 255 };
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
    if (nivelLuz > 3) nivelLuz = 0;
  }

  lastBotonIntensidad = estadoActual;
}

void setup() {
  Serial.begin(9600);

  ledAuto.iniciar();
  ledManual.iniciar();
  botonModo.iniciar();
  botonIntensidad.iniciar();
  ldr.iniciar();
}

void loop() {

  leerCambioModo();
  leerCambioIntensidad();

  int intensidad = nivelesPWM[nivelLuz];

  if (modoAutomatico) {
    int luz = ldr.leer();
    bool oscuro = luz < umbralLuz;

    ledManual.escribir(0);
    ledAuto.escribir(oscuro ? intensidad : 0);

    Serial.print("ANIVEL DE LUZ : ");
    Serial.println(luz);
  }
  else {
    ledAuto.escribir(0);
    ledManual.escribir(intensidad);

    Serial.println("MANUAL");
  }

  delay(50);
}
