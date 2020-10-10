#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <U8g2lib.h> 

#define Iout_AD_Pin 34                                                  // 输出电流AD采样引脚
#define Vout_AD_Pin 36                                                  // 输出电压AD采样引脚
#define High_Pin 33                                                     // High引脚
#define PF2_Pin 25                                                      // PF2引脚
#define Vout_On_Pin 26                                                  // 输出电压开关控制引脚
#define Vin_On_Pin 27                                                   // 输出电压开关控制引脚
#define Pwm1_Pin 21                                                     // 调光控制引脚
#define N 100                                                           // AD

extern int freq;                                                        // 频率
extern int channel;                                                     // led通道
extern int resolution;                                                  // 分辨率(占空比最大可写4095)

extern byte uart2_Data[24];                                             // 串口2接收的数据；串口2发送24个字节大小的数据
extern boolean wifi_Flag;                                               // 网络配置标志位
extern boolean websocket_Flag;                                          // 服务器连接标志位
extern String password;                                                 // wifi密码
extern String wifiname;                                                 // wifi名称
extern String mac;                                                      // 节点mac地址
extern const char* websockets_server_host;                              // 服务器地址   3s1777a348.wicp.vip
extern const uint16_t websockets_server_port;                           // 服务器端口号    37063
extern U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2;                        // SH1106软件驱动I2C设置，设置对应的clk(22),data(32)引脚即可

//extern int high_count;                                                // pf2引脚由低电平变为高电平时触发中断的次数
//extern static long firstms;                                           // 第一次进入中断时的毫秒数
//extern static long lastms;                                            // 第二次进入中断时的毫秒数
extern boolean VinonFlag;                                               // 输入电压继电器开合标志位初始化为true,上电后默认打开
extern boolean VinoffFlag;                                              // 输入电压继电器关闭状态标志位初始化

extern boolean switchFlag;                                              // 开关状态
extern int pwm;                                                         // 初始化调光参数0-4095；
extern int volt_paraREG;                                                // 测量芯片电压参数寄存器参数
extern float volt_REG;                                                  // 测量芯片电压寄存器参数
extern int curr_paraREG;                                                // 测量芯片电流参数寄存器参数
extern float curr_REG;                                                  // 测量芯片电流寄存器参数
extern int power_paraREG;                                               // 测量芯片功率参数寄存器参数
extern float power_REG;                                                 // 测量芯片功率寄存器参数
       
extern float ovolt;                                                     // 输出电压
extern float ocurr;                                                     // 输出电流
extern float opower;                                                    // 输出功率
extern float ivolt;                                                     // 输入电压
extern float icurr;                                                     // 输入电流
extern float ipower;                                                    // 输入功率
extern float pf;                                                        // pf值
extern float effici;                                                    // 工作效率

         
extern int connectTimeCount;                                            // 与服务器连接的时间，每接受到服务器端的信息则将connectTimeCount清零
extern StaticJsonDocument<400> jsonDoc;                              // 声明一个JsonDocument对象，200是大小

//声明全局函数
void init();                                                           // 初始化配置:端口，设置等
bool AutoConfig();                                                     // 自动联网配置
void SmartConfig();                                                    // 智能配网
void connectServer();                                                  // 连接服务器
void highDutyCycle();                                                  // High引脚上升沿脉冲中断处理函数，读频率是50HZ，在上升沿时对开关指令进行处理
bool data_handle_function(void*);                                      // 串口2数据读取，输出AD数据处理
void sendJson(StaticJsonDocument<400>);                             // 对通讯协议进行打包处理发送至服务器
void uploadHeartbeat();                                               // 上传心跳包
void uploadMacAddr();                                                 // 上报mac地址
void switchStateRep(StaticJsonDocument<400>);                      // 开关指令回复
void dimStateRep(StaticJsonDocument<400>);                         // 调光指令回复
void readStateRep(StaticJsonDocument<400>);                        //  读状态回复


