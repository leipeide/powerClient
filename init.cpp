#include <Arduino.h>
#include "config.h"
#include <U8g2lib.h>   

int freq = 2000;                                                                   // 频率
int channel = 0;                                                                   // led通道
int resolution = 12;                                                               // 分辨率(占空比最大可写4095)


//构造相应模式下的u8对象：SH1106驱动的oled显示屏，采用软件驱动I2C
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,22,32);                          //SH1106软件驱动I2C设置，设置对应的clk(22),data(32)引脚即可


//初始化参数配置
void init(){
      Serial.println("进入init");
      // 设置串口波特率
      Serial.begin(115200);                                                     // 打开Usb烧录程序串口
      Serial2.begin(4800);                                                      // 打开UART2,读取功率计输入参数
      //1.初始化屏幕
      u8g2.begin();               
     
      // 初始化调光引脚,占空比
      pinMode(Pwm1_Pin,OUTPUT);
      ledcSetup(channel, freq, resolution);                                     // 设置通道
      ledcAttachPin(Pwm1_Pin, channel);                                         // 将通道与对应的引脚连接
      ledcWrite(channel, 4095);                                                 // 输出PWM 100%
      
      //设置引脚输入输出模式    
      pinMode(High_Pin,INPUT);    
      pinMode(PF2_Pin,INPUT);  
      pinMode(Vout_On_Pin,OUTPUT);            
      pinMode(Vin_On_Pin,OUTPUT);   
      //初始化电压控制引脚高电平          
      //digitalWrite(Vout_On_Pin,HIGH);                                         // 高电平有效                       
      //digitalWrite(Vin_On_Pin,HIGH);                                          // 高电平有效
}
