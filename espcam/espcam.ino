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

#include "settings.h"
#include <WiFi.h>
#include "Arduino.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "camera.h"

// Unique ID
char clientID[20];

int UploadInterval=DEFAULT_UPLOAD_INTERVAL;
unsigned long LastUpdated=0;

// These are defined in settings.h

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* imageServerHostname = IMAGE_SERVER_HOST;
int imageServerHttpPort= IMAGE_SERVER_HTTP_PORT;
int imageServerControlPort=IMAGE_SERVER_CONTROL_PORT;

void PostImage()
{
  Camera cam;
  WiFiClient client;
    
  Serial.println("PostImage starting");
  cam.capture();
  if(cam.Res == ESP_OK){
    Serial.println("Connecting...");
    if (client.connect(imageServerHostname, imageServerHttpPort)) {
      Serial.println("Connected");
      client.println("POST /upload HTTP/1.1");
      client.print("Host: ");
      client.println(imageServerHostname);
      client.println("Cache-Control: no-cache");
      client.print("Client-ID: ");
      client.println(clientID);
      client.print("API-Key: ");
      client.println(IMAGE_SERVER_API_KEY);
      client.println("Content-Type: image/jpeg");
      client.print("Content-Length: ");
      client.println(cam.JpgBufLen);
      client.println();
      Serial.println("Sending...");
      client.write((const uint8_t *) cam.JpgBuf, cam.JpgBufLen);
      
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
  cam.release();
  
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
      delay(COMMAND_CHANNEL_RETRY_INTERVAL);
      return;
    }
    else
    {
      controlChannelClient->print("Client-ID: ");
      controlChannelClient->println(clientID);
    }
  }
  while(controlChannelClient->available())
  {
    char ch=(char)controlChannelClient->read();
    cmdBuf[cmdBufPtr++]=ch;

    if(ch==0x0d)
	continue;

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

  Camera camera;
  camera.setup();
 
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  

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

#define LOOP_SKIP_RATE 64

void loop() {
  for(int i=0;i<UploadInterval;i+=LOOP_SKIP_RATE)
  {
    ProcessControlChannel();
    delay(LOOP_SKIP_RATE);
  }

  Serial.print(UploadInterval);
  
  if(WiFi.status() == WL_CONNECTED)
    PostImage();

  ProcessControlChannel();
}
