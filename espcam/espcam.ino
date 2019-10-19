/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com
  
  IMPORTANT!!! 
   - Select Board "ESP32 Wrover Module"
   - Select the Partion Scheme "Huge APP (3MB No OTA)
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "dl_lib.h"
#include "esp_system.h"

// Unique ID
char clientID[20];

// This project was tested with the AI Thinker Model, M5STACK PSRAM Model and M5STACK WITHOUT PSRAM
#define CAMERA_MODEL_AI_THINKER
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WITHOUT_PSRAM

// Not tested with this model
//#define CAMERA_MODEL_WROVER_KIT

#if defined(CAMERA_MODEL_WROVER_KIT)
  #define PWDN_GPIO_NUM    -1
  #define RESET_GPIO_NUM   -1
  #define XCLK_GPIO_NUM    21
  #define SIOD_GPIO_NUM    26
  #define SIOC_GPIO_NUM    27
  
  #define Y9_GPIO_NUM      35
  #define Y8_GPIO_NUM      34
  #define Y7_GPIO_NUM      39
  #define Y6_GPIO_NUM      36
  #define Y5_GPIO_NUM      19
  #define Y4_GPIO_NUM      18
  #define Y3_GPIO_NUM       5
  #define Y2_GPIO_NUM       4
  #define VSYNC_GPIO_NUM   25
  #define HREF_GPIO_NUM    23
  #define PCLK_GPIO_NUM    22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_M5STACK_WITHOUT_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       17
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Camera model not selected"
#endif

#define DEFAULT_UPLOAD_INTERVAL (10*60*1000)

int UploadInterval=DEFAULT_UPLOAD_INTERVAL;
unsigned long LastUpdated=0;


//Replace with your network credentials
const char* ssid = "ssid";
const char* password = "wifipass";
const char* imageServerHostname = "hostname";
int imageServerHttpPort=62100;
int imageServerControlPort=62101;

void PostImage()
{
  Serial.println("PostImage starting");
  
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  
  WiFiClient client;

  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    res = ESP_FAIL;
  } else {
    if(fb->width > 400){
      if(fb->format != PIXFORMAT_JPEG){
        bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
        esp_camera_fb_return(fb);
        fb = NULL;
        if(!jpeg_converted){
          Serial.println("JPEG compression failed");
          res = ESP_FAIL;
        }
      } else {
        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
      }
    }
  }
  if(res == ESP_OK){
    Serial.println("Connecting...");
    if (client.connect(imageServerHostname, imageServerHttpPort)) {
      Serial.println("Connected");
      client.println("POST /posts HTTP/1.1");
      client.print("Host: ");
      client.println(imageServerHostname);
      client.println("Cache-Control: no-cache");
      client.print("Client-ID: ");
      client.println(clientID);      
      client.println("Content-Type: image/jpeg");
      client.print("Content-Length: ");
      client.println(_jpg_buf_len);
      client.println();
      Serial.println("Sending...");
      client.write((const uint8_t *) _jpg_buf, _jpg_buf_len);
      
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 60000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
      }
    }
    else
     Serial.println("Connection failed!");
  }    
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
     Serial.println("Post done");
}

void ProcessCommand(char *cmd,int len)
{
  if(len<2)
  {
    Serial.println("Too short command");
    return;
  }
  cmd[len-1]=0;
  Serial.print("Command:");
  Serial.println(cmd);
  int i; 
  if(sscanf(cmd,"Interval=%d", &i)==1)
  {
    UploadInterval=i;
    LastUpdated=millis();  
    Serial.print("New interval:");
    Serial.println(UploadInterval);  
  }
}

WiFiClient *controlChannelClient=NULL;
#define CMD_BUF_LEN 100
char cmdBuf[CMD_BUF_LEN];
int cmdBufPtr=0;

void ProcessControlChannel()
{
  if(controlChannelClient==NULL)
    controlChannelClient=new WiFiClient();

  if(!controlChannelClient->connected())    
  {
    if (!controlChannelClient->connect(imageServerHostname, imageServerControlPort)) {
      delay(10000);
      return;
    }
    else
    {
      controlChannelClient->print("Client-ID: ");
      controlChannelClient->println(clientID);      
    }
  }
  if(controlChannelClient->available())
  {
    char ch=(char)controlChannelClient->read();
    cmdBuf[cmdBufPtr++]=ch;
    if(ch==0x0a)
    {
      ProcessCommand(cmdBuf,cmdBufPtr-1);
      cmdBufPtr=0;
    }
    if(cmdBufPtr>=CMD_BUF_LEN)
      cmdBufPtr=0;
  }

  //If server has not sent update interval lately, change to default.
  //Note that problem with this kind of code is that millis will roll over at some time.
  if(LastUpdated+60*1000<millis())
  {
    UploadInterval=DEFAULT_UPLOAD_INTERVAL;
    LastUpdated=millis();
  }
}



void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  Serial.print("My IP:");
  Serial.print(WiFi.localIP());
  Serial.println("");
  
  uint8_t chipid[6];
  esp_efuse_mac_get_default(chipid);
  sprintf(clientID,"%02x%02x%02x%02x%02x%02x",chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);
  Serial.print("My ID:");
  Serial.print(clientID);
  Serial.println("");
}

void loop() {
  for(int i=0;i<UploadInterval/32;i++)
  {
    ProcessControlChannel();
    delay(32);
  }
  
  if(WiFi.status() == WL_CONNECTED)
    PostImage();

  ProcessControlChannel();
}
