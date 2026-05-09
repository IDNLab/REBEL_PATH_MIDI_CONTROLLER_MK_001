# REBEL_PATH_MIDI_CONTROLLER_MK_001

Un controller MIDI hardware basato su Arduino che legge 16 potenziometri tramite un multiplexer e invia messaggi MIDI Control Change via USB.

## Caratteristiche

- Lettura di 16 potenziometri con un singolo ingresso analogico tramite multiplexer 16-canali
- Visualizzazione dei valori e dello shift corrente su un display LCD I2C 16x2
- Due pulsanti per cambiare lo shift dei Control Change (CC)
- Invio di messaggi MIDI CC via USB (compatibile con Arduino Leonardo / Micro o altre schede con MIDIUSB)
- Filtro anti-jitter e smoothing per valori potenziometro stabili

## Struttura del progetto

- `SWF/INO/midicontroller_mark_001.ino`: sketch Arduino principale

## Requisiti software

- Arduino IDE o piattaforma compatibile
- Librerie Arduino:
  - `MIDIUSB`
  - `Wire`
  - `LiquidCrystal_I2C`

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

1. Apri `SWF/INO/midicontroller_mark_001.ino` nell'Arduino IDE
2. Seleziona la scheda compatibile con MIDIUSB (es. Arduino Leonardo o Pro Micro)
3. Seleziona la porta USB corretta
4. Compila e carica lo sketch

## Note

- Assicurati che l'indirizzo I2C del display sia corretto; se il display non si accende, verifica l'indirizzo e aggiorna `Config::LCD_ADDR`
- Se usi un'altra scheda, verifica la compatibilità con la libreria `MIDIUSB`
- Il codice è già predisposto per ridurre il jitter dei potenziometri con una soglia minima (`DEADZONE`) e smoothing
