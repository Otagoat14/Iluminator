class Componente {
protected:
  const char* Nombre;
  bool Estado;
  int Pin;

public:
  virtual void iniciar() = 0;
  virtual int leer() { return -1; }
  virtual void escribir(int v) { }
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

  void escribir(int v) override {
    Estado = (v != 0);
    digitalWrite(Pin, Estado ? HIGH : LOW);
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

  int leer() override {
    Estado = (digitalRead(Pin) == LOW);
    return Estado ? 1 : 0;
  }
};


class SensorLDR : public Componente {
public:
  SensorLDR(const char* n, int p) {
    Nombre = n;
    Pin = p;
    Estado = true;
  }

  void iniciar() override { }

  int leer() override {
    return analogRead(Pin);
  }
};


const int PIN_LDR = A0;
const int PIN_LED_GLOBAL = 7;   
const int PIN_BOTON_AUTO = 2;

Led luzGlobal("Global", PIN_LED_GLOBAL);
SensorLDR ldr("LDR", PIN_LDR);
Boton botonAuto("Auto", PIN_BOTON_AUTO);

bool modoAutomatico = true;
const int umbralLuz = 500;

bool botonPrev = false;

void setup() {
  luzGlobal.iniciar();
  ldr.iniciar();
  botonAuto.iniciar();
}

void loop() {
  bool autoAhora = (botonAuto.leer() == 1);

  if (autoAhora && !botonPrev) {
    modoAutomatico = !modoAutomatico;
  }
  botonPrev = autoAhora;

  if (modoAutomatico) {
    int luz = ldr.leer();
    bool estaOscuro = (luz < umbralLuz);
    luzGlobal.escribir(estaOscuro ? 1 : 0);
  } else {
    luzGlobal.escribir(0);
  }

  delay(20);
}
