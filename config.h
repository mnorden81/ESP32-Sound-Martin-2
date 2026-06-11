#ifndef CONFIG_H
#define CONFIG_H

#include <EEPROM.h>

// Struktur für alle konfigurierbaren Parameter
struct ConfigData {
  uint16_t version;               // ← Versionskennung
  int Source_Start_Sound[9];      // Source_Start_Sound 1-9
  int Volumen_Sound[9];           // Lautstärke Sound 1-9
  int Mode_Sound[9];              // Mode Sound 2 Normal = 0 Loop = 1 Tippbetrieb = 3
  int Source_Speed_Sound_0;       // Source_Speed_Sound 1
  int throttle_mode;
  int Min_Speed_Sound_0;          // untere Geschwindigkeit Motor in % (100% ist 1fache Wiedergabegeschwindigkeit
  int Max_Speed_Sound_0;          // obere Geschwindigkeit Motor in % (150% ist 1,5fache Wiedergabegeschwindigkeit
  int Einkanal_Channel;           // Einkanal SBUSKanal
  int Einkanal_mode;              // Einkanal Mode
  int Einkanal_RC_System;         // Einkanal RC_System Elrs Flysky ...
  int modul_adress;               // Adresse des Moduls für WM-Modus über CRSF
  int Source_Ebenen_Um_Kanal;     // EbenenUmschaltung
  int Source_Ebenen_Kanal;        // EbenenUmschaltung
  int throttle_ramp;
  int throttle_dead_band;
  int shutdowndelay;              // Number of seconds to wait before shutdown sound is played.
  int engine_on_toggle;
  int Hardware_Config;            // Hardware Config
  int PWM_scale_min;              // PWM min Wert Scalierung
  int PWM_scale_max;              // PWM max Wert Scalierung
  char WiFi_SSID[32];             // WiFi SSID (max 31 chars + null terminator)
  char WiFi_Password[64];         // WiFi Passwort (max 63 chars + null terminator)
};

// Globale Instanz der Konfiguration
extern ConfigData config;

// Funktionen zum Laden und Speichern
void loadConfig();
void saveConfig();
void Reset_all();
void set_sbus();
void set_pwm();
void set_pin(); 

#endif
