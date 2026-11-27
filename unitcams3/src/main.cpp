#include <Arduino.h>
#include <esp_camera.h>
#include <soc/usb_serial_jtag_reg.h>
#include <soc/soc.h>

// Unit CamS3 U174 / OV2640 (2MP), NOT the PY260 5MP variant.
HardwareSerial cameraUart(1);
bool ready=false;
uint32_t crc(uint32_t v,const uint8_t* p,size_t n) {
    for(size_t i=0;i<n;++i) {v^=p[i];for(int b=0;b<8;++b)v=(v>>1)^((v&1)?0xedb88320U:0);}
    return v;
}
void put32(uint8_t* p,uint32_t v) {for(int i=0;i<4;++i)p[i]=v>>(i*8);}
void capture(bool preview) {
    camera_fb_t* frame=nullptr;
    if(ready) {
        auto* sensor=esp_camera_sensor_get();
        sensor->set_framesize(sensor,preview?FRAMESIZE_QQVGA:FRAMESIZE_UXGA);
        sensor->set_quality(sensor,preview?20:12);
        // Discard queued frames from the previous resolution/exposure.
        for(int i=0;i<2;++i) {auto* old=esp_camera_fb_get();if(old)esp_camera_fb_return(old);}
        frame=esp_camera_fb_get();
    }
    uint8_t header[16]={'S','C','J','1',uint8_t(frame?0:1)};
    if(frame) {put32(header+8,frame->len);put32(header+12,crc(0xffffffffU,frame->buf,frame->len)^0xffffffffU);}
    cameraUart.write(header,sizeof(header));
    if(frame) {
        for(size_t offset=0;offset<frame->len;offset+=256) {
            cameraUart.write(frame->buf+offset,std::min(size_t(256),frame->len-offset));
            cameraUart.flush();delay(3); // Give the no-PSRAM Cardputer time to write SD.
        }
        esp_camera_fb_return(frame);
    }
}
void setup() {
    // Grove pins are also USB D-/D+. Release native USB before UART routing.
    REG_CLR_BIT(USB_SERIAL_JTAG_CONF0_REG,USB_SERIAL_JTAG_USB_PAD_ENABLE);
    cameraUart.begin(460800,SERIAL_8N1,19,20);
    camera_config_t c={};
    c.ledc_channel=LEDC_CHANNEL_0;c.ledc_timer=LEDC_TIMER_0;
    c.pin_d0=6;c.pin_d1=15;c.pin_d2=16;c.pin_d3=7;
    c.pin_d4=5;c.pin_d5=10;c.pin_d6=4;c.pin_d7=13;
    c.pin_xclk=11;c.pin_pclk=12;c.pin_vsync=42;c.pin_href=18;
    c.pin_sccb_sda=17;c.pin_sccb_scl=41;c.pin_pwdn=-1;c.pin_reset=21;
    c.xclk_freq_hz=20000000;c.pixel_format=PIXFORMAT_JPEG;
    c.frame_size=FRAMESIZE_UXGA;c.jpeg_quality=12;c.fb_count=1;
    c.fb_location=CAMERA_FB_IN_PSRAM;c.grab_mode=CAMERA_GRAB_WHEN_EMPTY;
    ready=psramFound() && esp_camera_init(&c)==ESP_OK;
    if(ready) {auto* s=esp_camera_sensor_get();if(s->id.PID!=OV2640_PID){esp_camera_deinit();ready=false;}}
}
void loop() {
    static size_t matched=0;
    constexpr char magic[]="SCQ1";
    while(cameraUart.available()) {
        uint8_t b=cameraUart.read();
        if(matched==4) {matched=0;if(b=='P'||b=='S')capture(b=='P');}
        else if(b==magic[matched]) ++matched;
        else matched=b=='S'?1:0;
    }
    delay(1);
}
