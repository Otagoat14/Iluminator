// ========== ARDUINO PANTALLA - DISPLAY LCD (ESCLAVO I2C) ==========

#include <Wire.h>
#include <LiquidCrystal.h>

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

class Pantalla : public Componente {
private:
  LiquidCrystal* lcd;
  
public:
  Pantalla(const char* n, int rs, int en, int d4, int d5, int d6, int d7) {
    Nombre = n;
    lcd = new LiquidCrystal(rs, en, d4, d5, d6, d7);
  }

  void iniciar() override {
    lcd->begin(16, 2);
    lcd->clear();
    lcd->print("Iniciando...");
    delay(1000);
    lcd->clear();
  }

  void escribir(int v) override {
    // No usado para LCD
  }

  void mostrarLinea(int fila, String texto) {
    lcd->setCursor(0, fila);
    // Rellenar con espacios para limpiar caracteres anteriores
    String textoCompleto = texto;
    while (textoCompleto.length() < 16) {
      textoCompleto += " ";
    }
    lcd->print(textoCompleto.substring(0, 16));
  }

  void limpiar() {
    lcd->clear();
  }

  ~Pantalla() {
    delete lcd;
  }
};

// Dirección I2C de este Arduino (debe ser única)
const int MI_DIRECCION = 8;

// Crear la pantalla LCD (ajusta los pines según tu conexión)
// LiquidCrystal(rs, en, d4, d5, d6, d7)
Pantalla pantallaLCD("LCD", 12, 11, 5, 4, 3, 2);

// Variables para almacenar los datos recibidos
bool modoAutomatico = true;
int nivelLuz = 0;
int ledsEncendidos = 0;
int valorLDR = 0;

String nombreModo[] = {"Relajacion", "Noche", "Estudio", "Fiesta"};

// Buffer para recibir datos I2C
byte datosRecibidos[5];
int indiceDatos = 0;
bool datosCompletos = false;

void recibirDatos(int numBytes) {
  indiceDatos = 0;
  
  while (Wire.available() && indiceDatos < 5) {
    datosRecibidos[indiceDatos] = Wire.read();
    indiceDatos++;
  }
  
  if (indiceDatos == 5) {
    datosCompletos = true;
  }
}

void procesarDatos() {
  if (datosCompletos) {
    modoAutomatico = (datosRecibidos[0] == 1);
    nivelLuz = datosRecibidos[1];
    ledsEncendidos = datosRecibidos[2];
    
    // Reconstruir el valor LDR de 2 bytes
    valorLDR = (datosRecibidos[3] << 8) | datosRecibidos[4];
    
    datosCompletos = false;
  }
}

void actualizarPantalla() {
  // Línea 1: Modo y nivel de luz
  String modo = modoAutomatico ? "AUTO" : "MANUAL";
  String linea1 = modo + " " + nombreModo[nivelLuz];
  
  // Línea 2: LEDs encendidos y valor LDR
  String linea2 = "L:" + String(ledsEncendidos) + " LDR:" + String(valorLDR);
  
  pantallaLCD.mostrarLinea(0, linea1);
  pantallaLCD.mostrarLinea(1, linea2);
}

void setup() {
  Serial.begin(9600); // Para debug si lo necesitas
  
  Wire.begin(MI_DIRECCION); // Iniciar como esclavo I2C
  Wire.onReceive(recibirDatos); // Registrar función de recepción
  
  pantallaLCD.iniciar();
}

void loop() {
  // Procesar y actualizar pantalla si hay datos nuevos
  if (datosCompletos) {
    procesarDatos();
    actualizarPantalla();
  }
  
  delay(50);
}