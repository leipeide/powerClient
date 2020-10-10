#include <Arduino.h>
#include "config.h"
#include <WiFi.h>

String password = "";                                                     // wifi密码
String wifiname = "";                                                     // wifi名称
String mac = "";                                                          // 节点mac地址
boolean wifi_Flag = false;                                                // 网络配置标志位；默认为false

// 6.自动联网
bool AutoConfig()
{
   // Serial.println("进入AutoConfig");
    WiFi.begin();                                                        //开始自动联网
    for (int i = 0; i < 20; i++)
    {
        int wstatus = WiFi.status();
        if (wstatus == WL_CONNECTED)                                     //检测网络是否连接成功
        { 
          password = WiFi.SSID();
          wifiname = WiFi.psk();
          mac = WiFi.macAddress();
          wifi_Flag = true;
          Serial.println("WIFI AutoConfig Success");
          Serial.printf("SSID:%s", password);
          Serial.printf(", PSW:%s\r\n", wifiname);
          /*Serial.print("LocalIP:");
          Serial.print(WiFi.localIP());
          Serial.print(" ,GateIP:");
          Serial.println(WiFi.gatewayIP());*/
          Serial.print("MAC:");
          Serial.println(WiFi.macAddress());
          return true;
        }
        else
        {
          Serial.print("WIFI AutoConfig Waiting......");
          Serial.println(wstatus);
          delay(1000);
        }
    }
    Serial.println("WIFI AutoConfig Faild!" );
    return false;
}

//5.智能配网函数
void SmartConfig() 
{
      //Serial.println("SmartConfig");
      WiFi.mode(WIFI_AP_STA);
      WiFi.beginSmartConfig();
      Serial.println("Waiting for SmartConfig.");
      
      while (!WiFi.smartConfigDone()) {
        Serial.print(".");
        delay(500);
      }
      
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
     }
     
     password = WiFi.SSID();
     wifiname = WiFi.psk();
     WiFi.setAutoConnect(true);                                              // 设置自动连接网络
     wifi_Flag = true;                                                       // 网络状态设置为true
     Serial.println("WIFI SmartConfig Success");
     Serial.printf("SSID:%s",  password);
     Serial.printf(", PSW:%s\r\n", wifiname);
     Serial.print("LocalIP:");
     Serial.print(WiFi.localIP());
     Serial.print(" ,GateIP:");
     Serial.println(WiFi.gatewayIP());

}
