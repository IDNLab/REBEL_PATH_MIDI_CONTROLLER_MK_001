# REBEL_PATH_MIDI_CONTROLLER_MK_001

Un controller MIDI hardware basato su Arduino che legge 16 potenziometri tramite un multiplexer e invia messaggi MIDI Control Change via USB.

## Caratteristiche

- Lettura di 16 potenziometri con un singolo ingresso analogico tramite multiplexer 16-canali
- Visualizzazione dei valori e dello shift corrente su un display LCD I2C 16x2
- Due pulsanti per cambiare lo shift dei Control Change (CC)
- Invio di messaggi MIDI CC via USB (compatibile con Arduino Leonardo / Micro o altre schede con MIDIUSB)
- Filtro anti-jitter e smoothing per valori potenziometro stabili
- **Supporto per molteplici schede di controllo Arduino**

## Struttura del progetto

### Script Arduino (SWF/INO/)

Gli script nella cartella `SWF/INO/` sono stati adattati per diverse schede di controllo:

- `midicontroller_mark_001.ino` - Sketch principale (versione base)
- `midicontroller_mark_001_Arduino_Leonardo.ino` - Versione ottimizzata per **Arduino Leonardo**

**Nota**: Ogni file è specifico per la scheda di controllo indicata nel nome. Scegli lo script compatibile con la tua scheda.

## Requisiti software

- Arduino IDE o piattaforma compatibile
- Librerie Arduino:
  - `MIDIUSB`
  - `Wire`
  - `LiquidCrystal_I2C`

## Compatibilità con le schede

Questo progetto supporta schede Arduino con:
- Supporto per MIDIUSB (nativamente o tramite libreria)
- Almeno 4 pin digitali disponibili (per il multiplexer S0-S3)
- Almeno 1 ingresso analogico (A0)
- 2 pin digitali aggiuntivi per i pulsanti
- Supporto I2C per il display LCD

### Schede testate

- **Arduino Leonardo** ✓ (supporto MIDIUSB nativo)
- **Arduino Micro** ✓ (supporto MIDIUSB nativo)

Per altre schede, consulta la documentazione della libreria MIDIUSB.

## Pin e connessioni

### Multiplexer 16 canali

- `S0` -> digitale 8
- `S1` -> digitale 9
- `S2` -> digitale 10
- `S3` -> digitale 11
- Segnale di uscita del MUX -> analogico `A0`

### Display LCD I2C

- SDA -> pin I2C SDA della scheda
- SCL -> pin I2C SCL della scheda
- Indirizzo I2C configurato in `0x27`

### Pulsanti

- `BTN_UP` -> digitale 7 (pulsante incremento shift)
- `BTN_DOWN` -> digitale 6 (pulsante decremento shift)
- Entrambi i pulsanti usano `INPUT_PULLUP`

## Funzionamento

- Ogni potenziometro invia un valore analogico letto attraverso il multiplexer
- Il codice converte il valore ADC (0-1023) in valori MIDI 0-127
- Vengono inviati messaggi Control Change tramite `MIDIUSB`
- Lo `shift` permette di spostare l'intervallo CC di 16 posizioni, abilitando fino a 128 CC totali
- Il display mostra il CC corrente, il valore MIDI e il valore di shift

## Upload

1. **Seleziona lo script corretto** in base alla tua scheda Arduino
2. Apri lo script nella cartella `SWF/INO/` con l'Arduino IDE
3. Seleziona la scheda corretta dal menu **Strumenti > Scheda**
4. Seleziona la porta USB corretta
5. Compila e carica lo sketch

## Configurazione personalizzata

Se utilizzi una scheda diversa da quelle testate, puoi adattare lo script modificando i pin nel namespace `Config`:

```cpp
namespace Config {
  const uint8_t S0 = 8;               // Personalizza questi pin
  const uint8_t S1 = 9;               // in base alla tua scheda
  const uint8_t S2 = 10;
  const uint8_t S3 = 11;
  const uint8_t BTN_UP = 7;
  const uint8_t BTN_DOWN = 6;
  const uint8_t MUX_SIG = A0;
}
```

## Note

- Assicurati che l'indirizzo I2C del display sia corretto; se il display non si accende, verifica l'indirizzo e aggiorna `Config::LCD_ADDR`
- Verifica la compatibilità della tua scheda con la libreria `MIDIUSB` prima di caricare il codice
- Se necessario, crea una versione personalizzata dello script per la tua scheda seguendo il pattern dei file esistenti
- Il codice è già predisposto per ridurre il jitter dei potenziometri con una soglia minima (`DEADZONE`) e smoothing
