#define TRIG1 10
#define ECHO1 9
#define TRIG2 8
#define ECHO2 7
#define BUZZ 5
#define MOTOR1 3
#define MOTOR2 6

float duration1,distance1; //超音波感測變數1
float duration2,distance2; //超音波感測變數2
float duration,distance; //最終感測變數
volatile int ggyy = 1;  // 使用這當 Flag 給  ISR 使用 !
volatile int gy = 0; //超過設定距離讓蜂鳴器保持低電位
int old_flag=6;  //判斷OCR1A值
int flag;

/// For Prescaler == 1024
///  1 秒 / (16 000 000 / 1024) = 1/250000 =  0.000064 sec / per cycle
/// 1 sec / 0.000064sec -1 = 15625 -1 = 15624
int myTOP = 15624;  // 1 sec when Prescaler == 1024
//// Interrupt Service Routine for TIMER1 CTC on OCR1A as TOP

ISR(TIMER1_COMPA_vect)
{
  digitalWrite( BUZZ, ggyy );    // ggyy 是 0 或 1
  ggyy = 1 - ggyy;   //  給下次進入  ISR 用
  if(gy) ggyy=0; //超過設定距離就保持低電位
}

void setup( ) {
  Serial.begin (9600);
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(BUZZ, OUTPUT); 

    
  noInterrupts(); // 禁止中斷
  TCCR1A = 0;
  TCCR1B = 0; 
  TCCR1B |= (1<<WGM12);  // CTC mode; Clear Timer on Compare
  TCCR1B |= (1<<CS10) | (1<<CS12);  // CS12 與 CS10 都是 1 表示 Prescaler 為 1024
  OCR1A = myTOP;  // TOP count for CTC, 與 prescaler 有關 (預設1秒)
  TCNT1=0;  // counter 歸零 
  TIMSK1 |= (1 << OCIE1A);  // enable CTC for TIMER1_COMPA_vect
  
}

void loop() {
  digitalWrite(TRIG1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG1, LOW);

  duration1 = pulseIn(ECHO1, HIGH); //微秒 
  
  if(duration1 >= 23280)  duration1=23280;  //防止出錯
  if(duration1 <= 58.2)  duration1=58.2;
  distance1 = duration1/58.2;

  digitalWrite(TRIG2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG2, LOW);

  duration2 = pulseIn(ECHO2, HIGH); //微秒
  
  if(duration2 >= 23280)  duration2=23280;  //防止出錯
  if(duration2 <= 58.2)  duration2=58.2; 
  distance2 = duration2/58.2;

  if(duration1>duration2) duration = duration2;
  else duration = duration1;
  
  distance = duration/58.2;

  Serial.print("distance1: ");
  Serial.print(distance1);
  Serial.print(";  distance2: ");
  Serial.print(distance2);
  Serial.print("distance: ");
  Serial.print(distance);
  Serial.print("; OCR1A: ");
  Serial.print( OCR1A );
  Serial.print("; flag: ");
  Serial.print( flag );
  Serial.println();

  if(distance > 100){ 
     gy=1;  
     analogWrite(MOTOR1,0); 
     analogWrite(MOTOR2,0);
  }//超過設定距離讓蜂鳴器保持低電位

  if(distance <= 100){
   gy=0;
   if(distance <= 100 && distance > 85)  { flag=1; analogWrite(MOTOR1,0);   analogWrite(MOTOR2,0);  }
   if(distance <= 85 && distance >  70)  { flag=2; analogWrite(MOTOR1,60);  analogWrite(MOTOR2,60); }
   if(distance <= 70 && distance  >  55) { flag=3; analogWrite(MOTOR1,110); analogWrite(MOTOR2,110);}
   if(distance <= 55 && distance  >  40) { flag=4; analogWrite(MOTOR1,170); analogWrite(MOTOR2,170);}
   if(distance <= 40)                    { flag=5; analogWrite(MOTOR1,230); analogWrite(MOTOR2,230);}           
   
   if(old_flag != flag)  //第一次old_flag=6一定不等於flag，所以會進迴圈 
   {
      digitalWrite( BUZZ, LOW );
      noInterrupts();  // 禁止中斷
      TCNT1=0;
      if(flag==1)  { OCR1A = 7500;  }
      if(flag==2)  { OCR1A = 5700;  }
      if(flag==3)  { OCR1A = 3800;  }
      if(flag==4)  { OCR1A = 1800;  }
      if(flag==5)  { OCR1A = 800;   }
    
      old_flag = flag ;    //等待old_flag 不等於 flag (距離區間有改變，需要更改OCRIA)
      interrupts();    // 允許中斷
   }
  }

}
