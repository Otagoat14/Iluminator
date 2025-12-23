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
const int PIN_MODO_AUTOMATICO = 6;   //Aqui cambiar por los pines analogicos
const int PIN_BOTON = 2;  //Lo mismo en este
const int PIN_MODO_MANUAL = 3; // Y en este
const int PIN_BOTON_MODOS = 8;

Led luzAuto("LUZ_MODO_AUTO", PIN_MODO_AUTOMATICO);
SensorLDR ldr("LDR", PIN_LDR);
Boton botonAuto("CAMBIAR_MODO_AUTO", PIN_BOTON);
Boton botonModos("CAMBIAR_MODOS_PERSONALIZADOS", PIN_BOTON_MODOS);
Led luzManual("LUZ_MODO_MANUAL", PIN_MODO_MANUAL);

bool modoAutomatico = true;
bool modoManual = false ;
bool modoFiesta = false ;
bool modoRelajacion = false ;
bool modoNoche = false ;
bool modoLectura = false ;

const int umbralLuz = 120;

bool botonPrev = false;



void setup() {
  luzAuto.iniciar();
  luzManual.iniciar();
  ldr.iniciar();
  botonAuto.iniciar();
}



void loop() {
  bool autoAhora = (botonAuto.leer() == 1);
  bool modosAhora = (botonModos.leer() == 1) ;

  if (autoAhora && !botonPrev) {
    modoAutomatico = !modoAutomatico;
  }
  botonPrev = autoAhora;

  if (modoAutomatico) {
    int luz = ldr.leer();
    bool estaOscuro = (luz < umbralLuz);
    luzManual.escribir(0);
    delay(20);
    luzAuto.escribir(estaOscuro ? 1 : 0);
  }
  else{
    luzAuto.escribir(0);
    delay(20);
    luzManual.escribir(1);
  }

  delay(20);

  
  


};
