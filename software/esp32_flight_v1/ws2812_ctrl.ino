void red_led(void){
  leds[0] = CRGB(255,0,0);
  FastLED.show();
}

void blue_led(void){
  leds[0] = CRGB(0,0,255);
  FastLED.show();
}

void green_led(void){
  leds[0] = CRGB(0,255,0);
  FastLED.show();
}

void led_null(void){
  leds[0] = CRGB(0,0,0);
  FastLED.show();
}
