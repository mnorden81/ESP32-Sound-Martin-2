/*
   ESP32-RC-Sound
   Ziege-One (Der RC-Modellbauer)
 
 ESP32 + SD + S2I Audio

 /////Pin Belegung////
 GPIO 13: Wifi Pin
 GPIO 14:               Input 3 (V2)
 GPIO 27:               Input 4 (V2)
 GPIO 33:               Input 5 (V2) 
 GPIO 32:               Input 6 (V2) 
 GPIO 15: 
 GPIO 16: Input 1 (V1)  Input 1 (V2)
 GPIO 17: Input 2 (V1)  Input 2 (V2)
 GPIO 22: Input 3 (V1)
 GPIO 00: Input 4 (V1)
 GPIO 02: Input 5 (V1)
 GPIO 04: Input 6 (V1)

 GPIO 05: SD_CS
 GPIO 18: SD_CLK  
 GPIO 19: SD_MISO
 GPIO 23: SD_MOSI

 GPIO 21: I2S_DOUT
 GPIO 25: I2S_LRC
 GPIO 26: I2S_BCLK

 */

// ======== ESP32-RC-Sound =======================================

/* Boardversion
ESP32                                         3.3.8
 */

/* Installierte Bibliotheken
Bolder Flight Systems SBUS                    8.1.4
CRSF_ESP32                                    https://github.com/Ziege-One/CRSF_ESP32
 */

#include <Arduino.h>
#include "XT_I2S_Audio.h"
#include <WiFi.h>             // WLAN-Funktionalität für ESP32
#include "sbus.h"
#include "crsf_esp32.h"       // CRSF-Protokoll (Crossfire) für serielle Übertragung

#include "config.h"           // Eigene Konfigurationsdatei (Version, Pins, Optionen)
#include "WebServerManager.h" // Webseite 

// Software Version
uint16_t Version = 41;        // 101 entspricht 1.01
char versionString[6];        // Zeichenkette zur Anzeige der Version

bool Source_Start_Sound_OK[9] = {false, false, false, false, false, false, false, false, false};// Source_Start_Sound 1-9 Quellen OK (SBUS oder PWM vorhanden)
bool Sound_on[9] = {false, false, false, false, false, false, false, false, false};        // Sound starten 1-9
bool Sound_play[9] = {false, false, false, false, false, false, false, false, false};      // Sound spielt 1-9
bool Sound_on_web[9] = {false, false, false, false, false, false, false, false, false};    // Sound starten über Webinterface 1-9

bool Sound_on_Motor;       // Sound Motor starten
bool Sound_on_Motor_state; // Sound Motor Toggle Speicher

// EinKanalModus
int Einkanal_RC_System_boot;                          // Bootstatus des RC-Systems
uint16_t einkanal_Data;   // Daten für die Ausgänge
uint16_t einkanal_SpeicherWM = 0;   // Speicher WM

//EbenenUmschaltung
int Source_Ebenen_Um_Kanal_wert = 0;
int Source_Ebenen_Kanal_wert = 0;
uint16_t ebenenkanal_Data = 99;         // Daten für die Ausgänge 

// SD Card 
#define SD_CS 5 // SD Card chip select

// I2S Audio
#define I2S_DOUT 21 // i2S Data out oin
#define I2S_BCLK 26 // Bit clock
#define I2S_LRC  25 // Left/Right clock, also known as Frame clock or word select

// Pins zuweisen 
volatile unsigned char WifiPin = 13;
volatile unsigned char LedPin = 2;
volatile unsigned char Input_Pin[6]  ={16, 17, 22, 0, 2, 4}; // Pin-Eingang

// Motor
bool engine_break;
unsigned long shutdown_timer;
volatile uint8_t engine_State = 0;                        // Motor Status 
enum Engine_State                                         // Motor Status enum
{
  OFF,
  STARTING,
  RUNNING,
  STOPPING,
};

// Ramp
unsigned long previousTime_ramp = 0;         // Previous time
float last_throttle = 0;
// int throttle_ramp = 20;       // max change of throttle in one Second
float throttle;      
// int throttle_dead_band = 10;  //Totband Gas

// Wlan Einstellungen AP Modus - werden aus config geladen
char ssid[32];
char password[64];

//Timer Zeiten
unsigned long currentTime = millis();   // Aktuelle Zeit
unsigned long previousTime = 0;         // Previous time
//const long timeoutTime = 2000;          // Define timeout time in milliseconds (example: 2000ms = 2s)

// SBUS
bfs::SbusRx sbus_rx(&Serial2,16, 27,true); // Sbus auf Serial2

bfs::SbusData sbus_data;

bool BUS_OK = false;

// CRSF-Kommunikation
CRSF crsf;                                   // Instanz für CRSF-Protokoll (Crossfire)

// Multiswitch-Kommandos (CRSF Payload-Codes)
#define MWset   0x01   // Setzen eines einzelnen Schalters
#define MWprop  0x02   // Propeller-/Eigenschaftsbefehl
#define MWset4  0x07   // Setzen von 4 Schaltern gleichzeitig
#define MWset4m 0x09   // Setzen von 4 Schaltern mit Maske

// CRSF-Kommandobereich für Multiswitch
#define Multiswitch 0xA1   // Kennung für Multiswitch-Kommandos

//Kanalwert Speicher SBUS oder CRSF
uint16_t channel_output[16];

// I2S 
XT_I2S_Class I2SAudio(I2S_LRC, I2S_BCLK, I2S_DOUT, I2S_NUM_0);

// Sounds Laden 
XT_Wav_Class Sound_loop("/loop.wav");
XT_Wav_Class Sound_shut("/shut.wav");
XT_Wav_Class Sound_start("/start.wav");
XT_Wav_Class Sound1("/sound1.wav");
XT_Wav_Class Sound2("/sound2.wav");
XT_Wav_Class Sound3("/sound3.wav");
XT_Wav_Class Sound4("/sound4.wav");
XT_Wav_Class Sound5("/sound5.wav");
XT_Wav_Class Sound6("/sound6.wav");
XT_Wav_Class Sound7("/sound7.wav");
XT_Wav_Class Sound8("/sound8.wav");

// PWM lesen mit Interrupt
volatile unsigned int PWM_pulse_width[6] = {3000, 3000, 3000, 3000, 3000, 3000};  
volatile unsigned int PWM_prev_time[6] = {0, 0, 0, 0, 0, 0}; 


void ISR_PWM_0() {
  if (digitalRead(Input_Pin[0])) {
    // Steigende Flanke → Startzeit speichern
    PWM_prev_time[0] = micros();
  } else {
    // Fallende Flanke → Pulsweite berechnen
    PWM_pulse_width[0] = micros() - PWM_prev_time[0];
  }
}

void ISR_PWM_1() {
  if (digitalRead(Input_Pin[1])) {
    // Steigende Flanke → Startzeit speichern
    PWM_prev_time[1] = micros();
  } else {
    // Fallende Flanke → Pulsweite berechnen
    PWM_pulse_width[1] = micros() - PWM_prev_time[1];
  }
}

void ISR_PWM_2() {
  if (digitalRead(Input_Pin[2])) {
    // Steigende Flanke → Startzeit speichern
    PWM_prev_time[2] = micros();
  } else {
    // Fallende Flanke → Pulsweite berechnen
    PWM_pulse_width[2] = micros() - PWM_prev_time[2];
  }
}

void ISR_PWM_3() {
  if (digitalRead(Input_Pin[3])) {
    // Steigende Flanke → Startzeit speichern
    PWM_prev_time[3] = micros();
  } else {
    // Fallende Flanke → Pulsweite berechnen
    PWM_pulse_width[3] = micros() - PWM_prev_time[3];
  }
}

void ISR_PWM_4() {
  if (digitalRead(Input_Pin[4])) {
    // Steigende Flanke → Startzeit speichern
    PWM_prev_time[4] = micros();
  } else {
    // Fallende Flanke → Pulsweite berechnen
    PWM_pulse_width[4] = micros() - PWM_prev_time[4];
  }
}

void ISR_PWM_5() {
  if (digitalRead(Input_Pin[5])) {
    // Steigende Flanke → Startzeit speichern
    PWM_prev_time[5] = micros();
  } else {
    // Fallende Flanke → Pulsweite berechnen
    PWM_pulse_width[5] = micros() - PWM_prev_time[5];
  }
}

// ======== Setup =======================================
void setup()
{
  Serial.begin(115200); // Used for info/debug

  loadConfig();    // Einstellung laden
  
  // WiFi-Credentials aus config in char-Arrays kopieren
  strncpy(ssid, config.WiFi_SSID, sizeof(ssid) - 1);
  ssid[sizeof(ssid) - 1] = '\0';
  strncpy(password, config.WiFi_Password, sizeof(password) - 1);
  password[sizeof(password) - 1] = '\0';
  
  // Prüfen, welches RC-System konfiguriert ist
  if (config.Einkanal_RC_System == 4) { // 4 = ELRS (CRSF-Protokoll)
    // CRSF-Schnittstelle initialisieren:
    // Übergabe: HardwareSerial-Instanz & RX/TX-Pins für das Crossfire-Modul
    crsf.init_crsf(&Serial1, 16, 22); //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!testen
  } else {
    // Andernfalls SBUS-Kommunikation starten
    sbus_rx.Begin();
  } 

  // Config Modus
  if (config.Hardware_Config == 0)
  {
    Serial.print("Config V1");
    Input_Pin[0] = 16; // Pin-Eingang
    Input_Pin[1] = 17; // Pin-Eingang
    Input_Pin[2] = 22; // Pin-Eingang
    Input_Pin[3] =  0; // Pin-Eingang
    Input_Pin[4] =  2; // Pin-Eingang
    Input_Pin[5] =  4; // Pin-Eingang
  }
  else if (config.Hardware_Config == 1)
  {
    Serial.print("Config V2");
    Input_Pin[0] = 16; // Pin-Eingang
    Input_Pin[1] = 17; // Pin-Eingang
    Input_Pin[2] = 14; // Pin-Eingang
    Input_Pin[3] = 27; // Pin-Eingang
    Input_Pin[4] = 32; // Pin-Eingang
    Input_Pin[5] = 33; // Pin-Eingang 
  }

  pinMode(WifiPin, INPUT_PULLUP);
  //pinMode(LedPin, OUTPUT);

  //pinMode(Input_Pin[0], INPUT_PULLUP); 
  pinMode(Input_Pin[1], INPUT_PULLUP); 
  pinMode(Input_Pin[2], INPUT_PULLUP);    
  pinMode(Input_Pin[3], INPUT_PULLUP);    
  pinMode(Input_Pin[4], INPUT_PULLUP); 
  pinMode(Input_Pin[5], INPUT_PULLUP); 
  
  SDCardInit();
  LoadFiles();  // lade Wave Dateien

  if (!digitalRead(WifiPin)) // Setup Modus
  {
    //digitalWrite(LedPin, HIGH);  
    Serial.print("AP (Zugangspunkt) einstellen…");
    WebServerManager::begin(ssid, password); // SSID, Passwort
  }
  else
  {
    Serial.println("Kein AP-Normalmodus");  
  }

  // Bootstatus des RC-Systems speichern
  Einkanal_RC_System_boot = config.Einkanal_RC_System;

  // Versionsnummer als Zeichenkette formatieren (z. B. 1.01)
  sprintf(versionString, "%d.%02d", Version / 100, Version % 100);
}


// ======== Loop =======================================
void loop()
{
  currentTime = millis();           // Aktuelle Zeit erneuern
  
  if (!digitalRead(WifiPin))        // Setup Modus
  {     
    WebServerManager::Webpage();                      // Webseite laden
  }
  
  // Prüfen, ob das RC-System beim Booten auf ELRS (CRSF) gesetzt wurde
  if (Einkanal_RC_System_boot == 4) { // ELRS (CRSF)
  
    // CRSF-Pakete vom Empfänger lesen (z. B. Link-Status, Telemetrie, Steuerbefehle)
    crsf.read_packets(0);  // Parameter 0 = keine Debug-Ausgabe

    BUS_OK = true;                 // SBUS ist vorhanden
    
    // Prüfen, ob ein direkter Gerätebefehl empfangen wurde
    if (crsf.getDeviceCommandReplyPending())
    {
      // Rückmeldeflag zurücksetzen, damit der Befehl nicht mehrfach verarbeitet wird
      crsf.setDeviceCommandReplyPending(false);

      // Funktion zur Verarbeitung des CRSF-Kommandos aufrufen
      einkanalFunctionCRSF();
    }
    
    //Kanal Werte in den Speicher legen
    for (int i = 0; i < 16; i++) {
    channel_output[i] = crsf.get_crfs_channels(i); 
    } 

  } else {
    // SBUS-Modus (klassische RC-Signale)
    if (sbus_rx.Read()) {                 // Prüfen, ob neue SBUS-Daten vorliegen
      sbus_data = sbus_rx.data();         // SBUS-Datenstruktur aktualisieren
      BUS_OK = true;                 // SBUS ist vorhanden
      if (sbus_data.failsafe) {           // Falls Failsafe aktiv → Ausgänge ausschalten
         //Output(0);                       // Alle Ausgänge auf 0 setzen
      }
      else
      {
        if (config.Einkanal_mode != 999)       // Einkanal aktiviert
        {
          // SBUS-Kanalwert auslesen und in interne Datenstruktur codieren
          einkanalFunction(sbus_data.ch[config.Einkanal_Channel]);             // Einkanal auswerten     
        }  
      }

      //Kanal Werte in den Speicher legen
      for (int i = 0; i < 16; i++) {
      channel_output[i] = sbus_data.ch[i]; 
      } 

    }
  } 

  Config();                         // Einstellungen anwenden
  
  // FillBuffer wird jetzt von einem separaten Producer Task aufgerufen!
  // I2SAudio.FillBuffer();  // <- REMOVED, Producer Task übernimmt das

  if (currentTime > 5000)  //Startup warte eben 5 Sekunden
  {
    ebenenFunction();


  if (Sound_on[1] or Sound_on_web[1])    // Spiele Sound 1
  {
    if (not Sound_play[1])
    {  
      Sound_play[1] = true;
      Sound1.LoadWavFile();
      Sound1.Volume = config.Volumen_Sound[1];
      if (config.Mode_Sound[1] == 1)            // Repeat ?
      {
        Sound1.RepeatForever  = true;
      }
      I2SAudio.Play(&Sound1);
      Sound_on_web[1] = false;   
    }
  }
  else
  {
    Sound1.RepeatForever = false;
    
    if (config.Mode_Sound[1] == 2 and Sound_play[1]) // Tippbetrieb ?
    {
      I2SAudio.Stop(&Sound1);
    }
       
    if (not Sound1.Playing and Sound_play[1])
    {
      Sound1.UnLoadWavFile();
      Sound_play[1] = false;      
    }    
  } 
  
  if (Sound_on[2] or Sound_on_web[2])    // Spiele Sound 2
  { 
    if (not Sound_play[2])
    {      
      Sound_play[2] = true;
      Sound2.LoadWavFile();
      Sound2.Volume = config.Volumen_Sound[2];
      if (config.Mode_Sound[2] == 1)            // Repeat ?
      {
        Sound2.RepeatForever  = true;
      }
      I2SAudio.Play(&Sound2);
      Sound_on_web[2] = false;
    }    
  }
  else
  {
    Sound2.RepeatForever = false;
    
    if (config.Mode_Sound[2] == 2 and Sound_play[2]) // Tippbetrieb ?
    {
      I2SAudio.Stop(&Sound2);
    }
       
    if (not Sound2.Playing and Sound_play[2])
    {
      Sound2.UnLoadWavFile();
      Sound_play[2] = false;      
    }    
  } 

  if (Sound_on[3] or Sound_on_web[3])    // Spiele Sound 3
  {
    if (not Sound_play[3])
    {  
      Sound_play[3] = true;   
      Sound3.LoadWavFile();
        Serial.print("Load Sound3: "); Serial.println(Sound3.FileName);
        Serial.print("Sound3.FileOK = "); Serial.println(Sound3.FileOK);
        Serial.print("Sound3.SampleRate = "); Serial.println(Sound3.SampleRate);
        Serial.print("Sound3.NumChannels = "); Serial.println(Sound3.NumChannels);
        Serial.print("Sound3.BytesPerSample = "); Serial.println(Sound3.BytesPerSample);
        Serial.print("Sound3.DataSize = "); Serial.println(Sound3.DataSize);
      Sound3.Volume = config.Volumen_Sound[3];
      if (config.Mode_Sound[3] == 1)            // Repeat ?
      {
        Sound3.RepeatForever  = true;
      }
      I2SAudio.Play(&Sound3);
      Sound_on_web[3] = false;      
    }
  }
  else
  {
    Sound3.RepeatForever = false;
    
    if (config.Mode_Sound[3] == 2 and Sound_play[3]) // Tippbetrieb ?
    {
      I2SAudio.Stop(&Sound3);
    }
       
    if (not Sound3.Playing and Sound_play[3])
    {
      Sound3.UnLoadWavFile();
      Sound_play[3] = false;
    }    
  } 

  if (Sound_on[4] or Sound_on_web[4])    // Spiele Sound 4
  {
    if (not Sound_play[4])
    {      
      Sound_play[4] = true;  
      Sound4.LoadWavFile();
      Sound4.Volume = config.Volumen_Sound[4];
      if (config.Mode_Sound[4] == 1)            // Repeat ?
      {
        Sound4.RepeatForever  = true;
      }
      I2SAudio.Play(&Sound4);
      Sound_on_web[4] = false;      
    }
  }
  else
  {
    Sound4.RepeatForever = false;
    
    if (config.Mode_Sound[4] == 2 and Sound_play[4]) // Tippbetrieb ?
    {
      I2SAudio.Stop(&Sound4);
    }
       
    if (not Sound4.Playing and Sound_play[4])
    {
      Sound4.UnLoadWavFile();
      Sound_play[4] = false;
    }    
  }   

  if (Sound_on[5] or Sound_on_web[5])    // Spiele Sound 5
  {
    if (not Sound_play[5])
    {      
      Sound_play[5] = true;  
      Sound5.LoadWavFile();
      Sound5.Volume = config.Volumen_Sound[5];
      if (config.Mode_Sound[5] == 1)            // Repeat ?
      {
        Sound5.RepeatForever  = true;
      }
      I2SAudio.Play(&Sound5);
      Sound_on_web[5] = false;      
    }
  }
  else
  {
    Sound5.RepeatForever = false;
    
    if (config.Mode_Sound[5] == 2 and Sound_play[5]) // Tippbetrieb ?
    {
      I2SAudio.Stop(&Sound5);
    }
       
    if (not Sound5.Playing and Sound_play[5])
    {
      Sound5.UnLoadWavFile();
      Sound_play[5] = false;
    }    
  }  

  if (Sound_on[6] or Sound_on_web[6])    // Spiele Sound 6
  {
    if (not Sound_play[6])
    {      
      Sound_play[6] = true;  
      Sound6.LoadWavFile();
      Sound6.Volume = config.Volumen_Sound[6];
      if (config.Mode_Sound[6] == 1)            // Repeat ?
      {
        Sound6.RepeatForever  = true;
      }
      I2SAudio.Play(&Sound6);
      Sound_on_web[6] = false;      
    }
  }
  else
  {
    Sound6.RepeatForever = false;
    
    if (config.Mode_Sound[6] == 2 and Sound_play[6]) // Tippbetrieb ?
    {
      I2SAudio.Stop(&Sound6);
    }
       
    if (not Sound6.Playing and Sound_play[6])
    {
      Sound6.UnLoadWavFile();
      Sound_play[6] = false;
    }    
  }  

  if (Sound_on[7] or Sound_on_web[7])    // Spiele Sound 7
  {
    if (not Sound_play[7])
    {      
      Sound_play[7] = true;  
      Sound7.LoadWavFile();
      Sound7.Volume = config.Volumen_Sound[7];
      if (config.Mode_Sound[7] == 1)            // Repeat ?
      {
        Sound7.RepeatForever  = true;
      }
      I2SAudio.Play(&Sound7);
      Sound_on_web[7] = false;      
    }
  }
  else
  {
    Sound7.RepeatForever = false;
    
    if (config.Mode_Sound[7] == 2 and Sound_play[7]) // Tippbetrieb ?
    {
      I2SAudio.Stop(&Sound7);
    }
        
    if (not Sound7.Playing and Sound_play[7])
    {
      Sound7.UnLoadWavFile();
      Sound_play[7] = false;
    }    
  }  

  if (Sound_on[8] or Sound_on_web[8])    // Spiele Sound 8
  {
    if (not Sound_play[8])
    {      
      Sound_play[8] = true;  
      Sound8.LoadWavFile();
      Sound8.Volume = config.Volumen_Sound[8];
      if (config.Mode_Sound[8] == 1)            // Repeat ?
      {
        Sound8.RepeatForever  = true;
      }
      I2SAudio.Play(&Sound8);
      Sound_on_web[8] = false;      
    }
  }
  else
  {
    Sound8.RepeatForever = false;

    if (config.Mode_Sound[8] == 2 and Sound_play[8]) // Tippbetrieb ?
    {
      I2SAudio.Stop(&Sound8);
    }
    
    if (not Sound8.Playing and Sound_play[8])
    {
      Sound8.UnLoadWavFile();
      Sound_play[8] = false;
    }    
  }    

  if (Sound_on[0] == true && !Sound_on_Motor_state && config.engine_on_toggle) {
    Sound_on_Motor = !Sound_on_Motor; // Toggle Motor Sound On
    Sound_on_Motor_state = true; 
  }
  else if (Sound_on[0] == false && Sound_on_Motor_state && config.engine_on_toggle) {
    Sound_on_Motor_state = false; 
  }
  else if (!config.engine_on_toggle) {
    Sound_on_Motor = Sound_on[0];
  }

	  // Motor ON OFF
  if (((Sound_on_Motor or Sound_on_web[0]) and not engine_break and not Sound_play[0]) or (throttle > 0 and engine_break and not Sound_play[0]) or (throttle > 0 and Sound_play[0])) //on
  {
	  Sound_play[0] = true;
	  shutdown_timer = millis();
	}  
  if (not Sound_on_Motor and not Sound_on_web[0]) //off
	{
	   Sound_play[0] = false;
	   engine_break = false;
	}
  if ((millis() - shutdown_timer) > (config.shutdowndelay * 1000) && Sound_play[0] && (config.shutdowndelay > 0)) //off über shutdown_timer
  {
	   Sound_play[0] = false;
	   engine_break = true;
  }	  

   switch (engine_State) {       // Motor Schrittkette
    case OFF:  
      if (Sound_play[0])
      {
        Sound_start.Volume = config.Volumen_Sound[0];
        I2SAudio.Play(&Sound_start);
        engine_State = STARTING;
      }
      break;
    case STARTING:
      if (not Sound_start.Playing)
      {  
        throttle = 0;
        last_throttle = 0;
        Sound_loop.RepeatForever = true;
        Sound_loop.Volume = config.Volumen_Sound[0];
        I2SAudio.Play(&Sound_loop);
        engine_State = RUNNING;
      }  
      break;
    case RUNNING:  
      if (not Sound_play[0])
      {
        throttle = 0;
        if (last_throttle == 0)
        {
          Sound_loop.RepeatForever = false;
          I2SAudio.StopAllSounds();
          engine_State = STOPPING;
        }
      }
      break;
    case STOPPING:
      if (not Sound_loop.Playing)
      {
        Sound_loop.RepeatForever = false;
        Sound_shut.Volume = config.Volumen_Sound[0];
        I2SAudio.Play(&Sound_shut);
        engine_State = OFF;
      }  
      break;  
  }

  // Geschwindigkeit Rampe und Sound Anpassung
  throttle = ramp_throttle(throttle);
  Sound_loop.Speed = floatMap(throttle, 0, 100, config.Min_Speed_Sound_0, config.Max_Speed_Sound_0) / 100;
  }
}

//------------------------------------------------------------------------------------------------------------------------

// ======== Intput  =======================================
void Config() {
  // Quelle für Sound Start 1-9 (Throttle; Sound1-8)
  int duration; //Impulslänge für PWM
  for(int x = 0; x <=8; x++)     // Throttle; Sound1-8
  {
    Source_Start_Sound_OK[x] = false; //Reset Ok
    if (config.Source_Start_Sound[x] < 20)  //SBUS L 
    {
      if (BUS_OK) // SBUS OK?
      {
        Source_Start_Sound_OK[x] = true;
        if (channel_output[config.Source_Start_Sound[x]] < 624)
        {
          Sound_on[x] = true;
        }
        else
        {
          Sound_on[x] = false;
        }
      }  
    }
    else if (config.Source_Start_Sound[x] < 40)  //SBUS H 
    {
      if (BUS_OK) // SBUS OK?
      {
        Source_Start_Sound_OK[x] = true;
        if (channel_output[config.Source_Start_Sound[x]-20] > 1424)
        {
          Sound_on[x] = true;
        }
        else
        {
          Sound_on[x] = false;
        }
      }    
    }
    else if (config.Source_Start_Sound[x] < 50)  //PWM Pin L
    {
      switch (config.Source_Start_Sound[x]-40) {
        case 0: attachInterrupt(digitalPinToInterrupt(Input_Pin[0]), ISR_PWM_0, CHANGE); break;
        case 1: attachInterrupt(digitalPinToInterrupt(Input_Pin[1]), ISR_PWM_1, CHANGE); break;
        case 2: attachInterrupt(digitalPinToInterrupt(Input_Pin[2]), ISR_PWM_2, CHANGE); break;
        case 3: attachInterrupt(digitalPinToInterrupt(Input_Pin[3]), ISR_PWM_3, CHANGE); break;
        case 4: attachInterrupt(digitalPinToInterrupt(Input_Pin[4]), ISR_PWM_4, CHANGE); break;
        case 5: attachInterrupt(digitalPinToInterrupt(Input_Pin[5]), ISR_PWM_5, CHANGE); break;
      }        
      //sei();  // Interrupts freigeben
      duration = PWM_pulse_width[config.Source_Start_Sound[x]-40];
      if (duration < 2600) // PWM OK?  
      {
        duration = map(duration, config.PWM_scale_min , config.PWM_scale_max , 0, 100); //PWM scalieren von 0-100
        Source_Start_Sound_OK[x] = true;      
        if (duration < 25)
        {
          Sound_on[x] = true;
        }
        else
        {
          Sound_on[x] = false;
        }
      }
    }  
    else if (config.Source_Start_Sound[x] < 60)  //PWM Pin H
    {
      switch (config.Source_Start_Sound[x]-50) {
        case 0: attachInterrupt(digitalPinToInterrupt(Input_Pin[0]), ISR_PWM_0, CHANGE); break;
        case 1: attachInterrupt(digitalPinToInterrupt(Input_Pin[1]), ISR_PWM_1, CHANGE); break;
        case 2: attachInterrupt(digitalPinToInterrupt(Input_Pin[2]), ISR_PWM_2, CHANGE); break;
        case 3: attachInterrupt(digitalPinToInterrupt(Input_Pin[3]), ISR_PWM_3, CHANGE); break;
        case 4: attachInterrupt(digitalPinToInterrupt(Input_Pin[4]), ISR_PWM_4, CHANGE); break;
        case 5: attachInterrupt(digitalPinToInterrupt(Input_Pin[5]), ISR_PWM_5, CHANGE); break;
      }        
      //sei();  // Interrupts freigeben
      duration = PWM_pulse_width[config.Source_Start_Sound[x]-50];
      if (duration < 2600) // PWM OK?  
      {
        duration = map(duration, config.PWM_scale_min , config.PWM_scale_max , 0, 100); //PWM scalieren von 0-100
        Source_Start_Sound_OK[x] = true;  
        if (duration > 75)
        {
          Sound_on[x] = true;
        }
        else
        {
          Sound_on[x] = false;
        }
      }
    }      
    else if (config.Source_Start_Sound[x] < 70)   //Pin Input
    {
      Source_Start_Sound_OK[x] = true;
      Sound_on[x] = not digitalRead(Input_Pin[config.Source_Start_Sound[x]-60]);
    }
    else if (config.Source_Start_Sound[x] < 80)      //Einkanal
    {
      if (BUS_OK) // SBUS OK?
      {
        Source_Start_Sound_OK[x] = true;
        if (bitRead (einkanal_Data ,(config.Source_Start_Sound[x]-70))) // Abfrage für Ausgang
        {
          Sound_on[x] = true;
        }
        else
        {
          Sound_on[x] = false;
        }
      }  
    } 
    else if (config.Source_Start_Sound[x] < 110)      //Ebenen Umschaltung
    {
      if ((config.Source_Start_Sound[x]-80) == ebenenkanal_Data)
      {
        Sound_on[x] = true;
      }
      else
      {
        Sound_on[x] = false;
      }
    }
        else if (config.Source_Start_Sound[x] < 201)      //Dauer an
    {
      Sound_on[x] = true;
    }
    else // Nichts
    {
      Sound_on[x] = false;
    }    
  }
  // Quelle Speed Sound 1 (throttle)
  
  if (config.Source_Speed_Sound_0 < 20)  //SBUS Input 
  {
    throttle = map(channel_output[config.Source_Speed_Sound_0], 82, 1900, 0, 100);
  }
  else //PWM
  {
    switch (config.Source_Speed_Sound_0-20) {
      case 0: attachInterrupt(digitalPinToInterrupt(Input_Pin[0]), ISR_PWM_0, CHANGE); break;
      case 1: attachInterrupt(digitalPinToInterrupt(Input_Pin[1]), ISR_PWM_1, CHANGE); break;
      case 2: attachInterrupt(digitalPinToInterrupt(Input_Pin[2]), ISR_PWM_2, CHANGE); break;
      case 3: attachInterrupt(digitalPinToInterrupt(Input_Pin[3]), ISR_PWM_3, CHANGE); break;
      case 4: attachInterrupt(digitalPinToInterrupt(Input_Pin[4]), ISR_PWM_4, CHANGE); break;
      case 5: attachInterrupt(digitalPinToInterrupt(Input_Pin[5]), ISR_PWM_5, CHANGE); break;
    }        
    //sei();  // enable interrupts
    duration = PWM_pulse_width[config.Source_Speed_Sound_0-20];
    throttle = map(duration, config.PWM_scale_min , config.PWM_scale_max , 0, 100); //PWM scalieren von 0-100
  }

  if (config.throttle_mode) //vor und Zurück
  {
    if (throttle > 50 + config.throttle_dead_band) // Totband
    {
      throttle = map(throttle, 50 + config.throttle_dead_band, 100, 0, 100);
    }
    else if (throttle < 50 - config.throttle_dead_band) // Totband
    {
      throttle = map(throttle, 50 - config.throttle_dead_band, 0, 0, 100);
    }
    else
    {
      throttle = 0;
    }
  }  
  else
  {
    if (throttle > config.throttle_dead_band) // Totband
    {
      throttle = map(throttle, config.throttle_dead_band, 100, 0, 100);
    }
    else
    {
      throttle = 0;
    }	 
  }  
  //Serial.println(" ");
}

// ======== Float Map  =======================================
float floatMap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// ======== SD Card Init  =======================================
void SDCardInit()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH); // SD-Karte auswählen, muss GPIO 5 verwenden (ESP32 )
  if (!SD.begin(SD_CS))
  {
    Serial.println("Fehler beim Lesen der SD-Karte!");
    //while (true)
      ; // Programm Ende
  }
}

// ======== LoadFiles  =======================================
void LoadFiles()
{
  Sound_loop.LoadWavFile();
  Sound_shut.LoadWavFile();
  Sound_start.LoadWavFile();
}

// ======== Rampe Throttle  =======================================
// Rampe need (currentTime as millis(); unsigned long previousTime_ramp as timesaver; float last_throttle as throttlesaver; float throttle_ramp as ramp per Second)
float ramp_throttle(float throttle_new)
{
     unsigned long RampTime = currentTime - previousTime_ramp;
     previousTime_ramp = currentTime;
   //  Serial.print("RampTime ms : ");
   //  Serial.println(RampTime);
     float throttle_ramp_f = config.throttle_ramp;
     float throttle_ramp_now = (throttle_ramp_f / 1000) * RampTime;
     if (throttle_new > last_throttle)          //throttel go up
    {
        if (last_throttle + throttle_ramp_now < throttle)
        {
          throttle_new = last_throttle + throttle_ramp_now;
        }
        else
        {
          throttle_new = throttle_new;
        }
    }
    else                                   //throttel go down
    {
        if (last_throttle - throttle_ramp_now > throttle)
        {
          throttle_new = last_throttle - throttle_ramp_now;
        }
        else
        {
          throttle_new = throttle_new;
        }
    }
    last_throttle = throttle_new;
    return throttle_new;
}

// ======== Einkanal  SBUS =======================================
void einkanalFunction(uint16_t channel) {
    einkanal_Data = channel;

  if (config.Einkanal_mode == 0) 
  { 

    if (config.Einkanal_RC_System == 0) {
      einkanal_Data = einkanal_Data  / 8;
      einkanal_Data = einkanal_Data; 
    }

    if (config.Einkanal_RC_System == 1) {    
      if (einkanal_Data < 206) { einkanal_Data = 206;}
     if (einkanal_Data > 1837) { einkanal_Data= 1837;}
     einkanal_Data = einkanal_Data - 206 ;
     einkanal_Data = (einkanal_Data * 10);
     einkanal_Data = einkanal_Data + 20;  
     einkanal_Data = einkanal_Data  / 8;
     einkanal_Data = einkanal_Data  / 8;
    } 
  
    if (config.Einkanal_RC_System == 2) {
      long einkanal_Data2;
      float einkanal_Data3;
      einkanal_Data3 = einkanal_Data - 172;
      einkanal_Data3 = einkanal_Data3 + 1.5;
      einkanal_Data3 = einkanal_Data3 * 0.155677655677655;
      einkanal_Data2 = einkanal_Data3;
      einkanal_Data = einkanal_Data2;
    }
  }

  if (config.Einkanal_mode > 9) 
  {
    uint16_t n;
    uint8_t  v;
    if (config.Einkanal_RC_System == 0) //FrSky
    {
      n = (channel >= 172) ? (channel - 172 + 1)  : 0;
      v = (n >> 4);
    }
    if (config.Einkanal_RC_System == 1) //FlySky
    {
      n = (channel >= 220) ? (channel - 220) : 0;
      uint16_t n2 = n + (n >> 6);
      v = (n2 >> 4);
    }
    if (config.Einkanal_RC_System == 2) //ELRS
    {
      n = (channel >= 172) ? (channel - 172)  : 0;
      v = (n >> 4);
    }    
    if (config.Einkanal_RC_System == 3) //Hott
    {
      n = (channel >= 205) ? (channel - 205)  : 0;
      v = (n >> 4);
    }    
    //uint16_t n = (channel >= 220) ? (channel - 220) : 0;
    //uint8_t  v = (n >> 4);
    uint8_t address = (v >> 4) & 0b11;
    uint8_t sw      = (v >> 1) & 0b111;
    uint8_t state   = v & 0b1;

    if (address == (config.Einkanal_mode - 10))  // Adresse ?
    {
      bitWrite(einkanal_SpeicherWM, sw, state);
    }

    einkanal_Data = einkanal_SpeicherWM;  // Verschiebe zum Output
  }
 
}

// ======== Einkanal  CRSF =======================================
void einkanalFunctionCRSF() {
  // Lese den CRSF-Befehl aus dem internen Buffer:
  // Byte 5 = Realm (z. B. 0xA1 für MultiSwitch)
  // Byte 6 = Command (z. B. 0x01, 0x07, 0x09, 0x02)
  uint8_t WMcode  = crsf.get_crfs_buffer(5);
  uint8_t command = crsf.get_crfs_buffer(6);
        
  // Prüfen, ob der Befehl zum Realm 0xA1 (Multiswitch) gehört
  if (WMcode == Multiswitch) {
    switch (command) {

        // ===== Fall 0x07: MWset4 =====
        // Setzt 8 Schalter mit jeweils 4 Zuständen
        // Payload: 1 Byte Adresse + 2 Byte Zustand
        case MWset4: {
            uint8_t address = crsf.get_crfs_buffer(7); // Switch-Adresse auslesen
            // Nur reagieren, wenn die Adresse mit der Moduladresse übereinstimmt
            if (address == config.modul_adress) {
              // Zustand aus zwei Bytes zusammensetzen (High + Low)
              uint16_t state = (crsf.get_crfs_buffer(8) << 8) + crsf.get_crfs_buffer(9);
              // Ausgabe entsprechend dem komprimierten Zustand setzen (PWM)
              einkanal_Data = (compressSwitches(state));
            }  
            break; 
        }
                    
        // ===== Fall 0x09: MWset4m =====
        // Setzt mehrere 4-Zustands-Schalter mit unterschiedlichen Adressen
        // Payload: <count> × [ <address> <stateH> <stateL> ]
        case MWset4m: {
            int8_t count = crsf.get_crfs_buffer(7); // Anzahl der Gruppen
            uint8_t  address[count];                // Array für Adressen
            uint16_t state[count];                  // Array für Zustände

            // Schleife über alle Gruppen
            for(uint8_t i = 0; i < count; ++i) {
              // Adresse der Gruppe auslesen
              address[i] = crsf.get_crfs_buffer(8 + (3 * i));
              // Nur reagieren, wenn Adresse mit Moduladresse übereinstimmt
              if (address[i] == config.modul_adress) {
                // Zustand aus High- und Low-Byte zusammensetzen
                state[i] = (crsf.get_crfs_buffer(9 + (3 * i)) << 8) + crsf.get_crfs_buffer(10 + (3 * i));
                // Ausgabe entsprechend dem komprimierten Zustand setzen (PWM)
                einkanal_Data = (compressSwitches(state[i]));
              }
            }
            break; 
        }
    }
  }  
}

// Funktion zur Komprimierung von 16-Bit Switch-Zuständen auf ein 8-Bit Ergebnis
// Jeder Schalter hat 4 mögliche Zustände (2 Bits), wir extrahieren jeweils nur das "aktiv"-Bit.
// Ergebnis: 8 Schalter → 8 Bits in einem Byte.
uint8_t compressSwitches(uint16_t state) {
    uint8_t result = 0;
    result |= ((state >> 0) & 0x1) << 0;    // Switch 1: Bits 0–1 → wir nehmen Bit 0 und setzen es auf Position 0 im Ergebnis
    result |= ((state >> 2) & 0x1) << 1;    // Switch 2: Bits 2–3 → wir nehmen Bit 2 und setzen es auf Position 1
    result |= ((state >> 4) & 0x1) << 2;    // Switch 3: Bits 4–5 → wir nehmen Bit 4 und setzen es auf Position 2
    result |= ((state >> 6) & 0x1) << 3;    // Switch 4: Bits 6–7 → wir nehmen Bit 6 und setzen es auf Position 3    
    result |= ((state >> 8) & 0x1) << 4;    // Switch 5: Bits 8–9 → wir nehmen Bit 8 und setzen es auf Position 4   
    result |= ((state >> 10) & 0x1) << 5;   // Switch 6: Bits 10–11 → wir nehmen Bit 10 und setzen es auf Position 5  
    result |= ((state >> 12) & 0x1) << 6;   // Switch 7: Bits 12–13 → wir nehmen Bit 12 und setzen es auf Position 6   
    result |= ((state >> 14) & 0x1) << 7;   // Switch 8: Bits 14–15 → wir nehmen Bit 14 und setzen es auf Position 7
    return result;   // Ergebnis zurückgeben: jedes Bit repräsentiert einen Schalter (0 = aus, 1 = an)
}

// ======== EbenenUmschaltung  =======================================
void ebenenFunction() {
  // Kanalwert lesen
  Source_Ebenen_Um_Kanal_wert = 1;
  Source_Ebenen_Kanal_wert = 0;
  if (config.Source_Ebenen_Um_Kanal < 20)  //SBUS Input 
  {
    Source_Ebenen_Um_Kanal_wert = map(channel_output[config.Source_Ebenen_Um_Kanal], 207, 1833, 0, 100);
  }
  else if (config.Source_Ebenen_Um_Kanal < 30) //PWM
  {
    switch (config.Source_Ebenen_Um_Kanal-20) {
      case 0: attachInterrupt(digitalPinToInterrupt(Input_Pin[0]), ISR_PWM_0, CHANGE); break;
      case 1: attachInterrupt(digitalPinToInterrupt(Input_Pin[1]), ISR_PWM_1, CHANGE); break;
      case 2: attachInterrupt(digitalPinToInterrupt(Input_Pin[2]), ISR_PWM_2, CHANGE); break;
      case 3: attachInterrupt(digitalPinToInterrupt(Input_Pin[3]), ISR_PWM_3, CHANGE); break;
      case 4: attachInterrupt(digitalPinToInterrupt(Input_Pin[4]), ISR_PWM_4, CHANGE); break;
      case 5: attachInterrupt(digitalPinToInterrupt(Input_Pin[5]), ISR_PWM_5, CHANGE); break;
    }        
    //sei();  // enable interrupts
    Source_Ebenen_Um_Kanal_wert = map(PWM_pulse_width[config.Source_Ebenen_Um_Kanal], 938, 2063, 0, 100);
  }

  if (config.Source_Ebenen_Kanal < 20)  //SBUS Input 
  {
    Source_Ebenen_Kanal_wert = map(channel_output[config.Source_Ebenen_Kanal], 207, 1833, 0, 100);
  }
  else if (config.Source_Ebenen_Kanal < 30) //PWM
  {
    switch (config.Source_Ebenen_Kanal-20) {
      case 0: attachInterrupt(digitalPinToInterrupt(Input_Pin[0]), ISR_PWM_0, CHANGE); break;
      case 1: attachInterrupt(digitalPinToInterrupt(Input_Pin[1]), ISR_PWM_1, CHANGE); break;
      case 2: attachInterrupt(digitalPinToInterrupt(Input_Pin[2]), ISR_PWM_2, CHANGE); break;
      case 3: attachInterrupt(digitalPinToInterrupt(Input_Pin[3]), ISR_PWM_3, CHANGE); break;
      case 4: attachInterrupt(digitalPinToInterrupt(Input_Pin[4]), ISR_PWM_4, CHANGE); break;
      case 5: attachInterrupt(digitalPinToInterrupt(Input_Pin[5]), ISR_PWM_5, CHANGE); break;
    }        
    //sei();  // enable interrupts
    Source_Ebenen_Kanal_wert = map(PWM_pulse_width[config.Source_Ebenen_Kanal], 938, 2063, 0, 100);
  }
    
  Source_Ebenen_Um_Kanal_wert = (Source_Ebenen_Um_Kanal_wert) / 33;     // Wert 0-2
  //Source_Ebenen_Kanal_wert = ((Source_Ebenen_Kanal_wert * 10) / 125);   // Wert 0-8

  if (Source_Ebenen_Kanal_wert < 6)  // Wert 0
  {
    ebenenkanal_Data = (Source_Ebenen_Um_Kanal_wert * 8) + 0;
  }
  else if (Source_Ebenen_Kanal_wert < 19)  // Wert 1
  {
    ebenenkanal_Data = (Source_Ebenen_Um_Kanal_wert * 8) + 1;
  }
  else if (Source_Ebenen_Kanal_wert < 31)  // Wert 2
  {
    ebenenkanal_Data = (Source_Ebenen_Um_Kanal_wert * 8) + 2;
  }
  else if (Source_Ebenen_Kanal_wert < 44)  // Wert 3
  {
    ebenenkanal_Data = (Source_Ebenen_Um_Kanal_wert * 8) + 3;
  }
  else if (Source_Ebenen_Kanal_wert < 56)  // Wert 4
  {
    ebenenkanal_Data = 99;
  }
  else if (Source_Ebenen_Kanal_wert < 69)  // Wert 5
  {
    ebenenkanal_Data = (Source_Ebenen_Um_Kanal_wert * 8) + 4;
  }
  else if (Source_Ebenen_Kanal_wert < 81)  // Wert 6
  {
    ebenenkanal_Data = (Source_Ebenen_Um_Kanal_wert * 8) + 5;
  }
  else if (Source_Ebenen_Kanal_wert < 94)  // Wert 7
  {
    ebenenkanal_Data = (Source_Ebenen_Um_Kanal_wert * 8) + 6;
  }
  else if (Source_Ebenen_Kanal_wert < 106)  // Wert 8
  {
    ebenenkanal_Data = (Source_Ebenen_Um_Kanal_wert * 8) + 7;
  }
      // Serial.print(" Ebenen UM Wert: ");
      // Serial.print(Source_Ebenen_Um_Kanal_wert);
      // Serial.print(" Ebenen Wert: ");
      // Serial.print(Source_Ebenen_Kanal_wert);
      // Serial.print(" Ebenen Data: ");
      // Serial.print(ebenenkanal_Data);
}

