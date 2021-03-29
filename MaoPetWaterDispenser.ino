
#include "wifi.h"                 //引用TuyaWiFi
#include "mcu_api.h"              //引用Tuya
#include <Adafruit_NeoPixel.h>    //引用WS2812控制库

//联网参数注册
char Device_Switch,UV_Switch,Warm_Switch,SB_Switch,WB_Switch,HOT_Switch;   //设备开关，UV杀菌开关，恒温开关，水泵开关，微波开关，加热器开关
long Temp_Set,temp_current;               //水温设置，当前水温
enum WorkMode Work_Mode;                  //设备模式
enum WaterLevel water_level;              //剩余水量档位

double Water_temp,Water_data;             //水位检测
long Water_Control = 90;                  //水泵默认速度
int SB_speed = 90;   

//EEPROM地址设定
int E_TempSet = 0;                        //设定温度EEPROM存储地址
int E_SBSpeed = 2;                        //水流大小EEPROM存储地址

//Millis定时参数设置
char Water_time = 0;     //水位检测定时
char W_time = 0;
char Temp_time = 0;      //水温检测定时
char T_time = 0;

//水温探测转换参数定义
const int sampleNumber = 10; //采样次数
const double balanceR = 10000.0; //参考电阻阻值，越精确越好
const double ADC_max = 1023.0;   // ADC10=1023,ADC12=4095
/*使用beta方程计算阻值。*/ 
const double beta = 3950.0; //商家给出的电阻对应25°C下的bata值
const double roomTemp = 298.15; //以开尔文为单位的室温25°C
const double roomTempR = 10000.0; //NTC热敏电阻在室温25°C下具有典型的电阻


//Tuya模组通讯管脚定义
SoftwareSerial mySerial(PB11, PB10); // 涂鸦模组串口通讯

//WS2812控制注册
#define WS2812_Num 1        // WS2812数量
Adafruit_NeoPixel pixels(WS2812_Num, WS2812_PIN, NEO_RGB + NEO_KHZ400);

//程序初始化
void setup() {
  wifi_protocol_init();          //WiFi协议初始化
  Serial.begin(115200);          // USB串口通讯初始化
  Serial.println("Hello world!");
  mySerial.begin(9600);          // 涂鸦模组串口通讯初始化
  mySerial.println("myserial init successful!");
  pixels.begin(); // 初始化NeoPixel带状对象
  pixels.clear();
  pinMode(BUTTON_PIN, INPUT_PULLUP); // 重置Wi-Fi按键初始化
  pinMode(LED_PIN, OUTPUT);          // Wi-Fi状态指示灯初始化
  pinMode(LED_PIN, HIGH);         
  pinMode(SW_PIN, INPUT_PULLUP);     // 水位测量初始化
  pinMode(UV_PIN, OUTPUT);           // UV灯初始化
  pinMode(SB_PIN, OUTPUT);           // 水泵初始化
  pinMode(HOT_PIN, OUTPUT);          // 加热器初始化
  pinMode(WB_PIN, OUTPUT);           // 微波探测初始化
}

//主体函数
void loop() {
  wifi_uart_service();     //涂鸦模组串口通讯
  myserialEvent();         //串口接收处理
  key_scan();              //按键配网   
  wifi_stat_WS2812();      //状态指示灯
  Water_Level_Get();       //获取水位档位
  WS2812_Control();        //WS2812指示灯控制
  Device_Work_Mode();      //设备模式选择
  NTC_Get_Temp();          //获取水温
  SB_Control();            //水泵控制
  WB_Control();            //微波传感器控制
  UV_Control();            //UV杀菌功能控制
  HOT_Control();           //加热器控制
}

// 串口接收处理
void myserialEvent() {
  if (mySerial.available()) {
    unsigned char value = (unsigned char)mySerial.read();
    uart_receive_input(value);
  }
}

//配网按键，按键按下后开始配网
void key_scan(void){
  unsigned char buttonState  = HIGH;
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW) 
  Serial.println("配网按键按下了");
  {
    delay(100);
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW)
    {
       mcu_set_wifi_mode(SMART_CONFIG);   //智能配网
       Serial.println("开始Smart配网");
    }
  }
}

//设备模式选择
void Device_Work_Mode(){
  //普通模式：开启水泵，关闭微波探测
  if (Work_Mode == normal){
    pixels.setBrightness(150); // 将亮度设置为大约2/5（最大= 255）
    WB_Switch = 0;
    SB_Switch = 1;
  }
  //夜间模式：开启水泵，关闭微波探测
  else if (Work_Mode == night){
    pixels.setBrightness(50); // 将亮度设置为大约2/5（最大= 255）
    WB_Switch = 0;
    SB_Switch = 1;
  }
  //感应模式：关闭水泵，开启微波探测，水泵由微波传感器直接控制
  else if  (Work_Mode == pir){
    pixels.setBrightness(150); // 将亮度设置为大约2/5（最大= 255）
    WB_Switch = 1;
    SB_Switch = 0;
  }
}

//10秒获取一次水温并上传水温信息到APP
void NTC_Get_Temp(){
  if ((millis() - Temp_time) >= 100){   //0.1秒增加一次计数
    Temp_time = millis();
    T_time++;
    if (T_time >= 100){   // 10秒测量一次水位
      T_time = 0;
      temp_current = readThermistor();
      mcu_dp_value_update(DPID_TEMP_CURRENT,temp_current); //当前水温温度VALUE型数据上报;
    }
  }
}
//水温获取转换
double readThermistor(){
  double rThermistor = 0; //保存热敏电阻的电阻值
  double tKelvin = 0; //以开尔文温度保存温度
  double tCelsius = 0; //以摄氏温度保存温度
  double adcAverage = 0; //保存平均电压测量值
  double adcSamplesi = 0 ;//保存当前采样值
  for(int i = 0; i<sampleNumber; i ++){ 
    adcSamplesi = analogRead(NTC_PIN); //从引脚和存储
    delay(10); //等待10毫秒
    adcAverage += adcSamplesi; //添加所有样本
  }
  adcAverage /= sampleNumber; //平均值w= sum/sampleNumber
  //Serial.println(adcAverage);
  /*公式计算热敏电阻的电阻。*/ 
  rThermistor = balanceR *((ADC_max / adcAverage) -  1); 
  tKelvin =(beta * roomTemp)/(beta +(roomTemp * log(rThermistor / roomTempR)));  
  tCelsius= tKelvin  -  272.15; //将开尔文转换为摄氏温度
  return tCelsius;
}

//感应模式下的微波传感器供电开关控制
void WB_Control(){
  if (WB_Switch == 1){
    digitalWrite(WB_PIN, HIGH);
  }
  else{
    digitalWrite(WB_PIN, LOW); 
  }
}

//恒温加热模式下的加热器状态控制
void HOT_Control(){
  //当温度开关打开且水位不为低时开启加热功能
  while (Warm_Switch == 1 && water_level != low){
    Temp_Set = EEPROM.read(E_TempSet);     //读取存储的设定温度信息
    //水温低于设定温度时开启加热器，否则关闭加热器
    if (temp_current <= Temp_Set){
      digitalWrite(HOT_PIN, HIGH);
    }
    else{
      digitalWrite(HOT_PIN, LOW);
    }
    break;
  } 

  while (Warm_Switch == 0 || water_level == low){
    digitalWrite(HOT_PIN, LOW);
    break;
  }
}
