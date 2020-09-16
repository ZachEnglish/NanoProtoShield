#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup(){
    g_nps.begin();
    
    g_nps.OLED_print(F("Calculating gyro offset, do not move MPU6050"));
    g_nps.OLED_display();
    g_nps.MPU_calculate_offsets(1000);
    g_nps.clear_all_displays();
}

uint8_t map_angle_to_brightness(float a){
    return( (a + 90.0f) * (255.0f/180.0f) );
}

uint16_t map_angle_to_hue(float a){
    return( (a + 90.0f) * (255.0f*255.0f/180.0f) );
}

void loop(){
    uint8_t brightness;
    uint16_t hue;
    float x, y;

    g_nps.MPU_update();

    x = g_nps.MPU_get_acc_angle_x();
    y = g_nps.MPU_get_acc_angle_y();

    brightness = map_angle_to_brightness(x);
    hue = map_angle_to_hue(y);

    g_nps.RGB_set_pixels_color(hue);
    g_nps.RGB_set_brightness(brightness);
    g_nps.RGB_show();
}