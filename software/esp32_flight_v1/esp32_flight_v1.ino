#include <Wire.h>
#include <FastLED.h>
#include <WiFi.h>

WiFiUDP Udp;

const char* wifi_SSID="ESP32_del";  //存储AP的名称信息
const char* wifi_Password="esp32_1234";  //存储AP的密码信息

uint16_t udp_port=1122;  //存储需要监听的端口号

char incomingPacket[1024];  //存储Udp客户端发过来的数据
uint16_t Data_length,len;
int commaPosition;
String message,inString;
bool check_channel = true;


//ws2812引脚定义
#define LED_PIN 4
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];
//ws2812引脚定义

//PID设置定义
float pid_p_gain_roll = 1.3;               //Gain setting for the pitch and roll P-controller (default = 1.3).
float pid_i_gain_roll = 0.04;              //Gain setting for the pitch and roll I-controller (default = 0.04).
float pid_d_gain_roll = 18.0;              //Gain setting for the pitch and roll D-controller (default = 18.0).
int pid_max_roll = 400;                    //Maximum output of the PID-controller (+/-).

float pid_p_gain_pitch = pid_p_gain_roll;  //Gain setting for the pitch P-controller.
float pid_i_gain_pitch = pid_i_gain_roll;  //Gain setting for the pitch I-controller.
float pid_d_gain_pitch = pid_d_gain_roll;  //Gain setting for the pitch D-controller.
int pid_max_pitch = pid_max_roll;          //Maximum output of the PID-controller (+/-).

float pid_p_gain_yaw = 4.0;                //Gain setting for the yaw P-controller (default = 4.0).
float pid_i_gain_yaw = 0.02;               //Gain setting for the yaw I-controller (default = 0.02).
float pid_d_gain_yaw = 0.0;                //Gain setting for the yaw D-controller (default = 0.0).
int pid_max_yaw = 400;                     //Maximum output of the PID-controller (+/-).

boolean auto_level = true;                 //Auto level on (true) or off (false).
//PID设置定义

//陀螺仪校准
//Manual accelerometer calibration values for IMU angles:
int16_t manual_acc_pitch_cal_value = 0;
int16_t manual_acc_roll_cal_value = 0;

//Manual gyro calibration values.
//Set the use_manual_calibration variable to true to use the manual calibration variables.
uint8_t use_manual_calibration = false;    // Set to false or true;
int16_t manual_gyro_pitch_cal_value = 0;
int16_t manual_gyro_roll_cal_value = 0;
int16_t manual_gyro_yaw_cal_value = 0;

uint8_t gyro_address = 0x68;               //The I2C address of the MPU-6050 is 0x68 in hexadecimal form.

int16_t cal_int;

int16_t temperature, count_var;
int16_t acc_x, acc_y, acc_z;
int16_t gyro_pitch, gyro_roll, gyro_yaw;
//陀螺仪校准

//MPU6050-I2C引脚
#define SCL 22
#define SDA 21
//MPU6050-I2C引脚

//错误参数定义
uint8_t error;
uint8_t error_counter, error_led,error_timer;;
//错误参数定义

//ESC参数引脚定义
uint32_t res;
#define freq_t 250    // 频率
#define freq_d 4000
#define channel0 0    // 通道
#define channel1 1
#define channel2 2
#define channel3 3
#define resolution 16   // 分辨率

//const int esc1_pin = 12;
//const int esc2_pin = 14;
//const int esc3_pin = 23;
//const int esc4_pin = 15;

#define esc1_pin 26
#define esc2_pin 12
#define esc3_pin 27
#define esc4_pin 23

int32_t channel_1 = 0,channel_2 = 0,channel_3 = 0,channel_4 = 0; //通道1-副翼右左右，通道2-升降右上下，通道3-油门左上下，通道4-方向，左左右
int16_t throttle;
int16_t esc_1, esc_2, esc_3, esc_4;
uint8_t last_channel_1, last_channel_2, last_channel_3, last_channel_4;
uint8_t highByte, lowByte, start,channel_int=1;
//ESC参数引脚定义

int32_t acc_total_vector;
int32_t gyro_roll_cal, gyro_pitch_cal, gyro_yaw_cal;
uint32_t loop_timer;

float roll_level_adjust = 0, pitch_level_adjust = 0;
float pid_error_temp;
float pid_i_mem_roll, pid_roll_setpoint, gyro_roll_input, pid_output_roll, pid_last_roll_d_error;
float pid_i_mem_pitch, pid_pitch_setpoint, gyro_pitch_input, pid_output_pitch, pid_last_pitch_d_error;
float pid_i_mem_yaw, pid_yaw_setpoint, gyro_yaw_input, pid_output_yaw, pid_last_yaw_d_error;
float angle_roll_acc, angle_pitch_acc, angle_pitch, angle_roll;
float battery_voltage;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  WiFi.softAP(wifi_SSID,wifi_Password);  //打开ESP32热点

  Serial.print("\n开发板IP地址为：");
  Serial.println(WiFi.softAPIP());  //串口输出模块IP地址

//  WiFi.begin(wifi_SSID,wifi_Password);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//  Serial.println("");
//  Serial.println("WiFi connected");
//  
//  Serial.print("Camera Stream Ready! Go to: http://");
//  Serial.print(WiFi.localIP());
  
  Udp.begin(udp_port);//启动UDP监听这个端口
  //UDP数据格式
  //Channel_1,Channel_2,Channel_3,Channel_4
  //UDP数据格式

  //ws2812初始化
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  red_led();

  //电调引脚初始化
  esc_init();
  delay(50);

  Wire.begin();                                                //Start the I2C as master
  Wire.setClock(400000L);
  Wire.beginTransmission(gyro_address);                        //Start communication with the MPU-6050.
  error = Wire.endTransmission();                              //End the transmission and register the exit status.
  while (error != 0) {                                          //Stay in this loop because the MPU-6050 did not responde.
    error = 2;                                                  //Set the error status to 2.
    error_signal();                                             //Show the error via the red LED.
    delay(4);
  }
  error = 0;
  gyro_setup();

  if (!use_manual_calibration) {
    //Create a 5 second delay before calibration.
    for (count_var = 0; count_var < 25; count_var++) {        //1250 loops of 4 microseconds = 5 seconds
      red_led();delay(100);
      led_null();delay(100);
    }
    count_var = 0;                                              //Set start back to 0.
  }

  battery_voltage = (float)analogRead(34) / 112.81;
  //battery_voltage = (float)analogRead(34) / 78.34;
  Serial.println(analogRead(34));
  Serial.println(battery_voltage);
  
  calibrate_gyro();

  while(check_channel){
    udp_read();
    if(channel_1 < 990 || channel_2 < 990 || channel_3 < 990 || channel_4 < 990){
      error = 3;                                                  //Set the error status to 3.
      error_signal();                                             //Show the error via the red LED.
      delay(4);
    }
    else if(channel_3 < 990 || channel_3 > 1050){
      error = 4;                                                  //Set the error status to 4.
      error_signal();                                             //Show the error via the red LED.
      delay(4);
    }
    else
    {
      error = 0;
      check_channel = false;
    }
  }

  
  green_led();
  loop_timer = micros();
}

void loop() {
  udp_read();
  
  //error_signal();                                                                  //Show the errors via the red LED.
  if(error == 1){
    leds[0] = CRGB(255,0,0);
    FastLED.show();
  }
  else if(error == 5){
    leds[0] = CRGB(0,0,255);
    FastLED.show();
  }
  
  gyro_signalen();                                                                 //Read the gyro and accelerometer data.

  //65.5 = 1 deg/sec (check the datasheet of the MPU-6050 for more information).
  gyro_roll_input = (gyro_roll_input * 0.7) + (((float)gyro_roll / 65.5) * 0.3);   //Gyro pid input is deg/sec.
  gyro_pitch_input = (gyro_pitch_input * 0.7) + (((float)gyro_pitch / 65.5) * 0.3);//Gyro pid input is deg/sec.
  gyro_yaw_input = (gyro_yaw_input * 0.7) + (((float)gyro_yaw / 65.5) * 0.3);      //Gyro pid input is deg/sec.

  //Gyro angle calculations
  //0.0000611 = 1 / (250Hz / 65.5)
  angle_pitch += (float)gyro_pitch * 0.0000611;                                    //Calculate the traveled pitch angle and add this to the angle_pitch variable.
  angle_roll += (float)gyro_roll * 0.0000611;                                      //Calculate the traveled roll angle and add this to the angle_roll variable.

  //0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians and not degrees.
  angle_pitch -= angle_roll * sin((float)gyro_yaw * 0.000001066);                  //If the IMU has yawed transfer the roll angle to the pitch angel.
  angle_roll += angle_pitch * sin((float)gyro_yaw * 0.000001066);                  //If the IMU has yawed transfer the pitch angle to the roll angel.

  //Accelerometer angle calculations
  acc_total_vector = sqrt((acc_x * acc_x) + (acc_y * acc_y) + (acc_z * acc_z));    //Calculate the total accelerometer vector.

  if (abs(acc_y) < acc_total_vector) {                                             //Prevent the asin function to produce a NaN.
    angle_pitch_acc = asin((float)acc_y / acc_total_vector) * 57.296;              //Calculate the pitch angle.
  }
  if (abs(acc_x) < acc_total_vector) {                                             //Prevent the asin function to produce a NaN.
    angle_roll_acc = asin((float)acc_x / acc_total_vector) * 57.296;               //Calculate the roll angle.
  }

  angle_pitch = angle_pitch * 0.9996 + angle_pitch_acc * 0.0004;                   //Correct the drift of the gyro pitch angle with the accelerometer pitch angle.
  angle_roll = angle_roll * 0.9996 + angle_roll_acc * 0.0004;                      //Correct the drift of the gyro roll angle with the accelerometer roll angle.

  pitch_level_adjust = angle_pitch * 15;                                           //Calculate the pitch angle correction.
  roll_level_adjust = angle_roll * 15;                                             //Calculate the roll angle correction.
  if (!auto_level) {                                                               //If the quadcopter is not in auto-level mode
    pitch_level_adjust = 0;                                                        //Set the pitch angle correction to zero.
    roll_level_adjust = 0;                                                         //Set the roll angle correcion to zero.
  }


  //For starting the motors: throttle low and yaw left (step 1).
  if (start == 0 && channel_3 < 1050 && channel_4 < 1050){
    start = 1;
    led_null();
  }
  //When yaw stick is back in the center position start the motors (step 2).
  if (start == 1 && channel_3 < 1050 && channel_4 > 1450) {
    start = 2;

    //led_null();                                                                //Turn off the green led.

    angle_pitch = angle_pitch_acc;                                                 //Set the gyro pitch angle equal to the accelerometer pitch angle when the quadcopter is started.
    angle_roll = angle_roll_acc;                                                   //Set the gyro roll angle equal to the accelerometer roll angle when the quadcopter is started.

    //Reset the PID controllers for a bumpless start.
    pid_i_mem_roll = 0;
    pid_last_roll_d_error = 0;
    pid_i_mem_pitch = 0;
    pid_last_pitch_d_error = 0;
    pid_i_mem_yaw = 0;
    pid_last_yaw_d_error = 0;
  }
  //Stopping the motors: throttle low and yaw right.
  if (start == 2 && channel_3 < 1050 && channel_4 > 1950) {
    start = 0;
    green_led();                                                               //Turn on the green led.
  }

  //The PID set point in degrees per second is determined by the roll receiver input.
  //In the case of deviding by 3 the max roll rate is aprox 164 degrees per second ( (500-8)/3 = 164d/s ).
  pid_roll_setpoint = 0;
  //We need a little dead band of 16us for better results.
  if (channel_1 > 1508)pid_roll_setpoint = channel_1 - 1508;
  else if (channel_1 < 1492)pid_roll_setpoint = channel_1 - 1492;

  pid_roll_setpoint -= roll_level_adjust;                                          //Subtract the angle correction from the standardized receiver roll input value.
  pid_roll_setpoint /= 3.0;                                                        //Divide the setpoint for the PID roll controller by 3 to get angles in degrees.


  //The PID set point in degrees per second is determined by the pitch receiver input.
  //In the case of deviding by 3 the max pitch rate is aprox 164 degrees per second ( (500-8)/3 = 164d/s ).
  pid_pitch_setpoint = 0;
  //We need a little dead band of 16us for better results.
  if (channel_2 > 1508)pid_pitch_setpoint = channel_2 - 1508;
  else if (channel_2 < 1492)pid_pitch_setpoint = channel_2 - 1492;

  pid_pitch_setpoint -= pitch_level_adjust;                                        //Subtract the angle correction from the standardized receiver pitch input value.
  pid_pitch_setpoint /= 3.0;                                                       //Divide the setpoint for the PID pitch controller by 3 to get angles in degrees.

  //The PID set point in degrees per second is determined by the yaw receiver input.
  //In the case of deviding by 3 the max yaw rate is aprox 164 degrees per second ( (500-8)/3 = 164d/s ).
  pid_yaw_setpoint = 0;
  //We need a little dead band of 16us for better results.
  if (channel_3 > 1050) { //Do not yaw when turning off the motors.
    if (channel_4 > 1508)pid_yaw_setpoint = (channel_4 - 1508) / 3.0;
    else if (channel_4 < 1492)pid_yaw_setpoint = (channel_4 - 1492) / 3.0;
  }

  calculate_pid();                                                                 //PID inputs are known. So we can calculate the pid output.

  battery_voltage = battery_voltage * 0.92 + ((float)analogRead(34) / 1410.1);
  if (battery_voltage < 9.5 && error == 0)error = 1;

  throttle = channel_3;                                                            //We need the throttle signal as a base signal.

  if (start == 2) {                                                                //The motors are started.
    if (throttle > 1800) throttle = 1800;                                          //We need some room to keep full control at full throttle.
    esc_1 = throttle - pid_output_pitch + pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 1 (front-right - CCW).
    esc_2 = throttle + pid_output_pitch + pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 2 (rear-right - CW).
    esc_3 = throttle + pid_output_pitch - pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 3 (rear-left - CCW).
    esc_4 = throttle - pid_output_pitch - pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 4 (front-left - CW).
    
    if (esc_1 < 1100) esc_1 = 1100;                                                //Keep the motors running.
    if (esc_2 < 1100) esc_2 = 1100;                                                //Keep the motors running.
    if (esc_3 < 1100) esc_3 = 1100;                                                //Keep the motors running.
    if (esc_4 < 1100) esc_4 = 1100;                                                //Keep the motors running.

    if (esc_1 > 2000)esc_1 = 2000;                                                 //Limit the esc-1 pulse to 2000us.
    if (esc_2 > 2000)esc_2 = 2000;                                                 //Limit the esc-2 pulse to 2000us.
    if (esc_3 > 2000)esc_3 = 2000;                                                 //Limit the esc-3 pulse to 2000us.
    if (esc_4 > 2000)esc_4 = 2000;                                                 //Limit the esc-4 pulse to 2000us.
  }

  else {
    esc_1 = 1000;                                                                  //If start is not 2 keep a 1000us pulse for ess-1.
    esc_2 = 1000;                                                                  //If start is not 2 keep a 1000us pulse for ess-2.
    esc_3 = 1000;                                                                  //If start is not 2 keep a 1000us pulse for ess-3.
    esc_4 = 1000;                                                                  //If start is not 2 keep a 1000us pulse for ess-4.
  }

  ledcWrite(channel0, us2res(esc_1));
  ledcWrite(channel1, us2res(esc_2));
  ledcWrite(channel2, us2res(esc_3));
  ledcWrite(channel3, us2res(esc_4));                                                        //This will reset timer 4 and the ESC pulses are directly created.

  if (micros() - loop_timer > 4050){
    error = 5;                                      //Turn on the LED if the loop time exceeds 4050us.
  }
  while (micros() - loop_timer < 4000);                                            //We wait until 4000us are passed.
  loop_timer = micros();                                                           //Set the timer for the next loop.
}
