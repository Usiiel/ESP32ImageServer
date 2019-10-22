#include "esp_camera.h"


class Camera 
{ 
  public:   
    size_t JpgBufLen = 0;
    uint8_t * JpgBuf = NULL;
    esp_err_t Res = ESP_OK;
    bool Log=false;
    
    void capture(); 
    void release();
    
  private:  
    camera_fb_t * _fb = NULL;

}; 
