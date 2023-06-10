#include <Arduino.h>

#include <SPI.h>
#include <TFT_eSPI.h>
#include <WS2812FX.h>
#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "Free_Fonts.h"
#include <BluetoothA2DPSink.h>
#include <BlynkSimpleEsp32.h>

//App vars
#define BLYNK_TEMPLATE_ID   "TMPL2rIHRzfVw"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN    "c6A6llVGDgG2EpUOALlMab_aCYOCUINw"

//Wifi vars
const char* ssid2 = "Kelton";
const char* pass2 = "keltonpass";

String ssid;
String pass;


//Time vars
const char* ntpserver = "pool.ntp.org";
const long offset = -28800;
const int dayoffset = 3600;

//Weather vars
String apikey = "f303ab772ee4bc62e062c647ef9e38e0";
String city = "Corvallis";
String country = "US";

//LCD vars
TFT_eSPI lcd = TFT_eSPI();
String curled = "Blink";
int curvol = 0;
int curscrn;

int maxh;
int minh;
int curh = 1;

int bright = 100;
int r = 0x00FF00;
int g = 0x000000;
int b = 0x000000;

//Bluetooth vars
BluetoothA2DPSink spkr;

//LED vars
#define LED_COUNT 17

WS2812FX leds[4] = {
 WS2812FX(LED_COUNT, 13, NEO_RGB + NEO_KHZ800),
 WS2812FX(LED_COUNT, 12, NEO_RGB + NEO_KHZ800),
 WS2812FX(LED_COUNT, 14, NEO_RGB + NEO_KHZ800),
 WS2812FX(LED_COUNT, 27, NEO_RGB + NEO_KHZ800)};

//Rotary Encoder vars
#define cw 35
#define ccw 34
#define push 32

int curstate;
int laststate;
unsigned long lastpress = 0;
int laststart;

//App button functions. Changes in the corresponding pins call the function
BLYNK_WRITE(V0)
{
  int value = param.asInt();
  select(2, value);
}

BLYNK_WRITE(V1)
{
  int value = param.asInt();
  select(2, value);
}

BLYNK_WRITE(V2)
{
  int value = param.asInt();
  select(2, value);
}

BLYNK_WRITE(V3)
{
  int value = param.asInt();
  select(2, value);
}

BLYNK_WRITE(V5)
{
  int value = param.asInt();
  select(2, value);
}

BLYNK_WRITE(V6)
{
  int value = param.asInt();
  select(2, value);
}

BLYNK_WRITE(V4)
{
  bright = param.asInt();
  for(int i = 0; i < 4; i++){
    leds[i].setBrightness(bright);
  }
}

// Change http response later to be useful without a serial monitor
String httpGETRequest(const char* serverName){
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  
  // Free resources
  http.end();

  return payload;
}

//Makes an api call to openweathermap
JSONVar getweather(){
  String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + country + "&APPID=" + apikey + "&units=imperial";
  String jbuffer = httpGETRequest(serverPath.c_str());
  //Serial.print(jbuffer);
  return JSON.parse(jbuffer);
}

//Creates the start menu screen
void LCD_start(){
  int xpos = 0;
  int ypos = 20;

  curscrn = 1;
  
  maxh = 128;
  minh = 157;
  curh = 128;
  
  JSONVar weather = getweather();
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  //Fills the screen blue then sets the text to black
  lcd.fillScreen(TFT_BLUE);
  lcd.setTextColor(TFT_BLACK);
  lcd.setCursor(xpos, ypos);

  lcd.setFreeFont(FSB12);

  //Uses the lcd print function to display lcd requirement information
  lcd.println(&timeinfo, "   Date: %D Time: %I:%M");
  lcd.print("   W: ");
  lcd.print(weather["weather"][0]["main"]);
  lcd.print(" T: ");
  lcd.print(weather["main"]["temp"]);
  lcd.println("F");

  lcd.print("   LED Mode: ");
  lcd.println(curled);

  lcd.print("   Current volume: ");
  lcd.println(curvol);

  lcd.println("   1. LED Modes");
  lcd.println("   2. Settings");
  
  lcd.fillCircle(6, maxh, 3, TFT_BLACK);
}

//Moves the circle cursor up one position per turn
void moveup(){
  //If the cursor isnt at max postion fill the previous position blue
  //then update cursor position by 1 and reprint the black circle
  if(curh != maxh){
    lcd.fillCircle(6, curh, 3, TFT_BLUE);
    curh = curh - 29;
    lcd.fillCircle(6, curh, 3, TFT_BLACK);
  }
}

void movedown(){
  //Same process as moveup but now going downwards
  if(curh != minh){
    lcd.fillCircle(6, curh, 3, TFT_BLUE);
    curh = curh + 29;
    lcd.fillCircle(6, curh, 3, TFT_BLACK);
  }
}

//This function allows the user to make a selection when they press the push button
//It works by first checking the current screen then checking current position
//Then whatever function is called for that position
void select(int curs, int curhe){
  switch(curs){
    //Start screen
    case 1:
      switch(curhe){
        case 128:
          ledscrn();
          break;

        case 157:
          setscrn();
          break;
      }
    break;
    //Led mode change screen
    case 2:
      switch(curhe){
        case 12:
            LCD_start();
          break;

        case 41:
            for(int i = 0; i < 4; i++){
              leds[i].setMode(1);
            }
            curled = "Blink";
          break;

        case 70:
            for(int i = 0; i < 4; i++){
              leds[i].setMode(2);
            }
            curled = "Breath";
          break;

        case 99:
            for(int i = 0; i < 4; i++){
              leds[i].setMode(12);
            }
            curled = "Rainbow";
          break;

        case 128:
            for(int i = 0; i < 4; i++){
              leds[i].setMode(10);
            }
            curled = "Dynamic";
          break;

        case 157:
            for(int i = 0; i < 4; i++){
              leds[i].setMode(19);
            }
            curled = "Twinkle";
          break;

        case 186:
            for(int i = 0; i < 4; i++){
              leds[i].setMode(12);
            }
            audioresp();
          break;
        default:
          break;
      }
    break;

    //settings screen
    case 3:
      switch(curhe){
        case 12:
          LCD_start();
          break;
        //Change Brightness
        case 41:
          bright = changeval(bright);

          for(int i = 0; i < 4; i++){
           leds[i].setBrightness(bright);
          }
          delay(200);
          break;
        //Change Red value
        case 70:
          r = changeval(r/65536)*65536;
          for(int i = 0; i < 4; i++){
           leds[i].setColor(r + g + b);
          }
          delay(200);
          break;
      
        //Change Green value
        case 99:
          g = changeval(g/256)*256;
          for(int i = 0; i < 4; i++){
           leds[i].setColor(r + g + b);
          }
          delay(200);
          break;
      
        //Change Blue value
        case 128:
          b = changeval(b);
          for(int i = 0; i < 4; i++){
           leds[i].setColor(r + g + b);
          }
          delay(200);
          break;
    }
  break;
  }
}

void ledscrn(){
  int xpos = 0;
  int ypos = 20;

  curscrn = 2;
  
  maxh = 12;
  minh = 186;
  curh = 12;

  lcd.fillScreen(TFT_BLUE);
  lcd.setTextColor(TFT_BLACK);
  lcd.setCursor(xpos, ypos);

  lcd.setFreeFont(FSB12);

  lcd.println("   1. Back");
  lcd.println("   2. Blink");
  lcd.println("   3. Breath");
  //using rainbow cycle
  lcd.println("   4. Rainbow");
  //using multi dynamic for this
  lcd.println("   5. Dynamic");
  lcd.println("   6. Twinkle");
  lcd.println("   7. Audio Responsive");

  lcd.fillCircle(6, curh, 3, TFT_BLACK);
}

void audioresp(){
  //First disconnect the wifi to not interfer with the bluetooth
  Blynk.disconnect();
  bool connect_print = true;
  long minlvl = 0;
  long maxlvl = 0;
  long avglvl = 0;
  long sampleavg= 0;
  long thres = 10;
  unsigned long timer = 0;

  int xpos = 0;
  int ypos = 20;

  curscrn = 3;
  
  maxh = 12;
  minh = 12;
  curh = 12;

  lcd.fillScreen(TFT_BLUE);
  lcd.setTextColor(TFT_BLACK);
  lcd.setCursor(xpos, ypos);

  lcd.setFreeFont(FSB12);

  lcd.println("   1. Back");

  lcd.fillCircle(6, curh, 3, TFT_BLACK);

  spkr.start("Infinity Cube");

  lcd.println("   Bluetooth started");
  lcd.println("   WiFi will be disabled");
  lcd.println("   Connect to Infinity Cube");

  //Get a baseline reading for the noise to avoid flickering
  for(int i = 0; i < 20; i++){
    avglvl =+ abs(analogRead(39)-analogRead(36));
    delay(25);
  }
  avglvl /= 20;

  //Play until the user presses the push button
  while(digitalRead(push) == HIGH){
    //Once the device is connected print connected
    if(spkr.is_connected() == true && connect_print == true){
      lcd.print("   Device connected");
      connect_print = false;
    }

    //By taking an analogRead of the + and - terminals of the speaker
    //we can get the audio waveform and use its magnitude for a general volume
    int sample = abs(analogRead(39) - analogRead(36));

    //Only updates brightness every 200ms to avoid flickering
    if(millis() > timer){
      //If the sample is above the average level change brightness
      //Otherwise set to minimum
      if(sample > avglvl){
        if(sample > maxlvl){
          maxlvl = sample;
        }
        //Using map to figure out how bright it should be
        bright = map(sample, avglvl, maxlvl, 1, 255);
        for(int i = 0; i < 4; i++){
          leds[i].setBrightness(bright);
        }
      }
      else{
        for(int i = 0; i < 4; i++){
          leds[i].setBrightness(1);
        }
      }
    sampleavg = 0;
    timer = millis() + 200;
    }

    for(int i = 0; i < 4; i++){
      leds[i].service();
    } 
  }

  //End a2dp sink to allow us to reconnect to wifi
  //This method is imperfect because it prevents reconnections
  spkr.end(true);

  bright = 100;
  for(int i = 0; i < 4; i++){
    leds[i].setBrightness(bright);
  }
  
  Blynk.connect();
  ledscrn();
}

void setscrn(){
  int xpos = 0;
  int ypos = 20;

  curscrn = 3;
  
  maxh = 12;
  minh = 128;
  curh = 12;

  lcd.fillScreen(TFT_BLUE);
  lcd.setTextColor(TFT_BLACK);
  lcd.setCursor(xpos, ypos);

  lcd.setFreeFont(FSB12);

  lcd.println("   1. Back");
  lcd.print("   2. Change Brightness   :");
  lcd.println(bright);
  lcd.print("   3. Change Red Value    :");
  lcd.println(r/65536);
  lcd.print("   4. Change Green Value:");
  lcd.println(g/256);
  lcd.print("   5. Change Blue Value   :");
  lcd.println(b);

  lcd.fillCircle(6, curh, 3, TFT_BLACK);
}

void wificonnect(){
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid2, pass2);
  Serial.println("Connecting...");

  delay(500);

  if(Blynk.connected() != true){
    WiFi.mode(WIFI_AP_STA);
    WiFi.beginSmartConfig();
    while(!WiFi.smartConfigDone()){
    delay(500);
    Serial.print(".");
  }

  ssid = WiFi.SSID();
  pass = WiFi.psk();

  const char* ssid3 = ssid.c_str();
  const char* pass3 = pass.c_str();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid3, pass3);
  }

  while(Blynk.connected() != true){
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
}

int changeval(int val){
  delay(200);
  while(digitalRead(push) == HIGH){
      curstate = digitalRead(cw);
      if(curstate != laststate && curstate == 1){
        if(digitalRead(ccw) != curstate && val > 1){
          val--;
          }
          else if(val < 255){
          val++;
          }
        lcd.fillRect(265, curh - 11, 40, 20, TFT_BLUE);
        lcd.setCursor(265, curh + 8);
        lcd.setTextColor(TFT_BLACK);
        lcd.setFreeFont(FSB12);
        lcd.print(val);
        delay(5);
    }
    laststate = curstate;
  }
  return val; 
}
void setup() {
  Serial.begin(115200);
  //set up wifi connection
  wificonnect();
  
  //set up time
  configTime(offset, dayoffset, ntpserver);

  //set up i2s
  i2s_pin_config_t my_pin_config = {
    .bck_io_num = 25,
    .ws_io_num = 33,
    .data_out_num = 26,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  static i2s_config_t i2s_config = {
    .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100, // updated automatically by A2DP
    .bits_per_sample = (i2s_bits_per_sample_t)32,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 1, // default interrupt priority
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = true,
    .tx_desc_auto_clear = true // avoiding noise in case of data unavailability
  };
  
  spkr.set_i2s_config(i2s_config);

  spkr.set_pin_config(my_pin_config);

  //set up lcd
  lcd.begin();
  lcd.setRotation(3);
  LCD_start();

  //set up LEDs
  for(int i = 0; i < 4; i++){
    leds[i].init();
    leds[i].setBrightness(100);
    leds[i].setSpeed(1000);
    leds[i].setColor(0x00FF00);
    leds[i].setMode(FX_MODE_STATIC);
    leds[i].start();
  }

  //set up rotary encoder
  pinMode(cw,INPUT);
  pinMode(ccw,INPUT);
  pinMode(push, INPUT);
  pinMode(39, INPUT);

  laststate = digitalRead(cw);
}

void loop() {
  Blynk.run();
  for(int i = 0; i < 4; i++){
    leds[i].service();
  }

  // variable to make sure start screen stays up to date
  laststart = millis();
  //Read current state of rotary encoder
  curstate = digitalRead(cw);

  //If the current state is different than the last state
  //and it equals one then it must be rotating
  if(curstate != laststate && curstate == 1){
    if(digitalRead(ccw) != curstate){
      movedown();
    }
    else{
      moveup();
    }
  }

  laststate = curstate;

  //Constantly reading for push button input
  if(digitalRead(push) == LOW ){
    //debounce
    if(millis() - lastpress > 50){
     select(curscrn, curh);
    }
    lastpress = millis();
  }

  if(millis() > laststart + 30000 && curscrn == 1){
    LCD_start();
  }
  delay(1);
}
