#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#include "JsonListener.h"
#include "WorldClockClient.h"
String timeZoneIds [] = {"America/New_York", "Europe/London", "Europe/Paris", "Australia/Sydney"};
WorldClockClient worldClockClient("de", "CH", "E, dd. MMMMM yyyy", 3, timeZoneIds);

#include "WundergroundClient.h"
bool changeMeteo=false;
const String  WUNDERGRROUND_API_KEY = "c88515956a30c5e9";
const boolean IS_METRIC = true;
const String  WUNDERGROUND_ZMW_CODE = "00000.123.07747";
const String WUNDERGRROUND_LANGUAGE = "FR";
String meteoDatas [3];
WundergroundClient wunderground(IS_METRIC);

#include <ArduinoJson.h>
#include "FS.h"

#define PINMATRIX 14

#define PINBTLEFT 5
#define PINBTCENTER 4
#define PINBTRIGHT 15

#define DEBUG  1

ESP8266WebServer server(80);


extern "C" {
#include "user_interface.h"
}

os_timer_t myTimer;
  
void youtube(void);
void hours(void);
void counter(void);
void meteo(void);
void rss(void);

String VERSION="1.01a";
const char* WIFISSIDValue;
const char* WIFIpassword;
String urlServer ;
const char* HTTPUser;
const char* HTTPPass ;
const char* jsonIp;
char ip1,ip2,ip3,ip4;
char mask1,mask2,mask3,mask4;
char gw1,gw2,gw3,gw4;

bool connexionOK=false;
bool configOK=false;

bool cmdComplete=false;
String SerialCMD;

int retryConnexionOk,retryWifi;
int countConnexion=0;
bool EndCheck=false;
bool ToReboot=false;
char wificonfig[512];
IPAddress ip,gateway,subnet;

uint8_t MAC_array[6];
char MAC_char[18];

bool debounce;

int x    = 8;
int pass = 0;
int i=0;
int brightness;
int brightnessAnalog;
int positionEcran=0;
int positionReel=0;
bool ChangePosition;
int tmpPosition;
int nbAppli=0;
int action=0;


// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)


Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32,8, PINMATRIX,
  NEO_MATRIX_TOP    + NEO_MATRIX_LEFT+
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(255, 210, 0),matrix.Color(0, 0, 255), matrix.Color(255, 0, 255), matrix.Color(0, 255, 255), matrix.Color(255, 255, 255),matrix.Color(91, 68, 43),matrix.Color(0, 0, 0)};


static unsigned char play[]={0x00,0x00,0x10,0x18,0x1c,0x18,0x10,0x00};
static unsigned char rond[]={0x00,0x7e,0xff,0xff,0xff,0xff,0xff,0x7e};
static unsigned char kk[]={0x04,0x00,0x18,0x3c,0x3c,0x7e,0x7e,0x00};
static unsigned char twitter[]={0x04,0x8f,0x6e,0x7e,0x7e,0x3c,0x38,0x30};
static unsigned char sun[]={0x24,0x00,0xbd,0x7e,0x7e,0x7e,0x00,0x00};
static unsigned char mask[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static unsigned char cloud[]={0x00,0x00,0x00,0x06,0x6f,0xef,0xff,0x7e};

int countRss=0;
String tabRss[2];

typedef void (*SimpleApplicationsList[])();
SimpleApplicationsList Applications = { hours, rss,meteo };
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

char serverIndex[512]="<h1>ESPMetric Config</h1><ul><li><a href='params'>Config ESPMetric</a></li><li><a href='update'>Flash ESPIOT</a></li></ul><br><br>Version: 1.01a<br><a href='https://github.com/fairecasoimeme/' target='_blank'>Documentation</a>";
char serverIndexUpdate[256] = "<h1>ESPMetric Config</h1><h2>Update Firmware</h2><form method='POST' action='/updateFile' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
char serverIndexConfig[1024] = "<h1>ESPMetric Config</h1><h2>Config ESPIOT</h2><form method='POST' action='/config'>SSID : <br><input type='text' name='ssid'><br>Pass : <br><input type='password' name='pass'><br>@IP : <br><input type='text' name='ip'><br>Mask : <br><input type='text' name='mask'><br>@GW : <br><input type='text' name='gw'><br>Http user : <br><input type='text' name='userhttp'><br>Http pass : <br><input type='text' name='passhttp'><br>URL : <br><input type='text' name='url'><br><input type='submit' value='OK'></form>";

const char* Style = "<style>body {  text-align: center; font-family:Arial, Tahoma;  background-color:#f0f0f0;}ul li { border:1px solid gray;  height:30px;  padding:3px;  list-style: none;}ul li a { text-decoration: none;}input{ border:1px solid gray;  height:25px;}input[type=text] { width:150px;  padding:5px;  font-size:10px;}#url {  width:300px;}</style>";

uint8_t sec=0;
uint8_t minute10=0;
bool minOccured;
bool min10Occured;
void timerCallback(void *pArg) {
      sec++;
      if (sec==60)
      {
        minute10++;
        minOccured = true;
        sec=0;
      }
      if (minute10==10)
      {
        min10Occured=true;
        minute10=0;
      }
}

void user_init(void) {

      os_timer_setfn(&myTimer, timerCallback, NULL);
      os_timer_arm(&myTimer, 1000, true);
} 



/* FONCTION POUR CHARGER LA CONFIG
 *  
 *  
 */
bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    if (DEBUG)
      Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    if (DEBUG)
      Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    if (DEBUG)
      Serial.println("Failed to parse config file");
    return false;
  }
  
  
  if (json.containsKey("WIFISSID")) 
  {
    WIFISSIDValue = json["WIFISSID"];
  }
  if (json.containsKey("WIFIpass")) 
  {
    WIFIpassword = json["WIFIpass"];
  }
  if (json.containsKey("Ip")) 
  {
    jsonIp = json["Ip"];
    ip.fromString(jsonIp);
  }
  if (json.containsKey("Mask")) 
  {
    jsonIp = json["Mask"];
    subnet.fromString(jsonIp);
  }
  if (json.containsKey("GW")) 
  {
    jsonIp = json["GW"];
    gateway.fromString(jsonIp);
  }
   if (json.containsKey("urlServer")) 
  {
     urlServer = json["urlServer"].asString();
  }
  if (json.containsKey("HTTPUser")) 
  {
     HTTPUser = json["HTTPUser"].asString();
  }
  if (json.containsKey("HTTPPass")) 
  {
     HTTPPass = json["HTTPPass"].asString();
  }

  uint8_t tmp[6];
  char tmpMAC[18];
  WiFi.macAddress(tmp);
  for (int i = 0; i < sizeof(tmp); ++i){
    sprintf(tmpMAC,"%s%02x",tmpMAC,tmp[i]);
  }
  json["ID"] =  String(tmpMAC);
  json.printTo(wificonfig,sizeof(wificonfig));
  //sprintf(serverIndexConfig,"<h1>ESPIOT Config</h1><h2>Config ESPIOT : %s</h2><form method='POST' action='/config'>SSID : <br><input type='text' name='ssid' value='%s'><br>Pass : <br><input type='password' name='pass'><br>@IP : <br><input type='text' name='ip' value='%s'><br>Mask : <br><input type='text' name='mask' value='%s'><br>@GW : <br><input type='text' name='gw' value='%s'><br>Http user : <br><input type='text' name='userhttp' value='%s'><br>Http pass : <br><input type='text' name='passhttp' value='%s'><br>URL : <br><input type='text' name='url' value='%s'><br><input type='submit' value='OK'></form>",json["ID"].asString(),json["WIFISSID"].asString(),json["Ip"].asString(),json["Mask"].asString(),json["GW"].asString(),json["HTTPUser"].asString(),json["HTTPPass"].asString(),json["urlServer"].asString());
 sprintf(serverIndexConfig,"<html><head></head><body><h1>ESPIOT Config</h1><h2>@MAC : %s</h2><form method='POST' action='/config'>SSID : <br><input type='text' name='ssid' value='%s'><br>Pass : <br><input type='password' name='pass'><br>@IP : <br><input type='text' name='ip' value='%s'><br>Mask : <br><input type='text' name='mask' value='%s'><br>@GW : <br><input type='text' name='gw' value='%s'><br>URL : <br><input type='text' name='url' value='%s' style='width:300px;'><br>Http user : <br><input type='text' name='userhttp' value='%s'><br>Http pass : <br><input type='text' name='passhttp' value='%s'><br><br><input type='submit' value='OK'></form></body></html>",json["ID"].asString(),json["WIFISSID"].asString(),json["Ip"].asString(),json["Mask"].asString(),json["GW"].asString(),json["urlServer"].asString(),json["HTTPUser"].asString(),json["HTTPPass"].asString());

  return true;
}

void nextAppli()
{
  // add one to the current pattern number, and wrap around at the end
  nbAppli = (nbAppli + 1) % ARRAY_SIZE( Applications);
}

void prevAppli()
{
  // add one to the current pattern number, and wrap around at the end
  nbAppli = (nbAppli - 1) % ARRAY_SIZE( Applications);
}

void leftButton()
{
  if (!debounce)
  {
     debounce=true;
     action=2;
     ChangePosition=true;
  }
  
}

void centerButton()
{
  if (!debounce)
  {
     debounce=true;
     action=1;
     changeMeteo=true;
  }
}

void rightButton()
{
   if (!debounce)
   {
     debounce=true;
     action=0;
     ChangePosition=true;
   }
   
}

void wifiWait()
{
  matrix.fillScreen(0);
  matrix.setTextColor(colors[6]);
  matrix.setCursor(6, 0);
  matrix.print(F("WIFI"));
  matrix.show();
}

void setup() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(100);
  matrix.setTextColor(colors[0]);

  pinMode(PINBTLEFT,INPUT_PULLUP);
  pinMode(PINBTCENTER,INPUT_PULLUP);
  pinMode(PINBTRIGHT,INPUT);

  if (!SPIFFS.begin()) {
      if (DEBUG)
        Serial.println("Failed to mount file system");
      return;
    }

    if (!loadConfig()) {
      if (DEBUG)
        Serial.println("Failed to load config");
    } else {
      configOK=true;
      if (DEBUG)
        Serial.println("Config loaded");
    }

    
  if (configOK)
  {
     
     wifiWait();
     WiFi.mode(WIFI_STA);
    
     WiFi.begin(WIFISSIDValue, WIFIpassword);
     
      WiFi.macAddress(MAC_array);
      for (int i = 0; i < sizeof(MAC_array); ++i){
        sprintf(MAC_char,"%s%02x:",MAC_char,MAC_array[i]);
      }
      if (DEBUG)
        Serial.println(WiFi.localIP());    
      
    }else{
      setupWifiAP();
      if (DEBUG)
        Serial.println("Config not ok");
      
   }
   retryWifi=0;  
   retryConnexionOk=0;
   countConnexion=0;
   EndCheck=false;
   ToReboot=false;

  
  attachInterrupt(PINBTLEFT, leftButton, CHANGE); 
  attachInterrupt(PINBTCENTER, centerButton, CHANGE); 
  attachInterrupt(PINBTRIGHT, rightButton, RISING); 
  Serial.begin(9600);

   user_init();
 
}



void loop() {
  server.handleClient();
  brightnessAnalog = analogRead(0);
  if (minOccured == true)
  {
    minOccured = false;
    ntpRequest();
    action=0;
    ChangePosition=true;
  }
  if (min10Occured == true)
  {
    min10Occured=false;
    meteoUpdate();
  }
  
  if (!EndCheck)
   {
      if (WiFi.status() != WL_CONNECTED) 
      {
       if (DEBUG)
        Serial.println(WiFi.status()); 
        countConnexion=0;
        if (retryWifi <40)
        {
          delay(500);
          if (DEBUG)
            Serial.print(".");
          retryWifi++;
        }else{
          EndCheck=true;
          setupWifiAP();
          serverWebCfg();
          if (DEBUG)
            Serial.println("délai dépassé");
        }
      }else{
        connexionOK=true;
        EndCheck=true;
        rssUpdate();
        ntpRequest();
        meteoUpdate();
        if (DEBUG)
          Serial.println("Connexion OK"); 
      }
   }else{
      if (brightnessAnalog<200)
      {
        brightness=10;
      }else if(brightnessAnalog<400)
      {
        brightness=20;
      }else if(brightnessAnalog>400)
      {
        brightness=30;
      }
      
      
      //matrix.setCursor(x, 0);
      matrix.setBrightness(brightness); 
    
    
      if (ChangePosition)
      {
        ChangePosition=false;
        if (action==0) //vers le haut
        {
           
            uint8_t stepCount;
             for (stepCount=0;stepCount<=8;stepCount++)
             {
                matrix.fillScreen(0);
                positionReel--;
                Applications[nbAppli]();
                matrix.show();
                delay(40);
             }
             nextAppli();
             positionReel=8;
             positionEcran=0;
             debounce=false;
               
           
        }else if (action==2) //vers le bas
        {
             uint8_t stepCount;
             for (stepCount=0;stepCount<=8;stepCount++)
             {
                matrix.fillScreen(0);
                positionReel++;
                Applications[nbAppli]();
                matrix.show();
                delay(40);
             }
             prevAppli();
             positionReel=-8;
             positionEcran=0;
             debounce=false;
        }
        
      }
      
      tmpPosition=positionReel - positionEcran;
      if (tmpPosition<0)
      {
        positionReel++;
        
      }else if (tmpPosition>0){
         positionReel--;
      }
    
      matrix.fillScreen(0);
      
      Applications[nbAppli]();

      matrix.show();
      delay(40);
   }
}




void youtube()
{
  matrix.setTextColor(colors[0]);
  matrix.setCursor(8, positionReel);
  matrix.print(F("test"));
  matrix.drawBitmap(0, positionReel,  rond, 8,8,colors[0]);
  matrix.drawBitmap(0, positionReel,  play, 8,8,colors[6]);
  
}


void meteoUpdate()
{
  wunderground.updateConditions(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_ZMW_CODE);
  meteoDatas [0] = wunderground.getCurrentTemp();
  meteoDatas [1] =wunderground.getHumidity(); 
  meteoDatas [2] = wunderground.getPressure();
}

void ntpRequest()
{
  worldClockClient.updateTime();
}

void rssUpdate()
{
  WiFiClient client;
  if (!client.connect("www.lemonde.fr", 80)) {
    Serial.println("connection failed");
    return;
  }

  client.print("GET /rss/une.xml HTTP/1.1\r\nHost: www.lemonde.fr\r\nConnection: close\r\n\r\n");
  
  int retryCounter = 0;
  while(!client.available()) {
    Serial.println(".");
    delay(1000);
    retryCounter++;
    if (retryCounter > 10) {
      return;
    }
  }

  int pos = 0;
  boolean isCmd = false;
  boolean cmdOk=false;
  boolean isTitle=false;
  char c;
  String cmd="";
  int size = 0;
  client.setNoDelay(false);
  while(client.connected()) {
    while((size = client.available()) > 0) {
      c = client.read();

      if (isCmd)
      {
        cmd=cmd+c;
      }
      
      if (c == '<') {
        isCmd = true;
        cmd="";
        
      }else if (c == '>'){
        isCmd=false;
        cmdOk=true;
      }
     
      if (cmdOk)
      {
        cmdOk=false;
        if (cmd=="title>")
        {
         
          isTitle=true;
        }
      }
       
      if (isTitle)
      {
        if (c!='<')
        {
          tabRss[countRss]= tabRss[countRss]+ c;
          if (countRss==1)
          {
            break;
          }
        }else{
          countRss=1;
          isTitle=false;
          
        }
      }
      
    }
  }
  
}

int tmpRssX=0;
void rss()
{
  matrix.setTextColor(colors[6]);
  matrix.setCursor((tmpRssX+32), positionReel);
  matrix.print(tabRss[1]);

  tmpRssX--;
  if ((tabRss[1].length() + tmpRssX)==0)
  {
    tmpRssX=0;
  }
}

void hours()
{
  
  matrix.setTextColor(colors[6]);
  matrix.setCursor(1, positionReel);
  matrix.print(worldClockClient.getHours(2) + ":" + worldClockClient.getMinutes(2));
}

void counter()
{
  matrix.setTextColor(colors[7]);
  matrix.setCursor(8, positionReel);
  matrix.print(String(i));
  matrix.drawBitmap(0, positionReel,  kk, 8,8,colors[7]);
  
  i++; 
}


int tmpX=9;
int defilement=0;
int meteoCount=0;
void meteo()
{
  
  matrix.setTextColor(colors[6]);
  matrix.setCursor(tmpX, positionReel);
  matrix.print(meteoDatas[meteoCount]);
  if ((defilement>50) && (defilement<1000))
  {
    if (changeMeteo)
    {
      changeMeteo=false;
       debounce=false;
      tmpX=41;
      defilement=50;
      meteoCount = (meteoCount + 1) % ARRAY_SIZE( meteoDatas);
    }else{
      if (tmpX==8)
      {
        defilement=25;
        changeMeteo=true;
        delay(2000);
      }else{
        tmpX--;
      }
    }
    
  }else 
  {
    if (changeMeteo)
    {
       tmpX--;
    }
    
  }
   //GESTION DES ICONES

  if ((wunderground.getMeteoconIcon(wunderground.getTodayIconText())=="H")|| (wunderground.getMeteoconIcon(wunderground.getTodayIconText())=="J"))
  {
    matrix.drawBitmap(0, positionReel,  mask, 8,8,colors[8]);
    matrix.drawBitmap(0, positionReel,  sun, 8,8,colors[2]);
    matrix.drawBitmap(0, positionReel,  cloud, 8,8,colors[6]);
  }else if (wunderground.getMeteoconIcon(wunderground.getTodayIconText())=="B")
  {
    matrix.drawBitmap(0, positionReel,  mask, 8,8,colors[8]);
    matrix.drawBitmap(0, positionReel,  sun, 8,8,colors[2]);
  }else if ((wunderground.getMeteoconIcon(wunderground.getTodayIconText())=="Y") || (wunderground.getMeteoconIcon(wunderground.getTodayIconText())=="M"))
  {
    matrix.drawBitmap(0, positionReel,  mask, 8,8,colors[8]);
    matrix.drawBitmap(0, positionReel,  cloud, 8,8,colors[6]);
  }
  defilement++;
  delay(20);
  
}



void setupWifiAP()
{
  WiFi.mode(WIFI_AP);

  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ESPIOT-" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  String WIFIPASSSTR = "admin"+macID;
  char WIFIPASS[WIFIPASSSTR.length()+1];
  memset(WIFIPASS,0,WIFIPASSSTR.length()+1);
  for (int i=0; i<WIFIPASSSTR.length(); i++)
    WIFIPASS[i] = WIFIPASSSTR.charAt(i);

  WiFi.softAP(AP_NameChar,WIFIPASS );
}

void serverWebCfg()
{
  char* host = "espiot";
   if (DEBUG)
    Serial.println("HTTP server started");
    MDNS.begin(host);
    server.on("/", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
       server.send(200, "text/html", strcat(serverIndex,Style));
    });
    server.on("/update", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", strcat(serverIndexUpdate,Style));
    });
    server.on("/params", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", strcat(serverIndexConfig,Style));
    });
     server.on("/config", HTTP_POST, [](){
      
       String StringConfig;
       String ssid=server.arg("ssid");
       String pass=server.arg("pass");
       String ip=server.arg("ip");

       String mask=server.arg("mask");

       String gw=server.arg("gw");

       String userhttp=server.arg("userhttp");
       String passhttp=server.arg("passhttp");
       String url=server.arg("url");
       uint8_t tmp[6];
       char tmpMAC[18];
       WiFi.macAddress(tmp);
       for (int i = 1; i < sizeof(tmp); ++i){
        sprintf(tmpMAC,"%s%02x:",tmpMAC,tmp[i]);
       }

       StringConfig = "{\"ID\":\""+String(tmpMAC)+"\",\"WIFISSID\":\""+ssid+"\",\"WIFIpass\":\""+pass+"\",\"Ip\":\""+ip+"\",\"Mask\":\""+mask+"\",\"GW\":\""+gw+"\",\"urlServer\":\""+url+"\",\"HTTPUser\":\""+userhttp+"\",\"HTTPPass\":\""+passhttp+"\"}";       
       
       StaticJsonBuffer<512> jsonBuffer;
       JsonObject& json = jsonBuffer.parseObject(StringConfig);
       File configFile = SPIFFS.open("/config.json", "w");
       if (!configFile) {
         if (DEBUG)
          Serial.println("Failed to open config file for writing");
       }else{
         json.printTo(configFile);
         //delay(6000);
         //ToReboot=true;
      }
      
      server.send(200, "text/plain", "OK");
    });
   
    server.on("/updateFile", HTTP_POST, [](){
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart();
    },[](){
      HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
        WiFiUDP::stopAll();
         if (DEBUG)
          Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        
      }
      yield();
    });
    server.begin();
    MDNS.addService("http", "tcp", 80);

}

