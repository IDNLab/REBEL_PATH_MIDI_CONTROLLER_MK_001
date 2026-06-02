//Modifiche al codice principale per funzionare con la versione Arduino Pro Micro

#include <MIDIUSB.h>              // Libreria per inviare MIDI via USB (Leonardo)
#include <Wire.h>                 // Libreria I2C (necessaria per LCD)
#include <LiquidCrystal_I2C.h>    // Libreria per LCD I2C

// ======================
// CONFIG
// ======================
namespace Config {

  const uint8_t NUM_POTS = 16;        // Numero di potenziometri collegati al multiplexer

  // LCD
  const uint8_t LCD_ADDR = 0x27;      // Indirizzo I2C del display
  const uint8_t LCD_COLS = 16;        // Colonne LCD
  const uint8_t LCD_ROWS = 2;         // Righe LCD

  // MUX
  const uint8_t MUX_SIG = A0;         // Pin analogico dove arriva il segnale dal MUX
  const uint8_t S0 = 8;               // Pin selezione MUX bit 0
  const uint8_t S1 = 9;               // Pin selezione MUX bit 1
  const uint8_t S2 = 10;              // Pin selezione MUX bit 2
  const uint8_t S3 = 15;              // Pin selezione MUX bit 3

  // Bottoni
  const uint8_t BTN_UP = 5;           // Bottone incremento shift
  const uint8_t BTN_DOWN = 4;         // Bottone decremento shift

  // Filtro
  const uint8_t DEADZONE = 1;         // Soglia minima per considerare un cambiamento valido
  const uint8_t SMOOTH_K = 1;         // Fattore smoothing (media esponenziale)
}

// ======================
// UTILS
// ======================
inline int toMidi(int v) {             // Funzione inline per convertire valori analogici
  return map(v, 1023, 0, 0, 127);     // Mappa da 0-1023 (ADC) a 0-127 (MIDI)
}

// ======================
// MUX
// ======================
class Mux16 {
public:
  void begin() {
    pinMode(Config::S0, OUTPUT);      // Imposta pin selezione come output
    pinMode(Config::S1, OUTPUT);
    pinMode(Config::S2, OUTPUT);
    pinMode(Config::S3, OUTPUT);
  }

  int read(uint8_t ch) {              // Legge un canale del multiplexer
    select(ch);                       // Seleziona il canale
    // rimossa per vedere come andiamo -->delayMicroseconds(10);            // Attesa stabilizzazione segnale
    analogRead(Config::MUX_SIG);      // Dummy read (scarta prima lettura instabile)
    // rimossa per vedere come andiamo -->delayMicroseconds(5);             // Piccola attesa
    return analogRead(Config::MUX_SIG); // Lettura reale stabile
  }

private:
  void select(uint8_t ch) {           // Imposta i pin S0-S3 in base al canale
    digitalWrite(Config::S0, ch & 1);
    digitalWrite(Config::S1, (ch >> 1) & 1);
    digitalWrite(Config::S2, (ch >> 2) & 1);
    digitalWrite(Config::S3, (ch >> 3) & 1);
  }
};

// ======================
// POT (stato per canale)
// ======================
struct Pot {
  int raw = 0;                        // Valore grezzo letto
  int smooth = 0;                     // Valore filtrato
  int lastMidi = -1;                  // Ultimo valore MIDI inviato

  bool update(int newRaw) {           // Aggiorna lo stato del pot
    raw = newRaw;                     // Salva valore raw

    // Filtro smoothing (media esponenziale)
    smooth = (smooth * (Config::SMOOTH_K - 1) + raw) / Config::SMOOTH_K;

    int midi = toMidi(smooth);        // Converte in valore MIDI

    // Se primo valore o cambiamento significativo
    if (lastMidi == -1 || abs(midi - lastMidi) > Config::DEADZONE) {
      lastMidi = midi;                // Aggiorna ultimo valore
      return true;                    // Segnala che è cambiato
    }
    return false;                     // Ignora piccoli jitter
  }
};

// ======================
// BOTTONI
// ======================
class Buttons {
public:
  void begin() {
    pinMode(Config::BTN_UP, INPUT_PULLUP);    // Bottone con pull-up interno
    pinMode(Config::BTN_DOWN, INPUT_PULLUP);
  }

  int poll() {                      // Controlla stato bottoni
    static bool lastUp = HIGH;      // Stato precedente bottone UP
    static bool lastDown = HIGH;    // Stato precedente bottone DOWN
    static unsigned long lastT = 0; // Tempo ultimo debounce

    int delta = 0;                  // Valore da restituire
    unsigned long now = millis();   // Tempo attuale

    bool up = digitalRead(Config::BTN_UP);      // Legge bottone UP
    bool down = digitalRead(Config::BTN_DOWN);  // Legge bottone DOWN

    if (now - lastT > 50) {         // Debounce 50ms

      if (up == LOW && lastUp == HIGH) { // Fronte di discesa (pressione)
        delta = +1;                 // Incrementa shift
        lastT = now;
      }

      if (down == LOW && lastDown == HIGH) {
        delta = -1;                 // Decrementa shift
        lastT = now;
      }
    }

    lastUp = up;                    // Aggiorna stato precedente
    lastDown = down;

    return delta;                   // Ritorna variazione
  }
};

// ======================
// MIDI
// ======================
namespace Midi {
  void sendCC(uint8_t cc, uint8_t val) {     // Invio Control Change
    midiEventPacket_t ev = {0x0B, 0xB0, cc, val}; // Pacchetto MIDI (canale 1)
    Serial.print("DEBUG MIDI: Status=0x");
    Serial.print(ev.header, HEX);
    Serial.print(" Command=0x");
    Serial.print(ev.byte1, HEX);
    Serial.print(" CC=");
    Serial.print(ev.byte2);
    Serial.print(" Val=");
    Serial.println(ev.byte3);
    MidiUSB.sendMIDI(ev);                    // Invia
    MidiUSB.flush();                         // Forza trasmissione
  }
}

// ======================
// DISPLAY
// ======================
class Display {
public:
  void begin() {
    lcd.init();               // Inizializza LCD
    lcd.backlight();          // Accende retroilluminazione
    showWelcome();            // Mostra schermata iniziale
  }

  void show(uint8_t cc, uint8_t val, uint8_t shift) {
    // Evita aggiornamenti inutili (no flicker)
    if (cc == lastCC && val == lastVal && shift == lastShift) return;

    lcd.setCursor(0, 0);      // Riga 1
    lcd.print("CC:");
    lcd.print(cc);
    lcd.print("    ");        // Cancella residui

    lcd.setCursor(0, 1);      // Riga 2
    lcd.print("Val:");
    lcd.print(val);
    lcd.print("   ");

    lcd.setCursor(10, 1);     // Posizione shift
    lcd.print("S:");
    lcd.print(shift);
    lcd.print(" ");

    // Salva stato mostrato
    lastCC = cc;
    lastVal = val;
    lastShift = shift;
  }

private:
  LiquidCrystal_I2C lcd{Config::LCD_ADDR, Config::LCD_COLS, Config::LCD_ROWS};

  int lastCC = -1;
  int lastVal = -1;
  int lastShift = -1;

  void showWelcome() {
    lcd.setCursor(0, 0);
    lcd.print("MIDI Controller");   // Messaggio iniziale
    lcd.setCursor(0, 1);
    lcd.print("Init...");
    delay(2000);                    // Mostra per 2 secondi
    lcd.clear();                    // Pulisce display
  }
};

// ======================
// APP
// ======================
Mux16 mux;                          // Oggetto multiplexer
Buttons buttons;                    // Oggetto bottoni
Display display;                    // Oggetto display
Pot pots[Config::NUM_POTS];         // Array di potenziometri

int shift = 0;                      // Offset CC

// ======================
// SETUP
// ======================
void setup() {

  Serial.begin(115200);             // Avvia seriale

  unsigned long t0 = millis();      
  while (!Serial && millis() - t0 < 1500); // Attesa connessione seriale (max 1.5s)

  mux.begin();                      // Inizializza multiplexer
  buttons.begin();                  // Inizializza bottoni
  display.begin();                  // Inizializza display

  Serial.println("Ready");          // Debug
}

// ======================
// LOOP
// ======================
void loop() {

  int delta = buttons.poll();       // Legge bottoni

  if (delta != 0) {
    shift = constrain(shift + delta, 0, 7); // Aggiorna shift (0-7)
    Serial.print("SHIFT: ");
    Serial.println(shift);
  }
  // ONLY FOR DEBUG Serial.println("inizio ciclo: ");
  for (uint8_t i = 0; i < Config::NUM_POTS; i++) {

    int raw = mux.read(i);          // Legge potenziometro

    if (pots[i].update(raw)) {      // Se valore valido cambiato

      uint8_t cc = i + shift * 16;  // Calcola Control Change
      uint8_t val = pots[i].lastMidi;

      Midi::sendCC(cc, val);        // Invia MIDI
      display.show(cc, val, shift); // Aggiorna LCD

      Serial.print("CC ");          // Debug seriale
      Serial.print(cc);
      Serial.print(" = ");
      Serial.println(val);
    }
  }
}
