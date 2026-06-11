#include "WebServerManager.h"
#include "config.h"
#include <Arduino.h>
#include "sbus.h"
#include "XT_I2S_Audio.h"

// Webserver auf Port 80
WiFiServer WebServerManager::server = WiFiServer(80);

// Speicher HTTP request
String WebServerManager::header = "";

// Für HTTP GET value
String WebServerManager::valueString = String(5);
int WebServerManager::pos1 = 0;
int WebServerManager::pos2 = 0;
int WebServerManager::Menu = 0;

//Timer Zeiten
unsigned long WebServerManager::previousTime = 0;
unsigned long WebServerManager::currentTime = 0;
const long WebServerManager::timeoutTime = 2000; // wie in deinem Sketch

// aus der ESP32_RC_Sound.ino externe
extern bool Sound_on_web[9];
extern uint16_t channel_output[16];
extern volatile unsigned int PWM_pulse_width[6];
extern XT_Wav_Class Sound_loop;
extern XT_Wav_Class Sound_shut;
extern XT_Wav_Class Sound_start;
extern XT_Wav_Class Sound1;
extern XT_Wav_Class Sound2;
extern XT_Wav_Class Sound3;
extern XT_Wav_Class Sound4;
extern XT_Wav_Class Sound5;
extern XT_Wav_Class Sound6;
extern XT_Wav_Class Sound7;
extern XT_Wav_Class Sound8;
extern char versionString[6];       // Zeichenkette zur Anzeige der Version

void WebServerManager::begin(const char* apSsid, const char* apPassword) {
  // AP starten
  WiFi.softAP(apSsid, apPassword);
  IPAddress Ip(192, 168, 1, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  Serial.print("AP gestartet, IP: ");
  Serial.println(WiFi.softAPIP());
  // Server starten
  server.begin();
}

// ======== Webseite  =======================================  
void WebServerManager::Webpage()
{  
  WiFiClient client = server.available();   // Listen for incoming clients
  
  if (client) {                             // If a new client connects,
    currentTime = millis();                 // kann weg ------------------------------------------------------------------------
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP-Header fangen immer mit einem Response-Code an (z.B. HTTP/1.1 200 OK)
            // gefolgt vom Content-Type damit der Client weiss was folgt, gefolgt von einer Leerzeile:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Webseiten Eingaben abfragen

            if (header.indexOf("GET /Sound/on") >= 0) 
            {
               Sound_on_web[Menu] = true;
            }

            if (header.indexOf("GET /save") >= 0) {  // Abfrage Button Save
              saveConfig();
            } 

            if (header.indexOf("GET /reset") >= 0) {  // Abfrage Button reset
              Reset_all();
            } 

            if (header.indexOf("GET /setsbus") >= 0) {  // Abfrage Button Voreinstellungen SBUS
              set_sbus();
            } 

            if (header.indexOf("GET /setpwm") >= 0) {  // Abfrage Button Voreinstellungen PWM
              set_pwm();
            } 

            if (header.indexOf("GET /setpin") >= 0) {  // Abfrage Button Voreinstellungen PIN
              set_pin();
            }                                     

            if (header.indexOf("GET /next") >= 0) {  // Abfrage Button Next
              Menu ++;
            } 

            if (header.indexOf("GET /back") >= 0) {  // Abfrage Button Back
              Menu --;
            }            

            if(header.indexOf("GET /?VolumenSound=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Volumen_Sound[Menu] = (valueString.toInt());
            }

            if(header.indexOf("GET /?Drehzahlmin=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Min_Speed_Sound_0 = (valueString.toInt());
            }

            if(header.indexOf("GET /?Drehzahlmax=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Max_Speed_Sound_0 = (valueString.toInt());
            }

            if(header.indexOf("GET /?shutdowndelay=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.shutdowndelay = (valueString.toInt());
            }            

            if(header.indexOf("GET /?MotorMODE=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.throttle_mode = (valueString.toInt());
            }

            if(header.indexOf("GET /?MotorOnToggle=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.engine_on_toggle = (valueString.toInt());
            }

            if(header.indexOf("GET /?SoundON=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Source_Start_Sound[Menu] = (valueString.toInt());
            }

            if(header.indexOf("GET /?SoundSPEED=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Source_Speed_Sound_0 = (valueString.toInt());
            }

            if(header.indexOf("GET /?RCSytem=")>=0) { // Abfrage RC-Sytem
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Einkanal_RC_System = (valueString.toInt());
            }

            if(header.indexOf("GET /?SbuschannelEinkanal=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Einkanal_Channel = (valueString.toInt());
            }

            if(header.indexOf("GET /?MODULADRESSE=")>=0) { // Abfrage CRSF Adresse
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.modul_adress = (valueString.toInt());
            }            

            if(header.indexOf("GET /?EinkanalMode=")>=0) { // Abfrage Einkanal-Mode
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Einkanal_mode = (valueString.toInt());
            }  

            if(header.indexOf("GET /?EbenenUmschaltungKanal=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Source_Ebenen_Um_Kanal = (valueString.toInt());
            }    
            if(header.indexOf("GET /?EbenenKanal=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Source_Ebenen_Kanal = (valueString.toInt());
            }       

            if(header.indexOf("GET /?SoundMODE=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Mode_Sound[Menu] = (valueString.toInt());
            } 
            
            if(header.indexOf("GET /?ThrottleRamp=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.throttle_ramp = (valueString.toInt());
            } 

            if(header.indexOf("GET /?ThrottleDeadBand=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.throttle_dead_band = (valueString.toInt());
            }            

            if(header.indexOf("GET /?HardwareConfig=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              config.Hardware_Config = (valueString.toInt());
            }    

            if (header.indexOf("GET /setPWM") >= 0) {
  
              // --- PWM Min auslesen ---
              int pwmMinIndex = header.indexOf("pwmMin=");
              if (pwmMinIndex > 0) {
                int pwmMinEnd = header.indexOf('&', pwmMinIndex);
                if (pwmMinEnd < 0) pwmMinEnd = header.length();
                String pwmMinStr = header.substring(pwmMinIndex + 7, pwmMinEnd);
                config.PWM_scale_min = pwmMinStr.toInt();
              }

              // --- PWM Max auslesen ---
              int pwmMaxIndex = header.indexOf("pwmMax=");
              if (pwmMaxIndex > 0) {
                int pwmMaxEnd = header.indexOf(' ', pwmMaxIndex);
                if (pwmMaxEnd < 0) pwmMaxEnd = header.length();
                String pwmMaxStr = header.substring(pwmMaxIndex + 7, pwmMaxEnd);
                config.PWM_scale_max = pwmMaxStr.toInt();
              }
            }
            
            //Werte begrenzen

            Menu = constrain(Menu, 0, 10); // Begrenzt den Bereich Menu auf 1 bis 10.
            
            //HTML Seite angezeigen:
            client.println("<!DOCTYPE html><html>");
            //client.println("<meta http-equiv='refresh' content='5'>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS zum Stylen der Ein/Aus-Schaltflächen
            // Fühlen Sie sich frei, die Attribute für Hintergrundfarbe und Schriftgröße nach Ihren Wünschen zu ändern
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { border: 4px solid black; color: white; padding: 10px 40px; width: 100%; border-radius: 10px;");
            client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");
            client.println(".slider { -webkit-appearance: none; width: 80%; height: 30px; background: #d3d3d3; outline: none; opacity: 0.7; -webkit-transition: .2s; transition: opacity .2s; }");
            client.println(".slider::-moz-range-thumb { width: 30px; height: 30px; background: #04AA6D; cursor: pointer; }");
            client.println(".buttonA {outline: none; cursor: pointer; padding: 14px; margin: 10px; width: 90%; font-family: Verdana, Helvetica, sans-serif; font-size: 20px; background-color: #2222; color: #111; border: 4px solid black; border-radius: 10px; text-align: center;}");
            client.println(".text1 {font-family: Verdana, Helvetica, sans-serif; font-size: 25px; color: #111; text-align: center; margin-top: 0px; margin-bottom: 0px; }");
            client.println(".text2 {font-family: Verdana, Helvetica, sans-serif; font-size: 20px; color: #111; text-align: center; margin-top: 0px; margin-bottom: 0px; }");
            client.println(".text3 {font-family: Verdana, Helvetica, sans-serif; font-size: 15px; color: #111; text-align: center; margin-top: 0px; margin-bottom: 0px; }");
            client.println(".text4 {font-family: Verdana, Helvetica, sans-serif; font-size: 10px; color: #111; text-align: center; margin-top: 0px; margin-bottom: 0px; }");

            client.println(".center {text-align: center; }");
            client.println(".left {text-align: left; }");
            client.println(".right {text-align: right; }");
            
          
            client.println(".button1 {background-color: #4CAF50;}");
            client.println(".button2 {background-color: #ff0000;}");
            client.println(".button3 {background-color: #4CAF50; float: left; width: 45%;}");
            client.println(".button4 {background-color: #ff0000; float: right; width: 45%;}");
            client.println(".button5 {background-color: #ffe453; float: left; width: 45%;}");
            client.println(".button6 {background-color: #ffe453; float: center; width: 45%;}");
            client.println(".button7 {background-color: #ffe453; float: right; width: 45%;}");
            client.println(".disabled {opacity: 0.4; cursor: not-alloed;}");
            client.println(".textbox {font-size: 25px; text-align: center;}");
            client.println("</style></head>");    
                     
            // Webseiten-Überschrift
            client.println("<body>");
            client.println("<p class=\"text1\" ><b>ESP32 RC-Sound</b></p>"); 
            client.println("<p class=\"text3\" >Version : ");
            client.println(versionString);
            client.println("</p>");

            switch (Menu) {
              case 0:

            client.println("<p class=\"text2\" ><b>Motor Einstellung</b></p>"); 
            
            client.println("<p><a href=\"/back\"><button class=\"button button3\">Back</button></a>");
            client.println("<a href=\"/next\"><button class=\"button button4\">Next</button></a></p>");

            client.println("<br />");
            client.println("<br />");
            client.println("<br />");
            client.println("<br />");            
        
            client.println("<p class=\"text2\" >Motor Mode</p>");  

            // Motor Mode  
            client.println("<p><select id=\"MotorMODE\" class=\"buttonA\" onchange=\"setmotormode()\">");
      	    client.println("<option value=\"0\">Eine Richtung</option>");
            client.println("<option value=\"1\">Zwei Richtungen</option>");
            client.println("</select><br></p>");

            client.println("<script> function setmotormode() { ");
            client.println("var sel = document.getElementById(\"MotorMODE\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?MotorMODE=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.throttle_mode, DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"MotorMODE\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");                 
        
            client.println("<p class=\"text2\" >Motor EIN Modus</p>");  

            // Motor Toggle On  
            client.println("<p><select id=\"MotorOnToggle\" class=\"buttonA\" onchange=\"setmotorontoggle()\">");
      	    client.println("<option value=\"0\">Normal</option>");
            client.println("<option value=\"1\">Toggle</option>");
            client.println("</select><br></p>");

            client.println("<script> function setmotorontoggle() { ");
            client.println("var sel = document.getElementById(\"MotorOnToggle\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?MotorOnToggle=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.engine_on_toggle, DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"MotorOnToggle\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");                      


            client.println("<p class=\"text2\" >Quelle Einschalten Motor</p>");

            // Quelle Einschalten Motor
	          client.println("<p><select id=\"SoundON\" class=\"buttonA\" onchange=\"setsoundon()\">");
            client.println("<optgroup label=\"BUS Kanal Low\">");
      	    client.println("<option value=\"0\">BUS Kanal Low 01</option>");
            client.println("<option value=\"1\">BUS Kanal Low 02</option>");
            client.println("<option value=\"2\">BUS Kanal Low 03</option>");
            client.println("<option value=\"3\">BUS Kanal Low 04</option>");
            client.println("<option value=\"4\">BUS Kanal Low 05</option>");
            client.println("<option value=\"5\">BUS Kanal Low 06</option>");
            client.println("<option value=\"6\">BUS Kanal Low 07</option>");
            client.println("<option value=\"7\">BUS Kanal Low 08</option>");
            client.println("<option value=\"8\">BUS Kanal Low 09</option>");
            client.println("<option value=\"9\">BUS Kanal Low 10</option>");
            client.println("<option value=\"10\">BUS Kanal Low 11</option>");
            client.println("<option value=\"11\">BUS Kanal Low 12</option>");
            client.println("<option value=\"12\">BUS Kanal Low 13</option>");
            client.println("<option value=\"13\">BUS Kanal Low 14</option>");
            client.println("<option value=\"14\">BUS Kanal Low 15</option>");
            client.println("<option value=\"15\">BUS Kanal Low 16</option>");
            client.println("</optgroup>");
            client.println("<optgroup label=\"BUS Kanal High\">");
      	    client.println("<option value=\"20\">BUS Kanal High 01</option>"); 
            client.println("<option value=\"21\">BUS Kanal High 02</option>"); 
            client.println("<option value=\"22\">BUS Kanal High 03</option>"); 
            client.println("<option value=\"23\">BUS Kanal High 04</option>"); 
            client.println("<option value=\"24\">BUS Kanal High 05</option>"); 
            client.println("<option value=\"25\">BUS Kanal High 06</option>"); 
            client.println("<option value=\"26\">BUS Kanal High 07</option>"); 
            client.println("<option value=\"27\">BUS Kanal High 08</option>"); 
            client.println("<option value=\"28\">BUS Kanal High 09</option>"); 
            client.println("<option value=\"29\">BUS Kanal High 10</option>"); 
            client.println("<option value=\"30\">BUS Kanal High 11</option>"); 
            client.println("<option value=\"31\">BUS Kanal High 12</option>"); 
            client.println("<option value=\"32\">BUS Kanal High 13</option>"); 
            client.println("<option value=\"33\">BUS Kanal High 14</option>"); 
            client.println("<option value=\"34\">BUS Kanal High 15</option>"); 
            client.println("<option value=\"35\">BUS Kanal High 16</option>"); 
            client.println("</optgroup>");
            client.println("<optgroup label=\"PWM Pin Low\">");
            client.println("<option value=\"40\">PWM Pin Low 01</option>"); 
            client.println("<option value=\"41\">PWM Pin Low 02</option>"); 
            client.println("<option value=\"42\">PWM Pin Low 03</option>"); 
            client.println("<option value=\"43\">PWM Pin Low 04</option>"); 
            client.println("<option value=\"44\">PWM Pin Low 05</option>"); 
            client.println("<option value=\"45\">PWM Pin Low 06</option>"); 
            client.println("</optgroup>");
            client.println("<optgroup label=\"PWM Pin High\">");
            client.println("<option value=\"50\">PWM Pin High 01</option>"); 
            client.println("<option value=\"51\">PWM Pin High 02</option>"); 
            client.println("<option value=\"52\">PWM Pin High 03</option>"); 
            client.println("<option value=\"53\">PWM Pin High 04</option>"); 
            client.println("<option value=\"54\">PWM Pin High 05</option>"); 
            client.println("<option value=\"55\">PWM Pin High 06</option>"); 
            client.println("</optgroup>");
            client.println("<optgroup label=\"Eingang Pin\">");
            client.println("<option value=\"60\">Eingang Pin 01</option>"); 
            client.println("<option value=\"61\">Eingang Pin 02</option>"); 
            client.println("<option value=\"62\">Eingang Pin 03</option>"); 
            client.println("<option value=\"63\">Eingang Pin 04</option>"); 
            client.println("<option value=\"64\">Eingang Pin 05</option>"); 
            client.println("<option value=\"65\">Eingang Pin 06</option>"); 
            client.println("</optgroup>");
            client.println("<optgroup label=\"Einkanal\">");
            client.println("<option value=\"70\">Einkanal 01</option>"); 
            client.println("<option value=\"71\">Einkanal 02</option>"); 
            client.println("<option value=\"72\">Einkanal 03</option>"); 
            client.println("<option value=\"73\">Einkanal 04</option>"); 
            client.println("<option value=\"74\">Einkanal 05</option>"); 
            client.println("<option value=\"75\">Einkanal 06</option>"); 
            client.println("<option value=\"76\">Einkanal 07</option>"); 
            client.println("<option value=\"77\">Einkanal 08</option>");                         
            client.println("</optgroup>");   
            client.println("<optgroup label=\"Optionen\">");
            client.println("<option value=\"200\">Dauerbetrieb an</option>"); 
            client.println("<option value=\"999\">Deaktiviert</option>");                            
            client.println("</optgroup>");                     
            client.println("</select><br></p>");


            client.println("<p class=\"text2\" >Quelle Motorspeed Motor</p>");

            // Quelle Motorspeed Motor
	          client.println("<p><select id=\"SoundSPEED\" class=\"buttonA\" onchange=\"setsoundspeed()\">");
            client.println("<optgroup label=\"BUS Kanal\">");
      	    client.println("<option value=\"0\">BUS Kanal 01</option>");
            client.println("<option value=\"1\">BUS Kanal 02</option>");
            client.println("<option value=\"2\">BUS Kanal 03</option>");
            client.println("<option value=\"3\">BUS Kanal 04</option>");
            client.println("<option value=\"4\">BUS Kanal 05</option>");
            client.println("<option value=\"5\">BUS Kanal 06</option>");
            client.println("<option value=\"6\">BUS Kanal 07</option>");
            client.println("<option value=\"7\">BUS Kanal 08</option>");
            client.println("<option value=\"8\">BUS Kanal 09</option>");
            client.println("<option value=\"9\">BUS Kanal 10</option>");
            client.println("<option value=\"10\">BUS Kanal 11</option>");
            client.println("<option value=\"11\">BUS Kanal 12</option>");
            client.println("<option value=\"12\">BUS Kanal 13</option>");
            client.println("<option value=\"13\">BUS Kanal 14</option>");
            client.println("<option value=\"14\">BUS Kanal 15</option>");
            client.println("<option value=\"15\">BUS Kanal 16</option>");
            client.println("<optgroup label=\"PWM Pin\">");
            client.println("<option value=\"20\">PWM Pin 01</option>"); 
            client.println("<option value=\"21\">PWM Pin 02</option>"); 
            client.println("<option value=\"22\">PWM Pin 03</option>"); 
            client.println("<option value=\"23\">PWM Pin 04</option>"); 
            client.println("<option value=\"24\">PWM Pin 05</option>"); 
            client.println("<option value=\"25\">PWM Pin 06</option>"); 
            client.println("</optgroup>");
            client.println("<optgroup label=\"Optionen\">"); 
            client.println("<option value=\"999\">Deaktiviert</option>");                            
            client.println("</optgroup>"); 
            client.println("</select><br></p>");

            client.println("<script> function setsoundspeed() { ");
            client.println("var sel = document.getElementById(\"SoundSPEED\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?SoundSPEED=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.Source_Speed_Sound_0, DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"SoundSPEED\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");            
             
             

            valueString = String(config.Volumen_Sound[Menu], DEC);
            client.println("<p class=\"text2\">Volumen : <span id=\"textVolumenSound\">" + valueString + "</span></p>");
            client.println("<p><input type=\"range\" min=\"0\" max=\"200\" step=\"5\" class=\"slider\" id=\"VolumenSound\" onchange=\"setVolumenSound(this.value)\" value=\"" + valueString + "\" /></p>");            

            valueString = String(config.Min_Speed_Sound_0, DEC);
            client.println("<p class=\"text2\">Drehzahl min : <span id=\"textDrehzahlmin\">" + valueString + " %</span></p>");
            client.println("<p><input type=\"range\" min=\"0\" max=\"200\" step=\"5\" class=\"slider\" id=\"Drehzahlmin\" onchange=\"setDrehzahlmin(this.value)\" value=\"" + valueString + "\" /></p>");            

            valueString = String(config.Max_Speed_Sound_0, DEC);
            client.println("<p class=\"text2\">Drehzahl max : <span id=\"textDrehzahlmax\">" + valueString + " %</span></p>");
            client.println("<p><input type=\"range\" min=\"100\" max=\"600\" step=\"5\" class=\"slider\" id=\"Drehzahlmax\" onchange=\"setDrehzahlmax(this.value)\" value=\"" + valueString + "\" /></p>");
            
            valueString = String(config.shutdowndelay, DEC);
            client.println("<p class=\"text2\">Motor aus Standgas in : <span id=\"textshutdowndelay\">" + valueString + " s</span></p>");
            client.println("<p><input type=\"range\" min=\"0\" max=\"60\" step=\"2\" class=\"slider\" id=\"Motor aus Verzögerung\" onchange=\"setshutdowndelay(this.value)\" value=\"" + valueString + "\" /></p>");

            valueString = String(config.throttle_ramp, DEC);
            client.println("<p class=\"text2\">Motor Rampe in : <span id=\"textthrottle_ramp\">" + valueString + " %/s</span></p>");
            client.println("<p><input type=\"range\" min=\"0\" max=\"50\" step=\"2\" class=\"slider\" id=\"Motor Rampe in\" onchange=\"setthrottle_ramp(this.value)\" value=\"" + valueString + "\" /></p>");

            valueString = String(config.throttle_dead_band, DEC);
            client.println("<p class=\"text2\">Standgas Totband in : <span id=\"textthrottle_dead_band\">" + valueString + " %</span></p>");
            client.println("<p><input type=\"range\" min=\"0\" max=\"50\" step=\"2\" class=\"slider\" id=\"Standgas Totband in\" onchange=\"setthrottle_dead_band(this.value)\" value=\"" + valueString + "\" /></p>");

            // Save Button
            client.println("<p><a href=\"/save\"><button class=\"button button2\">Save</button></a></p>");   

            client.println("<br />");
            client.println("<br />");
            client.println("<p class=\"text2\" >Voreinstellungen</p>");
            client.println("<br />");
            client.println("<p><a href=\"/setsbus\"><button class=\"button button2\">SBUS</button></a></p>");
            client.println("<br />");
            client.println("<p><a href=\"/setpwm\"><button class=\"button button2\">PWM</button></a></p>");
            client.println("<br />");
            client.println("<p><a href=\"/setpin\"><button class=\"button button2\">PIN</button></a></p>");
            client.println("<br />");

            client.println("<p><a href=\"/reset\"><button class=\"button button2\">Werkseinstellung</button></a></p>");

            break;

              case 1:
              case 2:
              case 3:
              case 4:
              case 5:
              case 6:
              case 7:
              case 8:
              
            client.println("<p class=\"text2\" ><b>Sound " + String(Menu) + " Einstellung</b></p>");  
            
            client.println("<p><a href=\"/back\"><button class=\"button button3\">Back</button></a>");
            client.println("<a href=\"/next\"><button class=\"button button4\">Next</button></a></p>");

            client.println("<br />");
            client.println("<br />");
            client.println("<br />");
            client.println("<br />");

            client.println("<p class=\"text2\" >Quelle Einschalten Sound</p>");


            // Quelle Einschalten Sound
	          client.println("<p><select id=\"SoundON\" class=\"buttonA\" onchange=\"setsoundon()\">");
            client.println("<optgroup label=\"BUS Kanal Low\">");
      	    client.println("<option value=\"0\">BUS Kanal Low 01</option>");
            client.println("<option value=\"1\">BUS Kanal Low 02</option>");
            client.println("<option value=\"2\">BUS Kanal Low 03</option>");
            client.println("<option value=\"3\">BUS Kanal Low 04</option>");
            client.println("<option value=\"4\">BUS Kanal Low 05</option>");
            client.println("<option value=\"5\">BUS Kanal Low 06</option>");
            client.println("<option value=\"6\">BUS Kanal Low 07</option>");
            client.println("<option value=\"7\">BUS Kanal Low 08</option>");
            client.println("<option value=\"8\">BUS Kanal Low 09</option>");
            client.println("<option value=\"9\">BUS Kanal Low 10</option>");
            client.println("<option value=\"10\">BUS Kanal Low 11</option>");
            client.println("<option value=\"11\">BUS Kanal Low 12</option>");
            client.println("<option value=\"12\">BUS Kanal Low 13</option>");
            client.println("<option value=\"13\">BUS Kanal Low 14</option>");
            client.println("<option value=\"14\">BUS Kanal Low 15</option>");
            client.println("<option value=\"15\">BUS Kanal Low 16</option>");
            client.println("</optgroup>");
            client.println("<optgroup label=\"BUS Kanal High\">");
      	    client.println("<option value=\"20\">BUS Kanal High 01</option>"); 
            client.println("<option value=\"21\">BUS Kanal High 02</option>"); 
            client.println("<option value=\"22\">BUS Kanal High 03</option>"); 
            client.println("<option value=\"23\">BUS Kanal High 04</option>"); 
            client.println("<option value=\"24\">BUS Kanal High 05</option>"); 
            client.println("<option value=\"25\">BUS Kanal High 06</option>"); 
            client.println("<option value=\"26\">BUS Kanal High 07</option>"); 
            client.println("<option value=\"27\">BUS Kanal High 08</option>"); 
            client.println("<option value=\"28\">BUS Kanal High 09</option>"); 
            client.println("<option value=\"29\">BUS Kanal High 10</option>"); 
            client.println("<option value=\"30\">BUS Kanal High 11</option>"); 
            client.println("<option value=\"31\">BUS Kanal High 12</option>"); 
            client.println("<option value=\"32\">BUS Kanal High 13</option>"); 
            client.println("<option value=\"33\">BUS Kanal High 14</option>"); 
            client.println("<option value=\"34\">BUS Kanal High 15</option>"); 
            client.println("<option value=\"35\">BUS Kanal High 16</option>"); 
            client.println("</optgroup>");
            client.println("<optgroup label=\"PWM Pin Low\">");
            client.println("<option value=\"40\">PWM Pin Low 01</option>"); 
            client.println("<option value=\"41\">PWM Pin Low 02</option>"); 
            client.println("<option value=\"42\">PWM Pin Low 03</option>"); 
            client.println("<option value=\"43\">PWM Pin Low 04</option>"); 
            client.println("<option value=\"44\">PWM Pin Low 05</option>"); 
            client.println("<option value=\"45\">PWM Pin Low 06</option>"); 
            client.println("</optgroup>");
            client.println("<optgroup label=\"PWM Pin High\">");
            client.println("<option value=\"50\">PWM Pin High 01</option>"); 
            client.println("<option value=\"51\">PWM Pin High 02</option>"); 
            client.println("<option value=\"52\">PWM Pin High 03</option>"); 
            client.println("<option value=\"53\">PWM Pin High 04</option>"); 
            client.println("<option value=\"54\">PWM Pin High 05</option>"); 
            client.println("<option value=\"55\">PWM Pin High 06</option>"); 
            client.println("</optgroup>");
            client.println("<optgroup label=\"Eingang Pin\">");
            client.println("<option value=\"60\">Eingang Pin 01</option>"); 
            client.println("<option value=\"61\">Eingang Pin 02</option>"); 
            client.println("<option value=\"62\">Eingang Pin 03</option>"); 
            client.println("<option value=\"63\">Eingang Pin 04</option>"); 
            client.println("<option value=\"64\">Eingang Pin 05</option>"); 
            client.println("<option value=\"65\">Eingang Pin 06</option>"); 
            client.println("</optgroup>");
            client.println("<optgroup label=\"Einkanal\">");
            client.println("<option value=\"70\">Einkanal 01</option>"); 
            client.println("<option value=\"71\">Einkanal 02</option>"); 
            client.println("<option value=\"72\">Einkanal 03</option>"); 
            client.println("<option value=\"73\">Einkanal 04</option>"); 
            client.println("<option value=\"74\">Einkanal 05</option>"); 
            client.println("<option value=\"75\">Einkanal 06</option>"); 
            client.println("<option value=\"76\">Einkanal 07</option>"); 
            client.println("<option value=\"77\">Einkanal 08</option>");                         
            client.println("</optgroup>");  
            client.println("<optgroup label=\"Ebenen Umschaltung\">");
            client.println("<option value=\"80\">Ebene 01 Kanal 01</option>"); 
            client.println("<option value=\"81\">Ebene 01 Kanal 02</option>"); 
            client.println("<option value=\"82\">Ebene 01 Kanal 03</option>"); 
            client.println("<option value=\"83\">Ebene 01 Kanal 04</option>"); 
            client.println("<option value=\"84\">Ebene 01 Kanal 05</option>"); 
            client.println("<option value=\"85\">Ebene 01 Kanal 06</option>"); 
            client.println("<option value=\"86\">Ebene 01 Kanal 07</option>"); 
            client.println("<option value=\"87\">Ebene 01 Kanal 08</option>");  
            client.println("<option value=\"88\">Ebene 02 Kanal 01</option>"); 
            client.println("<option value=\"89\">Ebene 02 Kanal 02</option>"); 
            client.println("<option value=\"90\">Ebene 02 Kanal 03</option>"); 
            client.println("<option value=\"91\">Ebene 02 Kanal 04</option>"); 
            client.println("<option value=\"92\">Ebene 02 Kanal 05</option>"); 
            client.println("<option value=\"93\">Ebene 02 Kanal 06</option>"); 
            client.println("<option value=\"94\">Ebene 02 Kanal 07</option>"); 
            client.println("<option value=\"95\">Ebene 02 Kanal 08</option>");
            client.println("<option value=\"96\">Ebene 03 Kanal 01</option>"); 
            client.println("<option value=\"97\">Ebene 03 Kanal 02</option>"); 
            client.println("<option value=\"98\">Ebene 03 Kanal 03</option>"); 
            client.println("<option value=\"99\">Ebene 03 Kanal 04</option>"); 
            client.println("<option value=\"100\">Ebene 03 Kanal 05</option>"); 
            client.println("<option value=\"101\">Ebene 03 Kanal 06</option>"); 
            client.println("<option value=\"102\">Ebene 03 Kanal 07</option>"); 
            client.println("<option value=\"103\">Ebene 03 Kanal 08</option>");                      
            client.println("</optgroup>");       
            client.println("<optgroup label=\"Optionen\">");
            client.println("<option value=\"200\">Dauerbetrieb an</option>"); 
            client.println("<option value=\"999\">Deaktiviert</option>");                            
            client.println("</optgroup>");        
            client.println("</select><br></p>");

            

            valueString = String(config.Volumen_Sound[Menu], DEC);
            client.println("<p class=\"text2\">Volumen : <span id=\"textVolumenSound\">" + valueString + "</span></p>");
            client.println("<p><input type=\"range\" min=\"0\" max=\"200\" step=\"5\" class=\"slider\" id=\"VolumenSound\" onchange=\"setVolumenSound(this.value)\" value=\"" + valueString + "\" /></p>");
            
            client.println("<p class=\"text2\" >Wiedergabe Sound</p>");
            // Loop Mode  
            client.println("<p><select id=\"SoundMODE\" class=\"buttonA\" onchange=\"setsoundmode()\">");
      	    client.println("<option value=\"0\">Normal</option>");
            client.println("<option value=\"1\">Loop</option>");
            client.println("<option value=\"2\">Tippbetrieb</option>");
            client.println("</select><br></p>");

            client.println("<script> function setsoundmode() { ");
            client.println("var sel = document.getElementById(\"SoundMODE\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?SoundMODE=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.Mode_Sound[Menu], DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"SoundMODE\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");   

            // Save Button
            client.println("<p><a href=\"/save\"><button class=\"button button2\">Save</button></a></p>");
            client.println("<p><a href=\"/Sound/on\"><button class=\"button button2\">Test Sound</button></a></p>");
                          
            break;                      
              case 9:
              
            client.println("<p class=\"text2\" ><b>Einstellung</b></p>");  
            
            client.println("<p><a href=\"/back\"><button class=\"button button3\">Back</button></a>");
            client.println("<a href=\"/next\"><button class=\"button button4\">Next</button></a></p>");

            client.println("<br />");
            client.println("<br />");
            client.println("<br />");
            client.println("<br />");

            client.println("<p class=\"text2\" >Einkanal Einstellungen</p>");

            if (config.Einkanal_RC_System != 4) {  // Keine ELRS (CRSF)
            client.println("<p class=\"text2\" >Einkanal Kanal</p>");

            client.println("<p><select id=\"SbuschannelEinkanal\" class=\"buttonA\" onchange=\"setsbuschanneleinkanal()\">");
            client.println("<optgroup label=\"SBUS Kanal\">");
      	    client.println("<option value=\"0\">SBUS Kanal 01</option>");
            client.println("<option value=\"1\">SBUS Kanal 02</option>");
            client.println("<option value=\"2\">SBUS Kanal 03</option>");
            client.println("<option value=\"3\">SBUS Kanal 04</option>");
            client.println("<option value=\"4\">SBUS Kanal 05</option>");
            client.println("<option value=\"5\">SBUS Kanal 06</option>");
            client.println("<option value=\"6\">SBUS Kanal 07</option>");
            client.println("<option value=\"7\">SBUS Kanal 08</option>");
            client.println("<option value=\"8\">SBUS Kanal 09</option>");
            client.println("<option value=\"9\">SBUS Kanal 10</option>");
            client.println("<option value=\"10\">SBUS Kanal 11</option>");
            client.println("<option value=\"11\">SBUS Kanal 12</option>");
            client.println("<option value=\"12\">SBUS Kanal 13</option>");
            client.println("<option value=\"13\">SBUS Kanal 14</option>");
            client.println("<option value=\"14\">SBUS Kanal 15</option>");
            client.println("<option value=\"15\">SBUS Kanal 16</option>");
            client.println("</optgroup>");
            client.println("<optgroup label=\"Optionen\">");
            client.println("<option value=\"999\">Deaktiviert</option>");                            
            client.println("</optgroup>"); 
            client.println("</select><br></p>");

            client.println("<script> function setsbuschanneleinkanal() { ");
            client.println("var sel = document.getElementById(\"SbuschannelEinkanal\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?SbuschannelEinkanal=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.Einkanal_Channel, DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"SbuschannelEinkanal\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");
            }

            if (config.Einkanal_RC_System == 4) {  // ELRS (CRSF)
            // CRSF Adresse; id= MODULADRESSE; script= setmoduladresse(); value= config.modul_adress;
            client.println("<p class=\"text2\" >Modul Adresse</p>");

            client.println("<p><select id=\"MODULADRESSE\" class=\"buttonA\" onchange=\"setmoduladresse()\">");
            client.println("<optgroup label=\"Modul Adresse\">");
      	    client.println("<option value=\"0\">0</option>");
            client.println("<option value=\"1\">1</option>");
            client.println("<option value=\"2\">2</option>");
            client.println("<option value=\"3\">3</option>");
            client.println("<option value=\"4\">4</option>");
            client.println("<option value=\"5\">5</option>");
            client.println("<option value=\"6\">6</option>");
            client.println("<option value=\"7\">7</option>");
            client.println("<option value=\"8\">8</option>");
            client.println("<option value=\"9\">9</option>");
            client.println("<option value=\"10\">10</option>");
            client.println("<option value=\"11\">11</option>");
            client.println("<option value=\"12\">12</option>");
            client.println("<option value=\"13\">13</option>");
            client.println("<option value=\"14\">14</option>");
            client.println("<option value=\"15\">15</option>");
            client.println("<option value=\"16\">16</option>");
            client.println("<option value=\"17\">17</option>");
            client.println("<option value=\"18\">18</option>");
            client.println("<option value=\"19\">19</option>");
            client.println("<option value=\"20\">20</option>");
            client.println("</optgroup>");
            client.println("</select><br></p>");

            client.println("<script> function setmoduladresse() { ");
            client.println("var sel = document.getElementById(\"MODULADRESSE\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?MODULADRESSE=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.modul_adress, DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"MODULADRESSE\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");
            }

            // RC-Sytem; id= RCSytem; script= setrcsytem(); value= Einkanal_RC_System;
            client.println("<p class=\"text2\" >RC-Sytem</p>");

            client.println("<p><select id=\"RCSytem\" class=\"buttonA\" onchange=\"setrcsytem()\">");
      	    client.println("<option value=\"0\">FrSky</option>");
            client.println("<option value=\"1\">FlySky</option>");
            client.println("<option value=\"2\">ELRS (SBUS)</option>");
            client.println("<option value=\"3\">Hott</option>");        
            client.println("<option value=\"4\">ELRS (CRSF)</option>");        
            client.println("</select><br></p>");

            client.println("<script> function setrcsytem() { ");
            client.println("var sel = document.getElementById(\"RCSytem\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?RCSytem=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.Einkanal_RC_System, DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"RCSytem\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");

            if (config.Einkanal_RC_System != 4) {  // Keine ELRS (CRSF)
            // Einkanal-Mode; id= EinkanalMode; script= seteinkanalmode(); value= config.einkanal_mode;
            client.println("<p class=\"text2\" >Einkanal-Mode</p>");

            client.println("<p><select id=\"EinkanalMode\" class=\"buttonA\" onchange=\"seteinkanalmode()\">");
      	    client.println("<option value=\"0\">Normal</option>");
            client.println("<option value=\"10\">SBUS WM Adr 0</option>");
            client.println("<option value=\"11\">SBUS WM Adr 1</option>");
            client.println("<option value=\"12\">SBUS WM Adr 2</option>");         
            client.println("<option value=\"13\">SBUS WM Adr 3</option>");    
            client.println("</select><br></p>");

            client.println("<script> function seteinkanalmode() { ");
            client.println("var sel = document.getElementById(\"EinkanalMode\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?EinkanalMode=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.Einkanal_mode, DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"EinkanalMode\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");
            }
 
            client.println("<hr>");

            // PWM MIN MAX
            client.println("<p class=\"text2\" >PWM Einstellungen</p>");
            client.println("<p class=\"text2\">PWM min (&micro;s)</p>");

            // PWM Min
            client.println("<input type=\"number\" class=\"buttonA\" "
               "value=\"" + String(config.PWM_scale_min) + "\" "
               "min=\"0\" max=\"4095\" "
               "onchange=\"location.href='/setPWM?pwmMin='+this.value+'&pwmMax=" + String(config.PWM_scale_max) + "'\">");
            
            client.println("<p class=\"text2\">PWM max (&micro;s)</p>");

            // PWM Max
            client.println("<input type=\"number\" class=\"buttonA\" "
               "value=\"" + String(config.PWM_scale_max) + "\" "
               "min=\"0\" max=\"4095\" "
               "onchange=\"location.href='/setPWM?pwmMin=" + String(config.PWM_scale_min) + "&pwmMax='+this.value\">");


            client.println("<hr>");            

            // Ebenen Umschaltung

            client.println("<p class=\"text2\" >Ebenen Umschaltung Einstellungen</p>");
            client.println("<p class=\"text2\" >Ebenen Umschaltung Kanal</p>");

            client.println("<p><select id=\"EbenenUmschaltungKanal\" class=\"buttonA\" onchange=\"setebenenumschaltungkanal()\">");
            client.println("<optgroup label=\"BUS Kanal\">");
      	    client.println("<option value=\"0\">BUS Kanal 01</option>");
            client.println("<option value=\"1\">BUS Kanal 02</option>");
            client.println("<option value=\"2\">BUS Kanal 03</option>");
            client.println("<option value=\"3\">BUS Kanal 04</option>");
            client.println("<option value=\"4\">BUS Kanal 05</option>");
            client.println("<option value=\"5\">BUS Kanal 06</option>");
            client.println("<option value=\"6\">BUS Kanal 07</option>");
            client.println("<option value=\"7\">BUS Kanal 08</option>");
            client.println("<option value=\"8\">BUS Kanal 09</option>");
            client.println("<option value=\"9\">BUS Kanal 10</option>");
            client.println("<option value=\"10\">BUS Kanal 11</option>");
            client.println("<option value=\"11\">BUS Kanal 12</option>");
            client.println("<option value=\"12\">BUS Kanal 13</option>");
            client.println("<option value=\"13\">BUS Kanal 14</option>");
            client.println("<option value=\"14\">BUS Kanal 15</option>");
            client.println("<option value=\"15\">BUS Kanal 16</option>");
            client.println("<optgroup label=\"PWM Pin\">");
            client.println("<option value=\"20\">PWM Pin 01</option>"); 
            client.println("<option value=\"21\">PWM Pin 02</option>"); 
            client.println("<option value=\"22\">PWM Pin 03</option>"); 
            client.println("<option value=\"23\">PWM Pin 04</option>"); 
            client.println("<option value=\"24\">PWM Pin 05</option>"); 
            client.println("<option value=\"25\">PWM Pin 06</option>"); 
            client.println("</optgroup>");
            client.println("<optgroup label=\"Optionen\">");
            client.println("<option value=\"999\">Deaktiviert</option>");                            
            client.println("</optgroup>"); 
            client.println("</select><br></p>");

            client.println("<script> function setebenenumschaltungkanal() { ");
            client.println("var sel = document.getElementById(\"EbenenUmschaltungKanal\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?EbenenUmschaltungKanal=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.Source_Ebenen_Um_Kanal, DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"EbenenUmschaltungKanal\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");

            client.println("<p class=\"text2\" >Ebenen Kanal</p>");

            client.println("<p><select id=\"EbenenKanal\" class=\"buttonA\" onchange=\"setebenenkanal()\">");
            client.println("<optgroup label=\"BUS Kanal\">");
      	    client.println("<option value=\"0\">BUS Kanal 01</option>");
            client.println("<option value=\"1\">BUS Kanal 02</option>");
            client.println("<option value=\"2\">BUS Kanal 03</option>");
            client.println("<option value=\"3\">BUS Kanal 04</option>");
            client.println("<option value=\"4\">BUS Kanal 05</option>");
            client.println("<option value=\"5\">BUS Kanal 06</option>");
            client.println("<option value=\"6\">BUS Kanal 07</option>");
            client.println("<option value=\"7\">BUS Kanal 08</option>");
            client.println("<option value=\"8\">BUS Kanal 09</option>");
            client.println("<option value=\"9\">BUS Kanal 10</option>");
            client.println("<option value=\"10\">BUS Kanal 11</option>");
            client.println("<option value=\"11\">BUS Kanal 12</option>");
            client.println("<option value=\"12\">BUS Kanal 13</option>");
            client.println("<option value=\"13\">BUS Kanal 14</option>");
            client.println("<option value=\"14\">BUS Kanal 15</option>");
            client.println("<option value=\"15\">BUS Kanal 16</option>");
            client.println("<optgroup label=\"PWM Pin\">");
            client.println("<option value=\"20\">PWM Pin 01</option>"); 
            client.println("<option value=\"21\">PWM Pin 02</option>"); 
            client.println("<option value=\"22\">PWM Pin 03</option>"); 
            client.println("<option value=\"23\">PWM Pin 04</option>"); 
            client.println("<option value=\"24\">PWM Pin 05</option>"); 
            client.println("<option value=\"25\">PWM Pin 06</option>"); 
            client.println("</optgroup>");
            client.println("<optgroup label=\"Optionen\">");
            client.println("<option value=\"999\">Deaktiviert</option>");                            
            client.println("</optgroup>"); 
            client.println("</select><br></p>");

            client.println("<script> function setebenenkanal() { ");
            client.println("var sel = document.getElementById(\"EbenenKanal\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?EbenenKanal=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.Source_Ebenen_Kanal, DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"EbenenKanal\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");

            client.println("<hr>");

            // Ebenen Umschaltung

            client.println("<p class=\"text2\" >Hardware Einstellungen</p>");
            client.println("<p class=\"text2\" >Hardwareconfig</p>");

            client.println("<p><select id=\"HardwareConfig\" class=\"buttonA\" onchange=\"setHardwareConfig()\">");
      	    client.println("<option value=\"0\">V1</option>");
            client.println("<option value=\"1\">V2</option>");
            client.println("</select><br></p>");

            client.println("<script> function setHardwareConfig() { ");
            client.println("var sel = document.getElementById(\"HardwareConfig\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?HardwareConfig=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.Hardware_Config, DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"HardwareConfig\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");

            // Save Button
            client.println("<p><a href=\"/save\"><button class=\"button button2\">Save</button></a></p>");

            break;
              case 10:
              
            client.println("<p class=\"text2\" ><b>Debug Info</b></p>");  
            
            client.println("<p><a href=\"/back\"><button class=\"button button3\">Back</button></a>");
            client.println("<a href=\"/next\"><button class=\"button button4\">Next</button></a></p>");
            
            client.println("<br />");
            client.println("<br />");
            
            // BUS Kanal 1 bis 8 Ausgeben
            client.println("<div class=\"left\" >");
            valueString = String(channel_output[0], DEC);
            client.println("<p><br> K1: " + valueString);        
            valueString = String(channel_output[1], DEC);
            client.println("<br> K2: " + valueString);
            valueString = String(channel_output[2], DEC);
            client.println("<br> K3: " + valueString);
            valueString = String(channel_output[3], DEC);
            client.println("<br> K4: " + valueString);
            valueString = String(channel_output[4], DEC);
            client.println("<br> K5: " + valueString);
            valueString = String(channel_output[5], DEC);
            client.println("<br> K6: " + valueString);
            valueString = String(channel_output[6], DEC);
            client.println("<br> K7: " + valueString);
            valueString = String(channel_output[7], DEC);
            client.println("<br> K8: " + valueString + "</p> <hr>");

            // PWM Pin 1 bis 6 Ausgeben
            client.println("<p>PWM PIN 1: " + String(PWM_pulse_width[0], DEC) + " &micro;s -> " + String(map(PWM_pulse_width[0], config.PWM_scale_min, config.PWM_scale_max, 0, 100), DEC) + " %");
            client.println("<br>PWM PIN 2: " + String(PWM_pulse_width[1], DEC) + " &micro;s -> " + String(map(PWM_pulse_width[1], config.PWM_scale_min, config.PWM_scale_max, 0, 100), DEC) + " %");
            client.println("<br>PWM PIN 3: " + String(PWM_pulse_width[2], DEC) + " &micro;s -> " + String(map(PWM_pulse_width[2], config.PWM_scale_min, config.PWM_scale_max, 0, 100), DEC) + " %");
            client.println("<br>PWM PIN 4: " + String(PWM_pulse_width[3], DEC) + " &micro;s -> " + String(map(PWM_pulse_width[3], config.PWM_scale_min, config.PWM_scale_max, 0, 100), DEC) + " %");
            client.println("<br>PWM PIN 5: " + String(PWM_pulse_width[4], DEC) + " &micro;s -> " + String(map(PWM_pulse_width[4], config.PWM_scale_min, config.PWM_scale_max, 0, 100), DEC) + " %");
            client.println("<br>PWM PIN 6: " + String(PWM_pulse_width[5], DEC) + " &micro;s -> " + String(map(PWM_pulse_width[5], config.PWM_scale_min, config.PWM_scale_max, 0, 100), DEC) + " %");
            client.println("</p> <hr>");

            // Datei laden OK ?
            valueString = String(Sound_loop.FileOK, DEC);
            client.println("<p> loop.wav OK: " + valueString);
            valueString = String(Sound_shut.FileOK, DEC);
            client.println("<br> shut.wav OK : " + valueString);
            valueString = String(Sound_start.FileOK, DEC);
            client.println("<br> start.wav OK : " + valueString);            
            valueString = String(Sound1.FileOK, DEC);
            client.println("<br> sound1.wav OK: " + valueString);
            valueString = String(Sound2.FileOK, DEC);
            client.println("<br> sound2.wav OK: " + valueString);     
            valueString = String(Sound3.FileOK, DEC);
            client.println("<br> sound3.wav OK: " + valueString);   
            valueString = String(Sound4.FileOK, DEC);
            client.println("<br> sound4.wav OK: " + valueString);   
            valueString = String(Sound5.FileOK, DEC);
            client.println("<br> sound5.wav OK: " + valueString);   
            valueString = String(Sound6.FileOK, DEC);
            client.println("<br> sound6.wav OK: " + valueString);   
            valueString = String(Sound7.FileOK, DEC);
            client.println("<br> sound7.wav OK: " + valueString);          
            valueString = String(Sound8.FileOK, DEC);
            client.println("<br> sound8.wav OK: " + valueString + "</p> <hr>");

            // Konfig Sound 1 bis 9 Ausgeben
            valueString = String(config.Hardware_Config, DEC);
            client.println("<p> Konfig Hardware: " + valueString);  
            valueString = String(config.Source_Start_Sound[0], DEC);
            client.println("<br> Konfig Motor Start: " + valueString);        
            valueString = String(config.Source_Start_Sound[1], DEC);
            client.println("<br> Konfig Sound 1 Start: " + valueString);
            valueString = String(config.Source_Start_Sound[2], DEC);
            client.println("<br> Konfig Sound 2 Start: " + valueString);
            valueString = String(config.Source_Start_Sound[3], DEC);
            client.println("<br> Konfig Sound 3 Start: " + valueString);
            valueString = String(config.Source_Start_Sound[4], DEC);
            client.println("<br> Konfig Sound 4 Start: " + valueString);
            valueString = String(config.Source_Start_Sound[5], DEC);
            client.println("<br> Konfig Sound 5 Start: " + valueString);
            valueString = String(config.Source_Start_Sound[6], DEC);
            client.println("<br> Konfig Sound 6 Start: " + valueString);
            valueString = String(config.Source_Start_Sound[7], DEC);
            client.println("<br> Konfig Sound 7 Start: " + valueString);
            valueString = String(config.Source_Start_Sound[8], DEC);
            client.println("<br> Konfig Sound 8 Start: " + valueString);            
            valueString = String(config.Source_Speed_Sound_0, DEC);
            client.println("<br> Konfig Speed Sound 1: " + valueString); 
            valueString = String(config.throttle_ramp, DEC);
            client.println("<br> Konfig Gas Ranpe: " + valueString);
            valueString = String(config.throttle_dead_band, DEC);
            client.println("<br> Konfig Totband: " + valueString);                        
            valueString = String(config.Source_Ebenen_Um_Kanal, DEC);
            client.println("<br> Konfig Eben Kanal Umschalter: " + valueString);
            valueString = String(config.Source_Ebenen_Kanal, DEC);
            client.println("<br> Konfig Eben Kanal Wert: " + valueString);
            valueString = String(config.Einkanal_RC_System, DEC);
            client.println("<br> Konfig Einkanal RC System: " + valueString);
            valueString = String(config.Einkanal_Channel, DEC);
            client.println("<br> Konfig Einkanal Channel: " + valueString);
            valueString = String(config.Einkanal_mode, DEC);
            client.println("<br> Konfig Einkanal Mode: " + valueString);
            valueString = String(config.modul_adress, DEC);
            client.println("<br> Konfig CRSF Modul Adress: " + valueString + "</p> </div>");
            }

            client.println("<script> function setsoundon() { ");
            client.println("var sel = document.getElementById(\"SoundON\");");
            client.println("var opt = sel.options[sel.selectedIndex];");
            client.println("var val = opt.value;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?SoundON=\" + val + \"&\", true);");
            client.println("xhr.send(); } </script>");

            valueString = String(config.Source_Start_Sound[Menu], DEC);

            client.println("<script> ");
            client.println("var selectedOption =" + valueString + ";");
            client.println("var selectElement = document.getElementById(\"SoundON\");");
            client.println("for (var i = 0; i < selectElement.options.length; i++) {");
            client.println("var option = selectElement.options[i];");
            client.println("if (option.value == selectedOption) {");
            client.println("option.selected = true;");
            client.println("break; }  } </script>");

            client.println("<script> function setVolumenSound(pos) { ");
            client.println("var VolumenValue = document.getElementById(\"VolumenSound\").value;");
            client.println("document.getElementById(\"textVolumenSound\").innerHTML = VolumenValue;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?VolumenSound=\" + pos + \"&\", true);");
            client.println("xhr.send(); } </script>");

            client.println("<script> function setDrehzahlmin(pos) { ");
            client.println("var VolumenValue = document.getElementById(\"Drehzahlmin\").value;");
            client.println("document.getElementById(\"textDrehzahlmin\").innerHTML = VolumenValue + \" %\";");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?Drehzahlmin=\" + pos + \"&\", true);");
            client.println("xhr.send(); } </script>");

            client.println("<script> function setDrehzahlmax(pos) { ");
            client.println("var VolumenValue = document.getElementById(\"Drehzahlmax\").value;");
            client.println("document.getElementById(\"textDrehzahlmax\").innerHTML = VolumenValue + \" %\";");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?Drehzahlmax=\" + pos + \"&\", true);");
            client.println("xhr.send(); } </script>");

            client.println("<script> function setshutdowndelay(pos) { ");
            client.println("var VolumenValue = document.getElementById(\"Motor aus Verzögerung\").value;");
            client.println("document.getElementById(\"textshutdowndelay\").innerHTML = VolumenValue + \" s\";");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?shutdowndelay=\" + pos + \"&\", true);");
            client.println("xhr.send(); } </script>");

            client.println("<script> function setthrottle_ramp(pos) { "); 
            client.println("var VolumenValue = document.getElementById(\"Motor Rampe in\").value;");
            client.println("document.getElementById(\"textthrottle_ramp\").innerHTML = VolumenValue + \" %/s\";");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?ThrottleRamp=\" + pos + \"&\", true);");
            client.println("xhr.send(); } </script>");

            client.println("<script> function setthrottle_dead_band(pos) { ");
            client.println("var VolumenValue = document.getElementById(\"Standgas Totband in\").value;");
            client.println("document.getElementById(\"textthrottle_dead_band\").innerHTML = VolumenValue + \" %\";");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?ThrottleDeadBand=\" + pos + \"&\", true);");
            client.println("xhr.send(); } </script>");                        
            
            client.println("</body></html>");     
                    
            // Die HTTP-Antwort endet mit einer weiteren Leerzeile
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // header löschen
    header = "";
    // Schließen Sie die Verbindung
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
    }
}    

