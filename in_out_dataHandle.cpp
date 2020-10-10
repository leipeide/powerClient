#include <Arduino.h>
#include "config.h" 
#include <U8g2lib.h>             


byte uart2_Data[24] = {};                                               // 串口2接收的数据；串口2发送24个字节大小的数据

int volt_paraREG = 0;                                                   // 测量芯片电压参数寄存器参数
float volt_REG = 0;                                                     // 测量芯片电压寄存器参数
int curr_paraREG = 0;                                                   // 测量芯片电流参数寄存器参数
float curr_REG = 0;                                                     // 测量芯片电流寄存器参数
int power_paraREG = 0;                                                  // 测量芯片功率参数寄存器参数
float power_REG = 0;                                                    // 测量芯片功率寄存器参数

float ovolt = 0;                                                        // 输出电压
float ocurr = 0;                                                        // 输出电流
float opower = 0;                                                       // 输出功率
float ivolt = 0.00;                                                     // 输入电压
float icurr = 0.00;                                                     // 输入电流
float ipower = 0.00;                                                    // 输入功率
float pf = 0.00;                                                        // pf值
float effici = 0.00;                                                    // 工作效率

// 函数声明
void AdSampOut();
void serial2Event(); 
void pageDisplay();
 

//1.输出AD采样、输入串口2数据读取;并在屏幕上显示
bool data_handle_function(void*){
  
   // 1.每1s AD采样输出电压电流等参数 
   AdSampOut();  
   
   // 2.读取计量芯片串口发送的数据
   serial2Event();  

   // 3.将输入输出参数在oled上进行显示
   pageDisplay();
   return true;
   
  }

  
// 2.串口2事件函数(处理串口2读取的数据：输入电压，电流，功率)
void serial2Event(){
    //Serial.println("进入串口事件函数");
    uart2_Data[24] = {};                                                                                                // 串口2接收的数据；串口2发送24个字节大小的数据
    if(Serial2.available() > 0) {                                                                                      // 判断缓冲器是否有数据装入
        Serial2.flush();                                                                                               // 等待超出的串行数据完成传输。
        if(Serial2.readBytes(uart2_Data,24) != 0){                                                                     // 返回0意味着没有发现有效数据
              if(sizeof(uart2_Data)/sizeof(uart2_Data[0]) == 24 && uart2_Data[1] == 0x5A){
                           
                   volt_paraREG = (int(uart2_Data[2]))*65536 + int(uart2_Data[3])*256 + int(uart2_Data[4]);             // 处理电压参数寄存器的值
                   volt_REG = int(uart2_Data[5])*65536 + int(uart2_Data[6])*256 + int(uart2_Data[7]);                   // 处理电压寄存器的值
                   ivolt = int((volt_paraREG / volt_REG * 1.53 - 2.0)*100.0) / 100.0;                                   // 电压值
                   
                   curr_paraREG = int(uart2_Data[8])*65536 + int(uart2_Data[9])*256 + int(uart2_Data[10]);              // 处理电流参数寄存器的值
                   curr_REG = int(uart2_Data[11])*65536 + int(uart2_Data[12])*256 + int(uart2_Data[13]);                // 处理电流寄存器的值
                   icurr = int(curr_paraREG / curr_REG * 0.1 * 100.0) / 100.0;                                          // 电流值
                  
                   power_paraREG = int(uart2_Data[14])*65536 + int(uart2_Data[15])*256 + int(uart2_Data[16]);           // 处理功率参数寄存器的值
                   power_REG = int(uart2_Data[17])*65536 + int(uart2_Data[18])*256 + int(uart2_Data[19]);               // 处理功率寄存器的值
                   ipower = int(ivolt * icurr * 100.0) / 100.0;                                                         // 功率值

                   pf = int(power_paraREG * 1.53 * 0.1 * 100.0/ power_REG / ipower) / 100.0;
                   
                  
                   /* 
                   Serial.print("串口2处理后的数据");
                   Serial.print("ivolt:");
                   Serial.print(ivolt);
                   Serial.print("icurr:");
                   Serial.print(icurr);
                   Serial.print("power:");
                   Serial.print(ipower);
                   Serial.print("pf:");
                   Serial.println(pf);
                   */
        }
        
        }
         
   }
       
}

 
//3.每1s钟AD采样输出参数
void AdSampOut(){
        long I_sum = 0;
        long V_sum = 0;
        // 算数平均法获取AD值
        for ( int count = 0; count < N; count++)
          {  
              I_sum = I_sum + analogRead(Iout_AD_Pin);
              V_sum = V_sum + analogRead(Vout_AD_Pin);
              delay(1); //延迟1毫秒
             // Serial.println(count);
          }
       
       int IoutAD = I_sum / N;
       int VoutAD = V_sum / N;
       ocurr = int(3.3*IoutAD/4095 * 100.0) / 100.0;                                                         // 处理读取的模拟电流值
       ovolt = int((VoutAD*203.3/4095*1.0176 + 5.7787 )*100.0) / 100.0;                                      // 处理读取的模拟电压值；1.0176、5.7787是为了解决电压与实际的偏差得到的系数
       //ocurr = int(3.3*IoutAD/4095 * 100.0) / 100.0;             
       //ovolt =  int(VoutAD*203.3/4095*100.0) / 100.0;                                                           // 处理读取的模拟电压值203.3/3.3
       opower = int(ovolt * ocurr * 100.0) / 100.0;
       /*
       int IoutAD = analogRead(Iout_AD_Pin);
       int VoutAD = analogRead(Vout_AD_Pin);
       ocurr = int(3.3*IoutAD/4095 * 100.0) / 100.0;                                                         // 处理读取的模拟电流值
       ovolt = int((VoutAD*203.3/4095*1.0176 + 5.7787 )*100.0) / 100.0;                                      // 处理读取的模拟电压值；1.0176、5.7787是为了解决电压与实际的偏差得到的系数
       opower = int(ovolt * ocurr * 100.0) / 100.0;
       */
       // ocurr = 3.3*IoutAD/4095 - 0.3;          
       //ovolt = 3.3*VoutAD*203.3/4095/3.3;                                                                  // 处理读取的模拟电压值203.3/3.3
     /*  
       Serial.print("输出电流采样数据：");
       Serial.println(IoutAD);
       Serial.print("输出电压采样数据：");
       Serial.println(VoutAD);
       Serial.print("真实输出电流值：");
       Serial.println(ocurr,4);
       Serial.print("真实输出电压值：");
       Serial.println(ovolt);
       Serial.print("输出功率：");
       Serial.println(opower,4);
       */
 }


// 4.进行屏幕显示
void pageDisplay(){
      if(opower == 0){
          effici = 0;
       }else{
          effici = int(opower / ipower * 100.0) / 100.0;                                      // 工作效率
       }
      u8g2.clearBuffer();                                                                // 2.清除缓存
      
      //开始绘制页面
      u8g2.drawHLine(0,11,128);                                                          // 绘制水平线；用于区分参数和状态 
      
      // wifi状态
      if(wifi_Flag){                                                                      // 联网状态，显示wifi图形
          u8g2.setFontDirection(1);                                                      // 设置旋转方向：0：0°；1：90°；2：180°；4：270°
          u8g2.drawPixel(120, 8);                                                        // 像素点
          u8g2.drawCircle(120,8,4,U8G2_DRAW_UPPER_RIGHT);                                // 画个空心圆，可选四个方向的半圆
          u8g2.drawCircle(120,8,7,U8G2_DRAW_UPPER_RIGHT);                                // 画个空心圆，可选四个方向的半圆
      }else{                                                                              // 未连接网络，显示×
          u8g2.drawLine(120, 8, 125, 3);                                                 // 两点之间连接的水平线
          u8g2.drawLine(125, 8, 120, 3);
      }
   
     // 绘制开关参数;采用特殊字符显示方式
     if(digitalRead(Vin_On_Pin)){                                                         // 开关状态
          u8g2.setFont(u8g2_font_unifont_t_symbols);
          u8g2.drawGlyph(95, 1, 0x2600);                                                 // 开灯
          // 绘制灯光参数
          u8g2.setFont(u8g2_font_t0_11_tf);
          u8g2.setFontDirection(0);
          u8g2.setCursor(70,9);
          u8g2.print(ledcRead(channel)*100/4095);                                       // 调光百分比
     }else{
         u8g2.setFont(u8g2_font_unifont_t_symbols);
         u8g2.drawGlyph(97, 1, 0x25cf);                                                 //  关灯  
          // 绘制灯光参数
          u8g2.setFont(u8g2_font_t0_11_tf);
          u8g2.setFontDirection(0);
          u8g2.setCursor(72,9);
          u8g2.print(ledcRead(channel)*100/4095);
     } 

     //绘制输入输出图标
     u8g2.drawCircle(6,19,5);                                                         // 画半径为5的空心圆（输入）
     u8g2.drawLine(6,18,6,22);
     u8g2.drawPixel(6,16);

     u8g2.drawCircle(73,19,5);                                                       // 画半径为5的空心圆（输出）
     u8g2.drawLine(73,21,73,16);   
     u8g2.drawLine(73,16,71,18);   
     u8g2.drawLine(73,16,75,18);   
     
     //绘制输入、输出参数
     u8g2.setFont(u8g2_font_t0_11_tf); 
     u8g2.drawStr(3,33,"u");
     u8g2.drawStr(3,43,"i");
     u8g2.drawStr(3,51,"p");
     u8g2.drawStr(3,61,"pf");

     u8g2.drawStr(70,33,"u");
     u8g2.drawStr(70,43,"i");
     u8g2.drawStr(70,51,"p");
     u8g2.drawStr(70,62,"ef");
     
     u8g2.setFont(u8g2_font_6x10_tn);
     u8g2.setCursor(20,33);
     u8g2.print(ivolt);
     u8g2.setCursor(20,43);
     u8g2.print(icurr);
     u8g2.setCursor(20,53);
     u8g2.print(ipower);
     u8g2.setCursor(20,63);
     u8g2.print(pf);
     
     u8g2.setCursor(87,33);
     u8g2.print(ovolt);
     u8g2.setCursor(87,43);
     u8g2.print(ocurr);
     u8g2.setCursor(87,53);
     u8g2.print(opower);
     u8g2.setCursor(87,63);
     u8g2.print(effici);
     
     u8g2.sendBuffer();          // 3.transfer internal memory to the display
   
  }
