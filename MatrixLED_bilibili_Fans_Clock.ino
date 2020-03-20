/*
 * 代码修改：UP：Hans叫泽涵  bilibili UID:15481541
 * 感谢UP:MagicBear  bilibili UID:12
 * 
 * 本代码适用于ESP8266 NodeMCU + MAX7219 4*8*8 Matrix LED
 * 
 * 感谢以下作者开源的代码对我的帮助
 * https://github.com/magicbear/ESP32-BILIBILI-Fans
 * https://github.com/G6EJD/ESP8266-MAX7219-LED-4x8x8-Matrix-Clock
 * 
 */

/*************************连接方法*************************
 LED Matrix Pin -> ESP8266 Pin
 Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
 Gnd            -> Gnd (G on NodeMCU)
 DIN            -> D7  (Same Pin for WEMOS)
  CS            -> D4  (Same Pin for WEMOS)
 CLK            -> D5  (Same Pin for WEMOS)
*********************************************************/

#include <ArduinoJson.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <time.h>
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
/*********************修改此处""内的信息*********************/
#define UID "bilibili UID"    //输入你的B站UID
/*********************************************************/

int pinCS = D4; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
#define _DISPLAY_ROTATE 1

const byte buffer_size = 45;
char time_value[buffer_size];
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 70; // In milliseconds
int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels

/*********************修改此处""内的信息*********************/
const char *ssid      = "Your WiFi name";       //输入你的WiFi名称
const char *password  = "Your WiFi password";   //输入你的WiFi密码
/*********************************************************/

String tape = "123456";
int errorCode = 0;

// 3x5 char maps
uint16_t numMap3x5[] = {
    32319,  //0
    10209,  //1
    24253,  //2
    22207,  //3
    28831,  //4
    30391,  //5
    32439,  //6
    16927,  //7
    32447,  //8
    30399   //9
};
// 4x5 char maps
uint32_t numMap4x5[] = {
    476718, //0
    10209,  //1
    315049, //2
    579246, //3
    478178, //4
    972470, //5
    480418, //6
    544408, //7
    349866, //8
    415406  //9
};

char msg_buf[256];

HTTPClient http;

unsigned long api_mtbs = 2200; //mean time between api requests
unsigned long api_lasttime = 2200;   //last time api request has been done

#define PIXEL_SHOW HIGH
#define PIXEL_HIDE LOW

/***************************bilibili_Fans***********************************************/   
void _drawPixel(Max72xxPanel display, uint8_t x, uint8_t y, uint8_t pixel)
{
  #ifdef _DISPLAY_FLIP
  display.drawPixel(x, 7-y, pixel);
  #else
  display.drawPixel(x, y, pixel);
  #endif
}

void _drawRoundRect(Max72xxPanel display, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t pixel)
{
  #ifdef _DISPLAY_FLIP
  display.drawRoundRect(x, 7-(h+y -1), w, h, r, pixel);
  #else
  display.drawRoundRect(x, y, w, h, r, pixel);
  #endif
}

void _drawLine(Max72xxPanel display, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixel)
{
  #ifdef _DISPLAY_FLIP
  display.drawLine(x1, 7-y1, x2, 7-y2, pixel);
  #else
  display.drawLine(x1, y1, x2, y2, pixel);
  #endif
}

void drawLogo(Max72xxPanel display, int eye_offset)
{
    display.fillRect(0, 0, 8, 8, PIXEL_HIDE);
    _drawPixel(display, 1, 0, PIXEL_SHOW);
    _drawPixel(display, 2, 1, PIXEL_SHOW);
    _drawPixel(display, 6, 0, PIXEL_SHOW);
    _drawPixel(display, 5, 1, PIXEL_SHOW);
    _drawPixel(display, 2, 4 + eye_offset, PIXEL_SHOW);
    _drawPixel(display, 5, 4 + eye_offset, PIXEL_SHOW);
    _drawRoundRect(display, 0, 2, 8, 6, 1, PIXEL_SHOW);
}

void drawSplashtop(Max72xxPanel display)
{
  // b
  _drawLine(display, 10, 1 - 1, 10, 6 - 1, PIXEL_SHOW);
  _drawLine(display, 11, 3 - 1, 12, 3 - 1, PIXEL_SHOW);
  _drawLine(display, 11, 6 - 1, 12, 6 - 1, PIXEL_SHOW);
  _drawLine(display, 13, 4 - 1, 13, 5 - 1, PIXEL_SHOW);

  // i
  _drawLine(display, 15, 1 - 1, 15, 6 - 1, PIXEL_SHOW);
  _drawPixel(display, 15, 2 - 1, PIXEL_HIDE);

  // l
  _drawLine(display, 17, 1 - 1, 17, 6 - 1, PIXEL_SHOW);

  // i
  _drawLine(display, 19, 1 - 1, 19, 6 - 1, PIXEL_SHOW);
  _drawPixel(display, 19, 2 - 1, PIXEL_HIDE);

  // b
  _drawLine(display, 21, 1 - 1, 21, 6 - 1, PIXEL_SHOW);
  _drawLine(display, 22, 3 - 1, 23, 3 - 1, PIXEL_SHOW);
  _drawLine(display, 22, 6 - 1, 23, 6 - 1, PIXEL_SHOW);
  _drawLine(display, 24, 4 - 1, 24, 5 - 1, PIXEL_SHOW);

  // i
  _drawLine(display, 26, 1 - 1, 26, 6 - 1, PIXEL_SHOW);
  _drawPixel(display, 26, 2 - 1, PIXEL_HIDE);

  // l
  _drawLine(display, 28, 1 - 1, 28, 6 - 1, PIXEL_SHOW);

  // i
  _drawLine(display, 30, 1 - 1, 30, 6 - 1, PIXEL_SHOW);
  _drawPixel(display, 30, 2 - 1, PIXEL_HIDE);

  display.write();
}

void drawMapValue3x5(Max72xxPanel display, uint8_t x, uint8_t y, uint32_t val)
{
    for (uint8_t i = 0; i < 20; i++)
    {
        if ((val >> i) & 1 == 1) {
            display.drawPixel(x + (3 - i / 5) - 1, y + (4 - i % 5), HIGH);
        }
    }
}

void drawMapValue4x5(Max72xxPanel display, uint8_t x, uint8_t y, uint32_t val)
{
    for (uint8_t i = 0; i < 20; i++)
    {
        if ((val >> i) & 1 == 1) {
            display.drawPixel(x + (4 - i / 5) - 1, y + (4 - i % 5), HIGH);
        }
    }
}

long parseRelationAPI(String json)
{
    // 4 FOR BLOCK + 5 FOR DATA +
    const size_t json_size = JSON_OBJECT_SIZE(4 + 5) + 128;
    DynamicJsonDocument doc(json_size);
    deserializeJson(doc, json);

    int code = doc["code"];
    const char *message = doc["message"];

    if (code != 0) {
        Serial.print("[API] Code:");
        Serial.print(code);
        Serial.print(" Message:");
        Serial.println(message);
        errorCode = code;
        return -1;
    }

    JsonObject data = doc["data"];
    unsigned long data_mid = data["mid"];
    int data_follower = data["follower"];
    if (data_mid == 0) {
        Serial.println("[API] Cannot found valid output");
        errorCode = -2;
        return -2;
    }
    errorCode = 0;
    Serial.print("[API] UID: ");
    Serial.print(data_mid);
    Serial.print(" Follower: ");
    Serial.println(data_follower);

    return data_follower;
}

void updateFans()
{
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://api.bilibili.com/x/relation/stat?vmid=" UID)) {  // HTTPS
        Serial.print("[HTTPS] GET...\n");
        // start connection and send HTTP header
        int httpCode = https.GET();
        // httpCode will be negative on error
        if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK) {
                drawLogo(matrix, 0); matrix.write();
                String payload = https.getString();
                long fans = parseRelationAPI(payload);
                if (fans >= 0)
                {
                    tape = fans;
                } else if (fans == -1)
                {
                    tape = "API -1";
                } else if (fans == -2)
                {
                    tape = "API -2";
                }
                drawLogo(matrix, 1); matrix.write();
            } else 
            {
                errorCode = httpCode;
            }
        } else {
            errorCode = httpCode;
            tape = "HTTPS Error";
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }

        https.end();
    } else {
        tape = "Conn Error";
        Serial.printf("[HTTPS] Unable to connect\n");
    }
    drawLogo(matrix, 0); matrix.write();
}

/***************************************************************************************/    

void setup() {
  SPI.begin();
  SPI.setFrequency(10000000); // Here is 10Mhz
  matrix.setIntensity(5); // Set brightness between 0 and 15
  for (int i = 0; i< numberOfHorizontalDisplays;  i++)
      matrix.setRotation(i,_DISPLAY_ROTATE);
  matrix.fillScreen(PIXEL_HIDE);
  drawLogo(matrix, 0);
  drawSplashtop(matrix);
  WiFi.mode(WIFI_STA);
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  WiFi.begin(ssid,password);
  
  Serial.begin(115200);
  Serial.println();
  Serial.printf("Flash: %d\n", ESP.getFlashChipRealSize());
  Serial.print("Connecting");
  int processbar = 0;
  while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
      drawLogo(matrix, processbar % 2);
      matrix.drawPixel(8 + (processbar % 24), 7, (processbar / 24) % 2 == 0 ? HIGH : LOW);
      matrix.write();
      processbar++;
  }
      
   Serial.println();

   Serial.print("Connected to wifi. My address:");
   IPAddress myAddress = WiFi.localIP();
   Serial.println(myAddress);

   WiFi.setSleepMode(WIFI_LIGHT_SLEEP, 1000);
   matrix.fillScreen(PIXEL_HIDE);
   drawLogo(matrix, 0);
   drawSplashtop(matrix);
  
  configTime(0 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
  setenv("TZ", "CST-8",1);
  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1);    // The first display is position upside down
  matrix.setRotation(1, 1);    // The first display is position upside down
  matrix.setRotation(2, 1);    // The first display is position upside down
  matrix.setRotation(3, 1);    // The first display is position upside down
}

void loop() {
  matrix.fillScreen(LOW);
  String time = get_time();
  time.trim();
  Serial.println(time);
  time.substring(23,28).toCharArray(time_value, 10); 
  Serial.println("HH:MM");
  Serial.println(time_value);
  //( Sun  21-07-19 ) ( PM 12:52:12 )
  matrix.drawChar(2,0, time_value[0], HIGH,LOW,1); // H
  matrix.drawChar(8,0, time_value[1], HIGH,LOW,1); // HH
  matrix.drawChar(14,0,time_value[2], HIGH,LOW,1); // HH:
  matrix.drawChar(20,0,time_value[3], HIGH,LOW,1); // HH:M
  matrix.drawChar(26,0,time_value[4], HIGH,LOW,1); // HH:MM
  matrix.write(); // Send bitmap to display
  delay(3000);

/***************************bilibili_Fans***********************************************/   
  unsigned long duration = millis() - api_lasttime;
    if (duration >= api_mtbs)
    {
        api_lasttime = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi Disconnected");
            ESP.restart();
        }
        updateFans();
    }
    matrix.fillScreen(LOW);
    int x = 0;
    if (errorCode == 0)
    {
        drawLogo(matrix, 0);
        x = 9;

        Serial.println(duration);
        uint32_t waitBarLen = 22 - (duration / float(api_mtbs) * 20);
        if (waitBarLen > 22) waitBarLen = 22;
        for (uint8_t waitBar = 0; waitBar < waitBarLen; waitBar++)
        {
          _drawPixel(matrix, 9 + waitBar, 6, PIXEL_SHOW);
          _drawPixel(matrix, 9 + waitBar, 7, PIXEL_SHOW);
        }
    }
    for (int i=0; i<tape.length(); i++) {
        if (tape[i] >= '0' && tape[i] <= '9')
        {
            drawMapValue3x5(matrix, x, 0, numMap3x5[tape[i] - '0']);
            x += 4;
        } else if (tape[i] == '.') {
            matrix.drawPixel(x, 4, HIGH);
            x += 2;
        } else if (tape[i] == ' ') {
            x += 1;
        } else {
            matrix.drawChar(x, 0, tape[i], HIGH, LOW, 1);
            x+=5;
        }
    }
    matrix.write(); // Send bitmap to display
    delay(3000);
/***************************************************************************************/    
  
  display_message(time); // Display time in format 'Wed, Mar 01 16:03:20 2017
}

void display_message(String message){
   for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background off, reverse to invert the image
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait/2);
  }
}

String get_time(){
  time_t now;
  time(&now);
  char time_output[buffer_size];
  // See http://www.cplusplus.com/reference/ctime/strftime/ for strftime functions
  // Desired format: ( Sun ,Jul 21 2019 )   ( AM  10:03:20 ) 
  strftime(time_output, buffer_size, "( %a  %y-%m-%d ) ( %p %T )", localtime(&now)); 
  return String(time_output); // returns ( Sat 20-Apr-19) ( AM 12:31:45 )
}
