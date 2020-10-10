#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <timer.h>
#include "config.h"
#include <U8g2lib.h>             


using namespace websockets;                                            // 
WebsocketsClient client;                                              // 声明websocket客户端对象;       
int connectTimeCount = 0;                                               // 与服务器连接的时间，每接受到服务器端的信息则将connectTimeCount清零
StaticJsonDocument<400> jsonDoc;                                      // 声明一个JsonDocument对象，200是大小

auto HB_Timer = timer_create_default();                                 // 使用默认设置创建计时器;心跳包机制定时器 
auto Data_Handle_Timer = timer_create_default();                        // 使用默认设置创建计时器;串口2读取数据、输出AD采样数据处理机制定时器 


//1.发送数据至服务器；json对象进行序列化，转换为json字符串
void sendJson(StaticJsonDocument<400> data)
{
    //序列化json对象
    char myDocP[measureJson(data) + 1];
    serializeJson(data, myDocP, measureJson(data) + 1);
    Serial.println(myDocP);
    client.send(myDocP);                                                 //websocket发送json数据
  
  }

//7.连接服务器
void connectServer()
{
    Serial.println(",Connected to Wifi, Connecting to server.");
    client.connect(websockets_server_host, websockets_server_port, "/power/websocket");                          //连接服务器
    delay(1000); // 延时1秒
    while(!client.available()){
        Serial.println("Server Connect falied! 重新连接");
        client.connect(websockets_server_host, websockets_server_port, "/power/websocket");                      // 重新连接服务器
        if(client.available()){
            break;
        }
     }
     Serial.println("Server Connected!");
     //1.上报mac地址
     uploadMacAddr();
}

// 8.服务器信息处理机制
void onMessageCallback(WebsocketsMessage message) {
   Serial.println(message.data());
   StaticJsonDocument<400> Re_JsonDoc;
   DeserializationError error = deserializeJson(Re_JsonDoc,message.data());                                    // 解码json字符串
   if(error) {                                                                                                    // 解码失败
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
   }   
   
   // 将与服务器连接的时间计数器清零
   connectTimeCount = 0;    
   Serial.println("将与服务器连接的时间计数器清零");                                                                   
   //处理请求机制
   if(Re_JsonDoc["msg"] == "request"){                                                                            // 服务器请求
         //Serial.println("服务器请求");
         if(Re_JsonDoc["cmd"] == "switch"){                                                                       // 处理开关指令
                switchStateRep(Re_JsonDoc);
                //Serial.println("服务器请求写指令");
         } else if (Re_JsonDoc["cmd"] == "dim"){                                                                 // 处理调光指令
                dimStateRep(Re_JsonDoc); 
                 
         } else if (Re_JsonDoc["cmd"] == "read"){                                                                // 处理读状态指令
                readStateRep(Re_JsonDoc); 
                 
         }    
      
    } else if (Re_JsonDoc["msg"] == "response"){                                                                  // 服务器回复
          
        if(Re_JsonDoc["cmd"] == "login"){
             //2.每10秒发送一个心跳包
             HB_Timer.every(10000,hb_judge_function);
        }
    }
    
}

//10.心跳包时间判断函数；正常每10S AD采样一次输出参数;发送一次心跳包
bool hb_judge_function(void *){
     connectTimeCount = connectTimeCount + 10;
     if(connectTimeCount > 30){
           Serial.println("超过30秒则客户端主动与服务器断开连接，并进行重连");
           client.close();  //与服务器断连，触发ConnectionClosed事件
           return false;
     }else{         
          // 发送心跳包
          uploadHeartbeat();   
     }
     return true;
 }

//9.websocket事件处理机制
void onEventsCallback(WebsocketsEvent event, String data) {

    if(event == WebsocketsEvent::ConnectionOpened) {                                          // Dispatched when connecting to a server
        Serial.println("Connnection Opened");
       
        
    } else if(event == WebsocketsEvent::ConnectionClosed) {                                  // Dispatched when close the server
        Serial.println("Connnection Closed");
        connectServer();                                                                       // 服务器重连
        
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
    
}



void setup() {
    
      init();     // 初始化引脚、参数配置
      
      attachInterrupt(High_Pin, highDutyCycle, RISING);          // High引脚上升沿中断，进入中断函数
      Data_Handle_Timer.every(1000,data_handle_function);        // 开启获取输入、输出参数定时器        
      //Serial.println("");
      //1.完成网络连接
      if (!AutoConfig()){
        SmartConfig();
      }
      
      //2.网络正常，进行与服务器连接
      if(wifi_Flag){
          connectServer();
        }
     
      //3.websocket客户端接收信息函数;每次收到消息时都会调用onMessage回调。为了让客户端处理新消息，用户必须调用poll（）
      client.onMessage(onMessageCallback);
      
      //4.每次接收控制帧或内部发生事件（例如用户关闭套接字）时，都将调用onEvent回调。为了让客户端处理新帧，用户必须调用poll（）。
      client.onEvent(onEventsCallback);


}


void loop() {   
  
    Data_Handle_Timer.tick();                             // 1.计时:开启数据处理包计时器
                                          
    if(client.available()){                              // 2.与服务器正常连接的状态下  
         client.poll();                                  // 保持接收信息
         HB_Timer.tick();                                // 计时:开启心跳包计时器
    }
 
}

