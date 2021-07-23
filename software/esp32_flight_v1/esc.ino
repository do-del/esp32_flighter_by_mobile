void esc_init(void){
  ledcSetup(channel0, freq_t, resolution); // 设置通道
  ledcAttachPin(esc1_pin, channel0);  // 将通道与对应的引脚连接
  
  ledcSetup(channel1, freq_t, resolution); // 设置通道
  ledcAttachPin(esc2_pin, channel1);  // 将通道与对应的引脚连接
  
  ledcSetup(channel2, freq_t, resolution); // 设置通道
  ledcAttachPin(esc3_pin, channel2);  // 将通道与对应的引脚连接
  
  ledcSetup(channel3, freq_t, resolution); // 设置通道
  ledcAttachPin(esc4_pin, channel3);  // 将通道与对应的引脚连接
  
  ledcWrite(channel0, us2res(1000));
  ledcWrite(channel1, us2res(1000));
  ledcWrite(channel2, us2res(1000));
  ledcWrite(channel3, us2res(1000));
}

inline uint16_t us2res(uint16_t us){
  res = 65535*us/freq_d;
  return res;
}
