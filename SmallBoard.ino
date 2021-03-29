
/*
 * 此为小板子上的功能定义
 * 包含： WS2812指示灯控制
 *       水位档位获取
 *       UV杀菌功能控制
 *       水泵水流控制
 */

//WiFi状态指示灯WS2812
void wifi_stat_WS2812(void)
{
  switch(mcu_get_wifi_work_state())  //获取wifi工作状态
  {
    //开始Smart配网，WS2812显示为绿色。
    case SMART_CONFIG_STATE:  
      pixels.clear(); // 将所有像素颜色设置为“关闭”
      pixels.setPixelColor(0, pixels.Color(0, 150, 0));//WS2812颜色控制
      pixels.show();   // 将更新的像素颜色发送到硬件。
      Serial.println("Smart配网中......");
      break;
    //WiFi未连接，WS2812显示为橙色。
    case WIFI_NOT_CONNECTED:  
      pixels.clear(); // 将所有像素颜色设置为“关闭”
      pixels.setPixelColor(0, pixels.Color(255, 102, 0));//WS2812颜色控制
      pixels.show();   // 将更新的像素颜色发送到硬件。
      Serial.println("WiFi未连接");
      break;  
    //WiFi已连接但未连接云。
    case WIFI_CONNECTED:  
      Serial.println("已连接到路由器");
      break;
    //配网成功，连接上云。且UV杀菌功能、加热恒温功能关闭和水位不为低的情况下，WS2812显示为淡蓝色。
    case WIFI_CONN_CLOUD:  
      if (Warm_Switch == 0 && water_level != low && UV_Switch == 0){
        pixels.clear(); // 将所有像素颜色设置为“关闭”
        pixels.setPixelColor(0, pixels.Color(51, 153, 255));//WS2812颜色控制
        pixels.show();   // 将更新的像素颜色发送到硬件。
      }
      digitalWrite(LED_PIN,LOW);
      //Serial.println("已连接到云");
      break;
    //其他情况，板载LED指示灯灯灭
    default:
      digitalWrite(LED_PIN,HIGH);
      break;
  }
}

//WS2812设备状态指示灯控制
void WS2812_Control(){
  //水量不足显示为红色。
  if (water_level == low){
    pixels.clear(); // 将所有像素颜色设置为“关闭”
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));//WS2812颜色控制为红色
    pixels.show();   // 将更新的像素颜色发送到硬件。
  }
  //UV杀菌状态显示为紫色。
  else if (UV_Switch == 1 && water_level != low){
    pixels.clear(); // 将所有像素颜色设置为“关闭”
    pixels.setPixelColor(0, pixels.Color(153, 0, 255));//WS2812颜色控制
    pixels.show();   // 将更新的像素颜色发送到硬件。
  }
  //加热状态显示为黄色。
  else if(Warm_Switch == 1 && water_level != low && UV_Switch == 0){
    pixels.clear(); // 将所有像素颜色设置为“关闭”
    pixels.setPixelColor(0, pixels.Color(255, 255, 0));//WS2812颜色控制
    pixels.show();   // 将更新的像素颜色发送到硬件。
  }
  else{
    pixels.clear(); // 将所有像素颜色设置为“关闭”
  }
  
}


//获取水位
//获取水位传感器ADC并转化为水位档位
void Water_Level_Get(){
  if ((millis() - Water_time) >= 100){   //0.11秒增加一次计数
    Water_time = millis();
    W_time++;
    if (W_time >= 100){   // 10秒测量一次水位
      W_time = 0;
      Water_temp=(long)analogRead(SW_PIN);

      //水位档位转换，并上传水位档位信息到APP
      if (Water_temp <= 500 && Water_temp > 100){
        water_level = middle;      //档位2，101-300
        mcu_dp_enum_update(DPID_WATER_LEVEL,water_level); //当前剩余水量档位，枚举型数据上报;
        Serial.println("当前水位中");
      }
      else if (Water_temp > 500){
        water_level = high;      //档位3，301-500
        mcu_dp_enum_update(DPID_WATER_LEVEL,water_level); //当前剩余水量档位，枚举型数据上报;
        Serial.println("当前水位高");
      }
      else {
        water_level = low;       //档位1，低水位<100
        mcu_dp_enum_update(DPID_WATER_LEVEL,water_level); //当前剩余水量档位，枚举型数据上报;
        Serial.println("当前水位低");
      }
      
    }
  }
}

//UV杀菌控制
void UV_Control(){
  //当UV杀菌开关打开时开启UVC灯
  if (UV_Switch  == 1){
    digitalWrite(UV_PIN, HIGH);
  }
  else{
    digitalWrite(UV_PIN, LOW);
  }
}

//水泵开关和水泵水流大小控制
void SB_Control(){
  //当水泵开关打开且水位不为低时开启水泵
  if (SB_Switch == 1 && water_level != low){
    SB_speed = EEPROM.read(E_SBSpeed);
    analogWrite(SB_PIN, SB_speed);
  }
  else{
    analogWrite(SB_PIN, 0);
  }
}
