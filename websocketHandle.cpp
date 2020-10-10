#include <Arduino.h>
#include "config.h"
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>

const char* websockets_server_host = "3s1777a348.wicp.vip";                               // 服务器网址 3s1777a348.wicp.vip
const uint16_t websockets_server_port = 37063;                                            // 服务器端口号 37063

int pwm = 0;                                                                              // 初始化调光参数0-4095；
boolean VinonFlag = false;                                                                // 输入电压继电器开合标志位初始化为true,上电后默认打开
boolean VinoffFlag = false;                                                               // 输入电压继电器关闭状态标志位初始
/*int high_count = 0;                                                                     // high引脚由低电平变为高电平时触发中断的次数
static long firstms;                                                                      // 第一次进入中断时的毫秒数
static long lastms;                                                                       // 第二次进入中断时的毫秒数*/         


// 11. High引脚上升沿脉冲中断处理函数，读频率是50HZ，在上升沿时对开关指令进行处理
void highDutyCycle(){
     if(VinonFlag || VinoffFlag){                                                         // 服务器端发送了开、关指令
          if(VinonFlag){                                                                  // 开关标志位为true时，在上升沿状态下打开输入电压继电器
               VinonFlag = false;                                                         // 打开继电器后恢复输入开关标志位
               digitalWrite(Vin_On_Pin,HIGH);                                             // 打开输入继电器
               Serial.println("继电器已打开");
               ledcWrite(channel,pwm);                                                    // 输出PWM
               Serial.print("输出PWM:");
               Serial.print(pwm);
                
          }else if(VinoffFlag){
               VinoffFlag = false;
               digitalWrite(Vin_On_Pin,LOW);                                              // 关闭输入继电器
               Serial.println("继电器已关闭");
               ledcWrite(channel,pwm);                                                    // 输出PWM
               Serial.print("输出PWM:");
               Serial.print(pwm);
          }
          // 回复开关指令
          jsonDoc["msg"] = "reponse";
          jsonDoc["cmd"] = "switch";
          jsonDoc["mac"] = WiFi.macAddress();
          jsonDoc["switchState"] = digitalRead(Vin_On_Pin);
          jsonDoc["precentage"] = ledcRead(channel)*100/4095;
          jsonDoc["ipower"] = ipower;
          jsonDoc["ivoltage"] = ivolt;
          jsonDoc["icurrent"] = icurr;
          jsonDoc["opower"] = opower;
          jsonDoc["ovoltage"] = ovolt;
          jsonDoc["ocurrent"] = ocurr;
          jsonDoc["pf"] = pf;
          jsonDoc["temperature"] = "";
          jsonDoc["ssid"] = password;
          jsonDoc["pw"] = wifiname;
          //json对象转换为json字符串并发送
          sendJson(jsonDoc);
      }
      /*
     high_count ++;
     long ms = micros();
     if(high_count == 1){   // 第一次进入中断
           firstms = ms;
      }else if(high_count == 2){
           lastms = ms;
           high_count = 0;
      }
      // f = 1000/(lastms - firstms)*1000;  // 得出频率
      /*else if(high_count > 2){        //超过两次则终止中断
          detachInterrupt(digitalPinToInterrupt(PF2_Pin));
      } */
  }


//6.上报节点mac地址，实现登录
void uploadMacAddr(){
      jsonDoc["msg"] = "request";
      jsonDoc["cmd"] = "login";
      jsonDoc["mac"] = WiFi.macAddress();
      //jsonDoc["switchState"] = 1;
      jsonDoc["switchState"] = digitalRead(Vin_On_Pin);
      //jsonDoc["precentage"] = 100;
      jsonDoc["precentage"] = ledcRead(channel)*100/4095;
      jsonDoc["ipower"] = ipower;
      jsonDoc["ivoltage"] = ivolt;
      jsonDoc["icurrent"] = icurr;
      jsonDoc["opower"] = opower;
      jsonDoc["ovoltage"] = ovolt;
      jsonDoc["ocurrent"] = ocurr;
      jsonDoc["pf"] = pf;
      jsonDoc["temperature"] = "";
      jsonDoc["ssid"] = password;
      jsonDoc["pw"] = wifiname;
      //json对象转换为json字符串并发送
      sendJson(jsonDoc);
  }

  //7.发送心跳包数据
void uploadHeartbeat(){
      jsonDoc["msg"] = "request";
      jsonDoc["cmd"] = "heartbeat";
      jsonDoc["mac"] = WiFi.macAddress();
      jsonDoc["switchState"] = 1;
     // jsonDoc["switchState"] = digitalRead(Vin_On_Pin);
      //jsonDoc["precentage"] = 100;
      jsonDoc["precentage"] = ledcRead(channel)*100/4095;
      jsonDoc["ipower"] = ipower;
      jsonDoc["ivoltage"] = ivolt;
      jsonDoc["icurrent"] = icurr;
      jsonDoc["opower"] = opower;
      jsonDoc["ovoltage"] = ovolt;
      jsonDoc["ocurrent"] = ocurr;
      jsonDoc["pf"] = pf;
      jsonDoc["temperature"] = "";
      jsonDoc["ssid"] = password;
      jsonDoc["pw"] = wifiname;
      //json对象转换为json字符串并发送
      sendJson(jsonDoc);
  }

// 处理开关状态指令并回复 
void switchStateRep(StaticJsonDocument<400> Re_JsonDoc){
         Serial.println("处理服务器开关请求");
        int percentage = Re_JsonDoc["precentage"];                                        // 调光百分比参数
        int switchState = Re_JsonDoc["switchState"];                                      // 开关参数
        // 处理写状态指令
        pwm = ceil(percentage*4095/100);                                                  // 向上取整
        if(pwm > 4095){
                  pwm = 4095;
        }else if(pwm < 0){
                  pwm = 0;
        }   
         
        // 修改pwm输出和使能脚的电平
        switch (switchState){
              case 1:                                                                     // 开状态指令
                 if(digitalRead(Vin_On_Pin) == HIGH){                                     // 状态本来就是开的时候
                      ledcWrite(channel,pwm);                                             // 输出PWM
                      Serial.print("输出PWM:");
                      Serial.print(pwm);  
                      // 回复开关指令
                      jsonDoc["msg"] = "reponse";
                      jsonDoc["cmd"] = "switch";
                      jsonDoc["mac"] = WiFi.macAddress();
                      jsonDoc["switchState"] = digitalRead(Vin_On_Pin);
                      jsonDoc["precentage"] = ledcRead(channel)*100/4095;
                      jsonDoc["ipower"] = ipower;
                      jsonDoc["ivoltage"] = ivolt;
                      jsonDoc["icurrent"] = icurr;
                      jsonDoc["opower"] = opower;
                      jsonDoc["ovoltage"] = ovolt;
                      jsonDoc["ocurrent"] = ocurr;
                      jsonDoc["pf"] = pf;
                      jsonDoc["temperature"] = "";
                      jsonDoc["ssid"] = password;
                      jsonDoc["pw"] = wifiname;
                      //json对象转换为json字符串并发送
                      sendJson(jsonDoc);
                          
                  }else{                                                                   // 继电器当前状态是关的时候
                         VinonFlag = true;                                                // 输入电压继电器开关标志位设置为true,在输入电压上升沿时触发开指令
                  }
                  break;
                    
              case 0:                                                                     // 关状态指令
                  if(digitalRead(Vin_On_Pin) == LOW){                                     // 状态本来就是关的时候
                       ledcWrite(channel,pwm);                                            // 输出PWM
                       Serial.print("输出PWM:");
                       Serial.print(pwm);  
                       // 回复开关指令
                       jsonDoc["msg"] = "reponse";
                       jsonDoc["cmd"] = "switch";
                       jsonDoc["mac"] = WiFi.macAddress();
                       jsonDoc["switchState"] = digitalRead(Vin_On_Pin);
                       jsonDoc["precentage"] = ledcRead(channel)*100/4095;
                       jsonDoc["ipower"] = ipower;
                       jsonDoc["ivoltage"] = ivolt;
                       jsonDoc["icurrent"] = icurr;
                       jsonDoc["opower"] = opower;
                       jsonDoc["ovoltage"] = ovolt;
                       jsonDoc["ocurrent"] = ocurr;
                       jsonDoc["pf"] = pf;
                       jsonDoc["temperature"] = "";
                       jsonDoc["ssid"] = password;
                       jsonDoc["pw"] = wifiname;
                       //json对象转换为json字符串并发送
                       sendJson(jsonDoc);
                           
                    }else{                                                       // 继电器当前状态是关的时候
                         VinoffFlag = true;                                     // 输入电压继电器关闭标志位设置为true,在输入电压上升沿时触发开指令
                    }
                    break;
                }
            
    }


// 处理调光状态指令并回复 
void dimStateRep(StaticJsonDocument<400> Re_JsonDoc){
        Serial.println("处理服务器调光请求");
        int percentage = Re_JsonDoc["precentage"];                      // 调光百分比参数
        // 处理写状态指令
        pwm = ceil(percentage*4095/100);                                // 向上取整
        if(pwm > 4095){
                  pwm = 4095;
        }else if(pwm < 0){
                  pwm = 0;
        }   

        ledcWrite(channel,pwm);  // 输出PWM
        Serial.print("输出PWM:");
        Serial.print(pwm);
            
        // 回复调光指令
        jsonDoc["msg"] = "reponse";
        jsonDoc["cmd"] = "dim";
        jsonDoc["mac"] = WiFi.macAddress();
        jsonDoc["switchState"] = 1;
        //jsonDoc["switchState"] = digitalRead(Vin_On_Pin);
        jsonDoc["precentage"] = ledcRead(channel)*100/4095;
        jsonDoc["ipower"] = ipower;
        jsonDoc["ivoltage"] = ivolt;
        jsonDoc["icurrent"] = icurr;
        jsonDoc["opower"] = opower;
        jsonDoc["ovoltage"] = ovolt;
        jsonDoc["ocurrent"] = ocurr;
        jsonDoc["pf"] = pf;
        jsonDoc["temperature"] = "";
        jsonDoc["ssid"] = password;
        jsonDoc["pw"] = wifiname;
        //json对象转换为json字符串并发送
        sendJson(jsonDoc);
    }

 // 处理读节点指令获取节点的最新状态，并回复 
 void readStateRep(StaticJsonDocument<400> Re_JsonDoc){
          // 回复读指令
          jsonDoc["msg"] = "reponse";
          jsonDoc["cmd"] = "read";
          jsonDoc["mac"] = WiFi.macAddress();
          jsonDoc["switchState"] = 1;
          //jsonDoc["switchState"] = digitalRead(Vin_On_Pin);
          //jsonDoc["precentage"] = 100;
          jsonDoc["precentage"] = ledcRead(channel)*100/4095;
          jsonDoc["ipower"] = ipower;
          jsonDoc["ivoltage"] = ivolt;
          jsonDoc["icurrent"] = icurr;
          jsonDoc["opower"] = opower;
          jsonDoc["ovoltage"] = ovolt;
          jsonDoc["ocurrent"] = ocurr;
          jsonDoc["pf"] = pf;
          jsonDoc["temperature"] = "";
          jsonDoc["ssid"] = password;
          jsonDoc["pw"] = wifiname;
          //json对象转换为json字符串并发送
          sendJson(jsonDoc);
    
    }
