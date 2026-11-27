#pragma once
#include <Arduino.h>
#include <SD.h>
#include <memory>

// SCQ1 request: magic + P (preview) / S (still).
// SCJ1 reply: magic, status, 3 reserved bytes, LE32 size, LE32 CRC32, JPEG.
namespace unitcam {
constexpr size_t PREVIEW_MAX = 32768;
inline HardwareSerial& port() { static HardwareSerial uart(1); return uart; }
inline uint32_t crc(uint32_t value, const uint8_t* data, size_t n) {
    for (size_t i=0;i<n;++i) { value ^= data[i]; for(int b=0;b<8;++b) value=(value>>1) ^ ((value&1)?0xedb88320U:0); }
    return value;
}
inline uint32_t le32(const uint8_t* p) { return uint32_t(p[0]) | uint32_t(p[1])<<8 | uint32_t(p[2])<<16 | uint32_t(p[3])<<24; }
inline void begin() { port().setRxBufferSize(8192); port().begin(460800, SERIAL_8N1, 2, 1); }
inline bool read(uint8_t* data, size_t n) {
    uint32_t idle=millis();
    while(n) {
        int available=port().available();
        if(available>0) { size_t count=std::min(n,size_t(available)); size_t got=port().read(data,count); data+=got;n-=got;idle=millis(); }
        else { if(millis()-idle>2500) return false; delay(1); }
    }
    return true;
}
inline const char* receive(bool preview, uint8_t* buffer, size_t capacity, File* file, size_t& length) {
    length=0;
    while(port().available()) port().read();
    const uint8_t request[]={'S','C','Q','1',uint8_t(preview?'P':'S')};
    port().write(request,sizeof(request));
    uint8_t head[16];
    if(!read(head,sizeof(head))) return "No camera reply. Check cable.";
    if(memcmp(head,"SCJ1",4)) return "Camera protocol mismatch";
    if(head[4]) return "Camera capture failed";
    const uint32_t size=le32(head+8), expected=le32(head+12);
    if(size<4 || size>(preview?capacity:1024U*1024U)) return "Camera image too large";
    uint8_t chunk[256], first[2]={}, last[2]={}; uint32_t check=0xffffffffU;
    const uint32_t started=millis();
    for(size_t offset=0;offset<size;) {
        const size_t n=std::min(size_t(size-offset),sizeof(chunk));
        if(millis()-started>45000 || !read(chunk,n)) return "Camera transfer timed out";
        if(!offset) memcpy(first,chunk,2);
        if(offset+n==size) { last[0]=n>=2?chunk[n-2]:last[1];last[1]=chunk[n-1]; }
        else if(n>=2) { last[0]=chunk[n-2];last[1]=chunk[n-1]; }
        check=crc(check,chunk,n);
        if(file) { if(file->write(chunk,n)!=n) return "SD photo write failed"; }
        else memcpy(buffer+offset,chunk,n);
        offset+=n;
    }
    if((check^0xffffffffU)!=expected || first[0]!=0xff || first[1]!=0xd8 || last[0]!=0xff || last[1]!=0xd9) return "Photo damaged. Try again.";
    length=size;
    static bool previewLogged=false;
    if(!preview || !previewLogged) {
        Serial.printf("UnitCamS3: %s JPEG %u bytes CRC OK\n",preview?"preview":"photo",unsigned(size));
        if(preview) previewLogged=true;
    }
    return nullptr;
}
inline const char* save(const String& destination) {
    String temporary=destination+".partial";
    if(SD.exists(destination)||SD.exists(temporary)) return "Photo exists; kept original";
    File file=SD.open(temporary,FILE_WRITE);
    if(!file) return "Cannot create photo file";
    size_t size=0; const char* error=receive(false,nullptr,0,&file,size);
    file.flush(); file.close();
    if(error) { SD.remove(temporary); return error; }
    if(!SD.rename(temporary,destination)) { SD.remove(temporary); return "Cannot finish photo save"; }
    Serial.printf("Photo saved: %s (%u bytes)\n",destination.c_str(),unsigned(size));
    return nullptr;
}
} // namespace unitcam
