
#include <Arduino.h>
//TO UPLOAD FS:
//pio run --target uploadfs --upload-protocol espota --upload-port 192.168.1.14
//TO UPLOAD CODE:
//pio run --target upload --upload-protocol espota --upload-port 192.168.1.14

#include <CRC32.h>
#include <EEPROM.h>

#include <LittleFS.h>
#include <SoftwareSerial.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

#include "melodies.h"

#include "config.h"

#define EEPROM_SIZE 512

typedef struct EEPROM_Settings_S{
    uint8_t wifiMode;
    char staName[32];
    char staPassword[64];
    uint8_t connectTimeout;
    char apName[32];
    char apPassword[64];

    float Kp;
    float Ki;
    float Kd;

    int16_t servosPwmCorrection[6];

    //keep at the end
    uint32_t crc;
} __attribute__((packed)) Settings;

Settings settings;

#define MASSAGE_ASCII 1

#ifndef SOFT_AP_NAME
#define SOFT_AP_NAME "Rover"
#endif

#ifndef SOFT_AP_PASSWORD
#define SOFT_AP_PASSWORD "R0v3rcmd"
#endif

#ifndef WIFI_DEFAULT_STA_NAME
#define WIFI_DEFAULT_STA_NAME "LocalNetwork"
#endif

#ifndef WIFI_DEFAULT_STA_PASSWORD
#define WIFI_DEFAULT_STA_PASSWORD "LocalNetworkPassword"
#endif

#define MOTORS_MODE_SPEED (1)
#define MOTORS_MODE_PWM (2)

char currentWifiName[32];
char currentWifiPassword[64];

typedef struct EcuStats_S{
  uint32_t time;
  int8_t motorsMode;
  int16_t motorsSpeed[6];
  int16_t motorsProbe[6];
  int16_t motorsPwm[6];
  int16_t servosPwm[6];
  float power;
  char camIp[32];
} EcuStats;

EcuStats ecuStats;

uint32_t nextSendSettingsTime = 0;

uint32_t nextSendWifiCredentialsTime = 0;

int bottomLed=0;
int turretLed=0;
int turretLaser=0;

SoftwareSerial swSer;

Melody currentMelody;
uint32_t currentMelodyNextTime=0L;

#if MASSAGE_ASCII
    #include <AsciiMassagePacker.h>
    #include <AsciiMassageParser.h>
    AsciiMassagePacker massageOutbound;
    AsciiMassageParser massageInbound;
    AsciiMassagePacker massageOutboundSw;
    AsciiMassageParser massageInboundSw;
#else
    #include <SlipMassagePacker.h>
    #include <SlipMassageParser.h>
    SlipMassagePacker massageOutbound;
    SlipMassageParser massageInbound;
    SlipMassagePacker massageOutboundSw;
    SlipMassageParser massageInboundSw;
#endif


ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

void handleEcuCommand();              // function prototypes for HTTP handlers
void handleWifiSetupCommand();              // function prototypes for HTTP handlers
String getContentType(String filename);
bool handleFileRead(String path);
bool loadSettings();
bool saveSettings();


void sendWifiCredentials(){

    if (strlen(currentWifiName)==0) return;
    if (strlen(currentWifiPassword)==0) return;
    
    massageOutboundSw.beginPacket("WiFi");
    massageOutboundSw.addInt(WiFi.status());
    massageOutboundSw.addString(currentWifiName);
    massageOutboundSw.addString(currentWifiPassword);
    massageOutboundSw.addString(WiFi.localIP().toString().c_str());
    massageOutboundSw.streamPacket(&swSer);

}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".js.gz")) return "application/javascript";
  else if(filename.endsWith(".css.gz")) return "text/css";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  else if(filename.endsWith(".svg")) return "image/svg+xml";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  //Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (LittleFS.exists(path + ".gz")) {                            // If the file exists
    File file = LittleFS.open(path + ".gz", "r");                 // Open it
    //server.sendHeader("Content-Encoding", "gzip",false);
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  else if (LittleFS.exists(path)) {                            // If the file exists
    File file = LittleFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  //Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

void playNextTone(){

    if (currentMelodyNextTime > 0 && millis() >= currentMelodyNextTime){

        if (*currentMelody.data){
            int divider = *(currentMelody.data+1);
            int currentNote = *(currentMelody.data+0);

            int noteDuration;
            if (divider > 0) {
                // regular note, just proceed
                noteDuration = currentMelody.tempo / divider;
            } else {
                // dotted notes are represented with negative durations!!
                noteDuration = currentMelody.tempo / (-divider) * 1.5f; // increases the duration in half for dotted notes
            }

            currentMelodyNextTime = millis() + noteDuration * 1.30;

            if ( (currentMelody.mode & MELODY_MODE_MUSIC) && currentNote > 1){
                massageOutbound.beginPacket("sTN");
                massageOutbound.addInt(currentNote);
                massageOutbound.addLong(noteDuration);
                massageOutbound.streamPacket(&Serial);
            }

            if (currentMelody.mode & MELODY_MODE_LED){
                turretLed = turretLed ? 0 : 1;
            }
            if (currentMelody.mode & MELODY_MODE_LASER){
                turretLaser = !turretLaser;
            }

            if (currentMelody.mode & MELODY_MODE_STEER_WHEELS){
                massageOutbound.beginPacket("sSP");
                massageOutbound.addInt(random(250,330));
                massageOutbound.addInt(random(250,330));
                massageOutbound.addInt(random(250,330));
                massageOutbound.addInt(random(250,330));
                massageOutbound.addInt(random(218,355));
                massageOutbound.addInt(random(218,355));
                massageOutbound.streamPacket(&Serial);
            }

            currentMelody.data+=2;

        }
        else{
            //stop
            currentMelodyNextTime=0L;
            turretLed = 0;
            turretLaser=0;
        }

        massageOutbound.beginPacket("sLP");
        massageOutbound.addInt(bottomLed);
        massageOutbound.addInt(turretLed);
        massageOutbound.addInt(turretLaser);
        massageOutbound.streamPacket(&Serial);

        massageOutboundSw.beginPacket("sLP");
        massageOutboundSw.addInt(bottomLed);
        massageOutboundSw.addInt(turretLed);
        massageOutboundSw.addInt(turretLaser);
        massageOutboundSw.streamPacket(&swSer);

    }

}

void handleMelodyCommand() {
    int res = 0;
    if (server.method() == HTTP_POST){
        int melodyIndex = atoi(server.arg("melody").c_str());
        int stop = atoi(server.arg("stop").c_str());

        if (stop){
            res=2;
            currentMelodyNextTime=0;
        }
        else if (melodyIndex >= 0 && melodyIndex < (int)MELODIES_COUNT){
            //execute melody

            currentMelody = MELODIES[melodyIndex];
            currentMelodyNextTime = millis();

            res=1;

        }
        else{
            res = 3;
        }

    }


    StaticJsonDocument<1024> json;
    char buffer[1024];
    JsonObject root = json.to<JsonObject>();

    root["res"] = res;
    
    serializeJson(root, buffer);
    server.send(200, "application/json", buffer);
}

void handleAccCommand() {
    if (server.method() == HTTP_POST){
        bottomLed = atoi(server.arg("bottomLed").c_str());
        turretLed = atoi(server.arg("turretLed").c_str());
        turretLaser = atoi(server.arg("turretLaser").c_str());

        massageOutbound.beginPacket("sLP");
        massageOutbound.addInt(bottomLed);
        massageOutbound.addInt(turretLed);
        massageOutbound.addInt(turretLaser);
        massageOutbound.streamPacket(&Serial);

        massageOutboundSw.beginPacket("sLP");
        massageOutboundSw.addInt(bottomLed);
        massageOutboundSw.addInt(turretLed);
        massageOutboundSw.addInt(turretLaser);
        massageOutboundSw.streamPacket(&swSer);

    }

    StaticJsonDocument<1024> json;
    char buffer[1024];

    JsonObject root = json.to<JsonObject>();

    root["bottomLed"] = bottomLed;
    root["turretLed"] = turretLed;
    root["turretLaser"] = turretLaser;
    
    serializeJson(root, buffer);

    server.send(200, "application/json", buffer);   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleEcuCommand() {

    if (server.method() == HTTP_POST)
    {

        if (atoi(server.arg("mode").c_str()) == MOTORS_MODE_SPEED){
            massageOutbound.beginPacket("sMS");
            massageOutbound.addInt(atoi(server.arg("m0").c_str()));
            massageOutbound.addInt(atoi(server.arg("m1").c_str()));
            massageOutbound.addInt(atoi(server.arg("m2").c_str()));
            massageOutbound.addInt(atoi(server.arg("m3").c_str()));
            massageOutbound.addInt(atoi(server.arg("m4").c_str()));
            massageOutbound.addInt(atoi(server.arg("m5").c_str()));
            massageOutbound.streamPacket(&Serial);
        }
        else if (atoi(server.arg("mode").c_str()) == MOTORS_MODE_PWM){
            massageOutbound.beginPacket("sMP");
            massageOutbound.addInt(atoi(server.arg("m0").c_str()));
            massageOutbound.addInt(atoi(server.arg("m1").c_str()));
            massageOutbound.addInt(atoi(server.arg("m2").c_str()));
            massageOutbound.addInt(atoi(server.arg("m3").c_str()));
            massageOutbound.addInt(atoi(server.arg("m4").c_str()));
            massageOutbound.addInt(atoi(server.arg("m5").c_str()));
            massageOutbound.streamPacket(&Serial);
        }

        massageOutbound.beginPacket("sSP");
        massageOutbound.addInt(atoi(server.arg("s0").c_str()) + settings.servosPwmCorrection[0]);
        massageOutbound.addInt(atoi(server.arg("s1").c_str()) + settings.servosPwmCorrection[1]);
        massageOutbound.addInt(atoi(server.arg("s2").c_str()) + settings.servosPwmCorrection[2]);
        massageOutbound.addInt(atoi(server.arg("s3").c_str()) + settings.servosPwmCorrection[3]);
        massageOutbound.addInt(atoi(server.arg("s4").c_str()) + settings.servosPwmCorrection[4]);
        massageOutbound.addInt(atoi(server.arg("s5").c_str()) + settings.servosPwmCorrection[5]);
        massageOutbound.streamPacket(&Serial);
    }

    {
        StaticJsonDocument<1024> json;
        char buffer[1024];

        JsonObject root = json.to<JsonObject>();

        root["time"] = ecuStats.time;
        root["motorsMode"] = ecuStats.motorsMode;
        root["power"] = ecuStats.power;
        root["camera"] = ecuStats.camIp;
        root["rssi"] = WiFi.RSSI();

        JsonArray motorsSpeed = root.createNestedArray("motorsSpeed");
        motorsSpeed.add(ecuStats.motorsSpeed[0]);
        motorsSpeed.add(ecuStats.motorsSpeed[1]);
        motorsSpeed.add(ecuStats.motorsSpeed[2]);
        motorsSpeed.add(ecuStats.motorsSpeed[3]);
        motorsSpeed.add(ecuStats.motorsSpeed[4]);
        motorsSpeed.add(ecuStats.motorsSpeed[5]);

        JsonArray motorsProbe = root.createNestedArray("motorsProbe");
        motorsProbe.add(ecuStats.motorsProbe[0]);
        motorsProbe.add(ecuStats.motorsProbe[1]);
        motorsProbe.add(ecuStats.motorsProbe[2]);
        motorsProbe.add(ecuStats.motorsProbe[3]);
        motorsProbe.add(ecuStats.motorsProbe[4]);
        motorsProbe.add(ecuStats.motorsProbe[5]);

        JsonArray motorsPwm = root.createNestedArray("motorsPwm");
        motorsPwm.add(ecuStats.motorsPwm[0]);
        motorsPwm.add(ecuStats.motorsPwm[1]);
        motorsPwm.add(ecuStats.motorsPwm[2]);
        motorsPwm.add(ecuStats.motorsPwm[3]);
        motorsPwm.add(ecuStats.motorsPwm[4]);
        motorsPwm.add(ecuStats.motorsPwm[5]);

        JsonArray servosPwm = root.createNestedArray("servosPwm");
        servosPwm.add(ecuStats.servosPwm[0]);
        servosPwm.add(ecuStats.servosPwm[1]);
        servosPwm.add(ecuStats.servosPwm[2]);
        servosPwm.add(ecuStats.servosPwm[3]);
        servosPwm.add(ecuStats.servosPwm[4]);
        servosPwm.add(ecuStats.servosPwm[5]);

        serializeJson(root, buffer);
    
        server.send(200, "application/json", buffer);
    }
}

void handleServoSetupCommand()
{
    if (server.method() == HTTP_POST)
    {
        settings.servosPwmCorrection[0] = atoi(server.arg("s0").c_str());
        settings.servosPwmCorrection[1] = atoi(server.arg("s1").c_str());
        settings.servosPwmCorrection[2] = atoi(server.arg("s2").c_str());
        settings.servosPwmCorrection[3] = atoi(server.arg("s3").c_str());
        settings.servosPwmCorrection[4] = atoi(server.arg("s4").c_str());
        settings.servosPwmCorrection[5] = atoi(server.arg("s5").c_str());

        settings.Kp = atof(server.arg("kp").c_str());
        settings.Ki = atof(server.arg("ki").c_str());
        settings.Kd = atof(server.arg("kd").c_str());

        sendSettingsToEcu();

        if (atoi(server.arg("save").c_str())){
            saveSettings();
        }

    }

    {
        StaticJsonDocument<1024> json;
        char buffer[1024];

        JsonObject root = json.to<JsonObject>();

        root["kp"] = settings.Kp;
        root["ki"] = settings.Ki;
        root["kd"] = settings.Kd;

        JsonArray servos = root.createNestedArray("servos");
        servos.add(settings.servosPwmCorrection[0]);
        servos.add(settings.servosPwmCorrection[1]);
        servos.add(settings.servosPwmCorrection[2]);
        servos.add(settings.servosPwmCorrection[3]);
        servos.add(settings.servosPwmCorrection[4]);
        servos.add(settings.servosPwmCorrection[5]);

        serializeJson(root, buffer);
    
        server.send(200, "application/json", buffer);
    }

}

void handleWifiSetupCommand(){

    bool error = false;

    if (server.method() == HTTP_POST){
        for (uint8_t i = 0; i < server.args(); i++)
        {
            const String name = server.argName(i);
            const String value = server.arg(i);
            if (name.equals("staName") && value.length() < 32)
            {
                strcpy(settings.staName, value.c_str());
            }
            else if (name.equals("staPassword") && value.length() < 64)
            {
                strcpy(settings.staPassword, value.c_str());
            }
            else if (name.equals("apName") && value.length() < 32)
            {
                strcpy(settings.apName, value.c_str());
            }
            else if (name.equals("apPassword") && value.length() < 64)
            {
                strcpy(settings.apPassword, value.c_str());
            }
            else
            {
                error = true;
            }
        }
    }

    {
        StaticJsonDocument<1024> json;
        char buffer[1024];

        JsonObject root = json.to<JsonObject>();

        root["staName"] = settings.staName;
        root["staPassword"] = settings.staPassword;
        root["apName"] = settings.apName;
        root["apPassword"] = settings.apPassword;
        root["error"] = error;

        serializeJson(root, buffer);
    
        server.send(200, "application/json", buffer);
    }

}

void sendSettingsToEcu(){

    massageOutbound.beginPacket("sMK");
    massageOutbound.addFloat(settings.Kp);
    massageOutbound.addFloat(settings.Ki);
    massageOutbound.addFloat(settings.Kd);
    massageOutbound.streamPacket(&Serial);

    massageOutbound.beginPacket("sSP");
    massageOutbound.addInt(205+102 + settings.servosPwmCorrection[0]);
    massageOutbound.addInt(205+102 + settings.servosPwmCorrection[1]);
    massageOutbound.addInt(205+102 + settings.servosPwmCorrection[2]);
    massageOutbound.addInt(205+102 + settings.servosPwmCorrection[3]);
    massageOutbound.addInt(205+102 + settings.servosPwmCorrection[4]);
    massageOutbound.addInt(205+102 + settings.servosPwmCorrection[5]);
    massageOutbound.streamPacket(&Serial);

}

bool loadSettings(){
    EEPROM.begin(EEPROM_SIZE);  //Initialize EEPROM
    EEPROM.get(0,settings);
    EEPROM.end();

    uint32_t savedCrc = settings.crc;

    settings.crc = 0;
    uint32_t checksum = CRC32::calculate(&settings, 1);

    bool ret;

    if (savedCrc != checksum){
        //wrong CRC, restore defaults

        settings.wifiMode = WIFI_STA;
        strcpy(settings.staName,WIFI_DEFAULT_STA_NAME);
        strcpy(settings.staPassword,WIFI_DEFAULT_STA_PASSWORD);
        settings.connectTimeout = 30;
        strcpy(settings.apName,SOFT_AP_NAME);
        strcpy(settings.apPassword,SOFT_AP_PASSWORD);
        memset(settings.servosPwmCorrection,0,sizeof(settings.servosPwmCorrection));
        settings.Kp = 0.5;
        settings.Ki = 10.0;
        settings.Kd = 0.01;

        ret = false;
    }
    else{
        ret = true;
    }

    return ret;

}
bool saveSettings(){
    EEPROM.begin(EEPROM_SIZE);
    settings.crc = 0;
    settings.crc = CRC32::calculate(&settings, 1);
    EEPROM.put(0,settings);
    EEPROM.end();
    return true;
}

void setup()
{
    Serial.begin(115200);

    //Software serial for esp32 camera communication
    //PIN 2: ESP8266 RX <----- ESP32 TX
    //PIN 4: ESP8266 TX -----> ESP32 RX
    swSer.begin(9600, SWSERIAL_8N1, 2, 4, false, 95, 11);


    memset(&ecuStats,0,sizeof(EcuStats));

    strcpy(currentWifiName,"");
    strcpy(currentWifiPassword,"");
    strcpy(ecuStats.camIp,"");

    if(!loadSettings()){
        //Serial.println("Restoring settings to default");
    }

    bool startAp = true;

    if (settings.wifiMode == WIFI_STA && strlen(settings.staName)>0 && strlen(settings.staPassword)>0){
        WiFi.mode(WIFI_STA);
        startAp = false;
        
        WiFi.begin(settings.staName, settings.staPassword);             // Connect to the network
        //Serial.print("Connecting to ");
        //Serial.print(settings.staName); Serial.println(": ");

        uint32_t connectionEndTime = millis() + settings.connectTimeout * 1000;

        while (WiFi.status() != WL_CONNECTED && millis() < connectionEndTime) { // Wait for the Wi-Fi to connect
            delay(1000);
            //Serial.print('.');
        }

        if (WiFi.status() == WL_CONNECTED){
            strcpy(currentWifiName,settings.staName);
            strcpy(currentWifiPassword,settings.staPassword);

            if (!MDNS.begin("rover")) {
                Serial.println("mDNS responder error!");
            }
            
            //Serial.println('\n');
            //Serial.println("Connection established!");  
            //Serial.print("IP address:\t");
            //Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
        }
        else{
            startAp = true;
        }

    }

    if (startAp && strlen(settings.apName)>0 && strlen(settings.apPassword)>0){
        WiFi.mode(WIFI_AP);
        boolean result = WiFi.softAP(settings.apName, settings.apPassword);
        if(result == true)
        {
            strcpy(currentWifiName,settings.apName);
            strcpy(currentWifiPassword,settings.apPassword);
            //Serial.println("WiFi AP Ready");
            //Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
        }
        else
        {
            //Serial.println("WiFi AP Failed!");
        }
    }

    LittleFS.begin();                           // Start the SPI Flash Files System
    
    server.on("/ecu", handleEcuCommand);
    server.on("/wifiSetup", handleWifiSetupCommand);
    server.on("/servoSetup", handleServoSetupCommand);
    server.on("/acc", handleAccCommand);
    server.on("/melody", handleMelodyCommand);

    server.onNotFound([]() {                              // If the client requests any URI
        if (!handleFileRead(server.uri()))                  // send it if it exists
        server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });

    server.begin();                           // Actually start the server

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        {
            type = "sketch";
        }
        else
        { // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
        {
            Serial.println("Auth Failed");
        }
        else if (error == OTA_BEGIN_ERROR)
        {
            Serial.println("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR)
        {
            Serial.println("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
            Serial.println("Receive Failed");
        }
        else if (error == OTA_END_ERROR)
        {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();

}

void loop()
{

#if SOFT_AP_ENABLED
    //Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
#endif

    server.handleClient();                    // Listen for HTTP requests from clients
    ArduinoOTA.handle();

    playNextTone();

    if ( massageInbound.parseStream( &Serial ) ) {

        if ( massageInbound.fullMatch ("stats") ) {
            ecuStats.time = massageInbound.nextLong();
            ecuStats.motorsMode = massageInbound.nextByte(); // motorsMode
            ecuStats.motorsSpeed[0] = massageInbound.nextInt();
            ecuStats.motorsSpeed[1] = massageInbound.nextInt();
            ecuStats.motorsSpeed[2] = massageInbound.nextInt();
            ecuStats.motorsSpeed[3] = massageInbound.nextInt();
            ecuStats.motorsSpeed[4] = massageInbound.nextInt();
            ecuStats.motorsSpeed[5] = massageInbound.nextInt();
            ecuStats.motorsProbe[0] = massageInbound.nextInt();
            ecuStats.motorsProbe[1] = massageInbound.nextInt();
            ecuStats.motorsProbe[2] = massageInbound.nextInt();
            ecuStats.motorsProbe[3] = massageInbound.nextInt();
            ecuStats.motorsProbe[4] = massageInbound.nextInt();
            ecuStats.motorsProbe[5] = massageInbound.nextInt();
            ecuStats.motorsPwm[0] = massageInbound.nextInt();
            ecuStats.motorsPwm[1] = massageInbound.nextInt();
            ecuStats.motorsPwm[2] = massageInbound.nextInt();
            ecuStats.motorsPwm[3] = massageInbound.nextInt();
            ecuStats.motorsPwm[4] = massageInbound.nextInt();
            ecuStats.motorsPwm[5] = massageInbound.nextInt();


            ecuStats.servosPwm[0] = massageInbound.nextInt();
            ecuStats.servosPwm[1] = massageInbound.nextInt();
            ecuStats.servosPwm[2] = massageInbound.nextInt();
            ecuStats.servosPwm[3] = massageInbound.nextInt();
            ecuStats.servosPwm[4] = massageInbound.nextInt();
            ecuStats.servosPwm[5] = massageInbound.nextInt();

            ecuStats.power = massageInbound.nextFloat();
        }

    }

    if ( massageInboundSw.parseStream( &swSer ) ) {

        if ( massageInboundSw.fullMatch ("gIP") ) {
            massageInboundSw.nextString(ecuStats.camIp,32);
        }

    }

    //for the firsr 30 seconds sent settings to the ECU every second
    uint32_t time = millis();
    if (time < 30000 && time > nextSendSettingsTime){
        sendSettingsToEcu();
        nextSendSettingsTime = time + 1000;
    }

    if (time < 120000 && time > nextSendWifiCredentialsTime){
        sendWifiCredentials();
        nextSendWifiCredentialsTime = time + 5000;
    }


}
