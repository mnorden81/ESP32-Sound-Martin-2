#include "config.h"

// Globale Instanz
ConfigData config;

 // Software Version
extern uint16_t Version; 

// ======== Werkseinstellungen  =======================================
void Reset_all() {
  config.Source_Start_Sound[0] = 999;
  config.Source_Start_Sound[1] = 999;
  config.Source_Start_Sound[2] = 999;
  config.Source_Start_Sound[3] = 999;
  config.Source_Start_Sound[4] = 999;
  config.Source_Start_Sound[5] = 999;
  config.Source_Start_Sound[6] = 999;
  config.Source_Start_Sound[7] = 999;
  config.Source_Start_Sound[8] = 999;

  config.Volumen_Sound[0] = 100;
  config.Volumen_Sound[1] = 100;
  config.Volumen_Sound[2] = 100;
  config.Volumen_Sound[3] = 100;
  config.Volumen_Sound[4] = 100;
  config.Volumen_Sound[5] = 100;
  config.Volumen_Sound[6] = 100;
  config.Volumen_Sound[7] = 100;
  config.Volumen_Sound[8] = 100;

  config.Mode_Sound[1] = 0;
  config.Mode_Sound[2] = 0;
  config.Mode_Sound[3] = 0;
  config.Mode_Sound[4] = 0;
  config.Mode_Sound[5] = 0;
  config.Mode_Sound[6] = 0;
  config.Mode_Sound[7] = 0;
  config.Mode_Sound[8] = 0;

  config.Source_Speed_Sound_0 = 999;
  config.throttle_mode = 0;
  config.Min_Speed_Sound_0 = 100;
  config.Max_Speed_Sound_0 = 300;

  config.Einkanal_Channel = 999;
  config.Einkanal_mode = 0;
  config.Einkanal_RC_System = 0;
  config.modul_adress = 0;

  config.Source_Ebenen_Um_Kanal = 999;
  config.Source_Ebenen_Kanal = 999;

  config.throttle_ramp = 20;
  config.throttle_dead_band = 10;

  config.shutdowndelay = 0;
  config.engine_on_toggle = 0;

  config.PWM_scale_min = 1000;
  config.PWM_scale_max = 2000;  
}
// ======== Voreinstellungen SBUS  =======================================
void set_sbus() {
  config.Source_Start_Sound[0] = 10;
  config.Source_Start_Sound[1] = 11;
  config.Source_Start_Sound[2] = 12;
  config.Source_Start_Sound[3] = 13;
  config.Source_Start_Sound[4] = 14;
  config.Source_Start_Sound[5] = 999;
  config.Source_Start_Sound[6] = 999;
  config.Source_Start_Sound[7] = 999;
  config.Source_Start_Sound[8] = 999;

  config.Source_Speed_Sound_0 = 1;
  config.Min_Speed_Sound_0 = 100;
  config.Max_Speed_Sound_0 = 300;

  config.Volumen_Sound[0] = 100;
  config.Volumen_Sound[1] = 100;
  config.Volumen_Sound[2] = 100;
  config.Volumen_Sound[3] = 100;
  config.Volumen_Sound[4] = 100;
  config.Volumen_Sound[5] = 100;
  config.Volumen_Sound[6] = 100;
  config.Volumen_Sound[7] = 100;
  config.Volumen_Sound[8] = 100;

  config.Mode_Sound[1] = 0;
  config.Mode_Sound[2] = 0;
  config.Mode_Sound[3] = 0;
  config.Mode_Sound[4] = 0;
  config.Mode_Sound[5] = 0;
  config.Mode_Sound[6] = 0;
  config.Mode_Sound[7] = 0;
  config.Mode_Sound[8] = 0;

  config.shutdowndelay = 0;
  config.engine_on_toggle = 0;

  config.Einkanal_Channel = 999;
  config.Source_Ebenen_Um_Kanal = 999;
  config.Source_Ebenen_Kanal = 999;

  config.throttle_ramp = 20;
  config.throttle_dead_band = 10; 

  config.PWM_scale_min = 1000;
  config.PWM_scale_max = 2000;
}

// ======== Voreinstellungen PWM  =======================================
void set_pwm() {
  config.Source_Start_Sound[0] = 41;
  config.Source_Start_Sound[1] = 42;
  config.Source_Start_Sound[2] = 43;
  config.Source_Start_Sound[3] = 44;
  config.Source_Start_Sound[4] = 45;
  config.Source_Start_Sound[5] = 999;
  config.Source_Start_Sound[6] = 999;
  config.Source_Start_Sound[7] = 999;
  config.Source_Start_Sound[8] = 999;

  config.Source_Speed_Sound_0 = 20;
  config.Min_Speed_Sound_0 = 100;
  config.Max_Speed_Sound_0 = 300;

  config.Volumen_Sound[0] = 100;
  config.Volumen_Sound[1] = 100;
  config.Volumen_Sound[2] = 100;
  config.Volumen_Sound[3] = 100;
  config.Volumen_Sound[4] = 100;
  config.Volumen_Sound[5] = 100;
  config.Volumen_Sound[6] = 100;
  config.Volumen_Sound[7] = 100;
  config.Volumen_Sound[8] = 100;

  config.Mode_Sound[1] = 0;
  config.Mode_Sound[2] = 0;
  config.Mode_Sound[3] = 0;
  config.Mode_Sound[4] = 0;
  config.Mode_Sound[5] = 0;
  config.Mode_Sound[6] = 0;
  config.Mode_Sound[7] = 0;
  config.Mode_Sound[8] = 0;

  config.shutdowndelay = 0;
  config.engine_on_toggle = 0;

  config.Einkanal_Channel = 999;
  config.Source_Ebenen_Um_Kanal = 999;
  config.Source_Ebenen_Kanal = 999;

  config.throttle_ramp = 20;
  config.throttle_dead_band = 10;  

  config.PWM_scale_min = 1000;
  config.PWM_scale_max = 2000;
}

// ======== Voreinstellungen Pin  =======================================
void set_pin() {
  config.Source_Start_Sound[0] = 61;
  config.Source_Start_Sound[1] = 62;
  config.Source_Start_Sound[2] = 63;
  config.Source_Start_Sound[3] = 64;
  config.Source_Start_Sound[5] = 999;
  config.Source_Start_Sound[6] = 999;
  config.Source_Start_Sound[7] = 999;
  config.Source_Start_Sound[8] = 999;

  config.Source_Speed_Sound_0 = 20;
  config.Min_Speed_Sound_0 = 100;
  config.Max_Speed_Sound_0 = 300;
                     
  config.Volumen_Sound[0] = 100;
  config.Volumen_Sound[1] = 100;
  config.Volumen_Sound[2] = 100;
  config.Volumen_Sound[3] = 100;
  config.Volumen_Sound[4] = 100;
  config.Volumen_Sound[5] = 100;
  config.Volumen_Sound[6] = 100;
  config.Volumen_Sound[7] = 100;
  config.Volumen_Sound[8] = 100;

  config.Mode_Sound[1] = 0;
  config.Mode_Sound[2] = 0;
  config.Mode_Sound[3] = 0;
  config.Mode_Sound[4] = 0;
  config.Mode_Sound[5] = 0;
  config.Mode_Sound[6] = 0;
  config.Mode_Sound[7] = 0;
  config.Mode_Sound[8] = 0;

  config.shutdowndelay = 0;
  config.engine_on_toggle = 0;

  config.Einkanal_Channel = 999;
  config.Source_Ebenen_Um_Kanal = 999;
  config.Source_Ebenen_Kanal = 999;

  config.throttle_ramp = 20;
  config.throttle_dead_band = 10;  

  config.PWM_scale_min = 1000;
  config.PWM_scale_max = 2000;       
}

void loadConfig() {
  EEPROM.begin(sizeof(ConfigData));
  EEPROM.get(0, config);

  if (config.version != Version) {
    Serial.println("EEPROM-Version passt nicht – Werkseinstellungen werden geladen.");
    Reset_all();
  }

  EEPROM.end();
}

void saveConfig() {
  config.version = Version;
  EEPROM.begin(sizeof(ConfigData));
  EEPROM.put(0, config);
  EEPROM.commit();
  EEPROM.end();
}