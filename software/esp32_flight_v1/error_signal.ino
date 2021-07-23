void error_signal(void){
  switch(error){
    case 0:break;
    case 1:leds[0] = CRGB(240,0,0);FastLED.show();break; //电池电压过低
    case 2:leds[0] = CRGB(0,255,255);FastLED.show();break; //I2C通信错误
    case 3:leds[0] = CRGB(255,255,255);FastLED.show();break; //输入通道错误
    case 4:leds[0] = CRGB(255,0,255);FastLED.show();break; //油门通道未处于关闭状态
    case 5:leds[0] = CRGB(255,255,0);FastLED.show();break;
  }
  
}
