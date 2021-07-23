/*  ������ / ���������

21.01.2011:
��������� ��������� ��������������� ���������� �� ����� A3
08.02.2011:
��������� �������� �������� ������������ ���������� (� ���������� )
�����������
-------------------------------------------------------------


RA2 (pin 1) - ���� �����-������
RA3 (pin 2) - ���� ��������������� ����������
RA1 (pin 18) - ������ SK ���
RA0 (pin 17) - ������ DI ���
RA4 (pin 3) - ���� ���������� 18B20
   --BUTTONS--
RB1 (pin 7) - START/STOP TIMER
RB2 (pin 8) - UP
RB3 (PIN 9) - DOWN
RB4 (PIN10) - MODE
*/


//function prototypes
void digout();
void beep(unsigned int length );
void delay500(void);
void delay15u(void);
void timeout(void);//����� ������� �� ����������
void s_to_time(void);//��������� ������� � hh-mm-cc
void tempout(unsigned int temp2write);
void timeup(char increment);
void timedown (char decrement);


//------------------------------------------------------------

#include <built_in.h>

#define D_zero 10    //����� 0
#define D_L    13    //������ L
#define D_RL   12    //������ L ��������
#define D_F    11    //F
#define D_P    14    //P
#define D_M    15    //������ ����� -
#define blank  0    //������

#include <built_in.h>
//------------------------------------------------------------

const unsigned char eead = 0; //����� ������ ����� ������ � ������

sbit SK at RA1_bit;
sbit DI at RA0_bit;

unsigned char lcd_code [16] = {D_zero, 1, 2, 3, 4, 5, 6, 7, 8 ,9,D_L,D_RL,D_F,D_P,D_M,blank};
unsigned char display [10]; //������ ������ ������� - ����� �������� ����� 10 �������� �� ����
unsigned char intcount;//��� ��������� ������� ������� ���������� (�� ���� �������)
unsigned char hh,mm,ss;//�������� ��� ������ �����
unsigned int time_s;//������� ������� � �������� (65536 = ���� 18 �����)
unsigned int pre_time_s;//������� ������� � �������� (65536 = ���� 18 �����)
unsigned int refresh;//����� ��� �������������� ����������
//������� � ������ ����, ��� ��� ���� �������, ���� ���������
//������ �� ���� �� �������, ����� 10 ����� ���������������
//��������� � ����� ������� - � ��� ��� �� ����.
unsigned char stoppressed; //���� ��� ��������� ������� �������
unsigned char pausepressed; //���� ��� ���������� ������� �������


// for "timeout()"
unsigned char ti;
unsigned char DDC,DDA,DDB; //���������� �������
// for "digout()"
unsigned char lcode,dig;
// for "s_to_time"
unsigned int tmpr;
//��� �������� ����������
const unsigned short TEMP_RESOLUTION = 12;//���������� - ����� ���� 9..12
unsigned int temp;//�����������

//------------------------------------------------------------


void s_to_time(void){
  hh = time_s / 3600;
  tmpr = time_s % 3600;
  mm = tmpr / 60;
  ss = (tmpr % 60)% 60;

}
//------------------------------------------------------------
//����� ������ ����������� ������ ����� �� ��������� Reentrancy
//��� pic16. ����� ��� ������� ���� ������� ������� � ������ ������
//��� ������ �� ����������. ������ ����� �  ����� ����. ����� � ������ ����:
void s_to_time_int(void){
  hh = time_s / 3600;
  tmpr = time_s % 3600;
  mm = tmpr / 60;
  ss = (tmpr % 60)% 60;

}
//------------------------------------------------------------


void digout(){
  //������ ���������� ������
  lcode = lcd_code[dig];
  DI = lcode.B3; //������� ������;
  delay15u();
  SK = 0;      //����������
  delay15u();
  SK = 1;
  DI = lcode.B2;
  delay15u();
  SK = 0;
  delay15u();
  SK = 1;
  DI = lcode.B1;
  delay15u();
  SK = 0;
  delay15u();
  SK = 1;
  DI = lcode.B0;
  delay15u();
  SK = 0;
  delay15u();
  SK = 1;
  delay15u();
} //digout
//------------------------------------------------------------

//�������� 0.5 ��� � ���� ������������ ��� �������� ������
void delay500(){
  Delay_Ms(500);
}
//-------------------------------------------------------------

void delay15u(){
  Delay_Us(15);
}
//-------------------------------------------------------------

void timeout(){
   //char ti;
   //char DC,DA,DB; //���������� �������
    //�������������� �������
   display[0] = 15;//������
   display[3] = 14; // "-"
   display[6] = 14; // "-"
   display[9] = 15;//������
   
   s_to_time(); //����������� ������� �� �����
   //������ ����� - ����
    //� �������� ������� � ��������
   DDC = hh % 10;
   DDB = hh - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[1] = DDB;
   display[2] = DDC;
   //������
   DDC = mm % 10;
   DDB = mm - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[4] = DDB;
   display[5] = DDC;
   //�������
   DDC = ss % 10;
   DDB = ss - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[7] = DDB;
   display[8] = DDC;
   //��������� �������, ����������� ������
   for (ti = 0; ti <= 9; ti++) {
    dig = display[ti];
    digout();
   }
   refresh = 0;//���������� ������� �������� �����������
}
//------------------------------------------------------------
//����� ������ ����������� ������ ����� �� ��������� Reentrancy
//��� pic16. ����� ��� ������� ���� ������� ������� � ������ ������
//��� ������ �� ����������. ������ ����� �  ����� ����. ����� � ������ ����:
void timeout_int(){
   //char ti;
   //char DC,DA,DB; //���������� �������
    //�������������� �������
   display[0] = 15;//������
   display[3] = 14; // "-"
   display[6] = 14; // "-"
   display[9] = 15;//������

   s_to_time_int(); //����������� ������� �� �����
   //������ ����� - ����
    //� �������� ������� � ��������
   DDC = hh % 10;
   DDB = hh - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[1] = DDB;
   display[2] = DDC;
   //������
   DDC = mm % 10;
   DDB = mm - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[4] = DDB;
   display[5] = DDC;
   //�������
   DDC = ss % 10;
   DDB = ss - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[7] = DDB;
   display[8] = DDC;
   //��������� �������, ����������� ������
   for (ti = 0; ti <= 9; ti++) {
    dig = display[ti];
    digout();
   }
   refresh = 0;//���������� ������� �������� �����������
}
//------------------------------------------------------------


//����� �� ������� ������ � ������������
//��� ����������� ������ ����� ������ ������ ������ ���� ����� 10 ��������
void tempout(unsigned int temp2write) {
  char i;
  const unsigned short RES_SHIFT = TEMP_RESOLUTION - 8;
  char temp_whole;
  unsigned int temp_fraction;

   //�������������� �������
   display[0] = 15;//������
   display[1] = 15;//������
   //���� ��� �� ����� �������� ���������� �����, ��������
   // �� ������� "-"
   display[6] = 14;//"-"
   display[8] = 15;//������
   display[9] = 15;//������

   //��������������

  // ������� ���� ����������� �������������
  if (temp2write & 0x8000) {
    display[2] = 14;               //"-"
    temp2write = ~temp2write + 1;
  } else  display[2] = 15;         //" ";

  //��������� ����� ����� ���������� ��������� �����������
  temp_whole = temp2write >> RES_SHIFT;

  // ������������ ����� ����� ���������� temp_whole
  //������� ������ ����������
  if (temp_whole/100) display[3] = temp_whole/100; else  //�����
     display[3] = 0; //���� "0"
  //������ � ������ ������� ����������
  display[4] = (temp_whole/10)%10;  // �������� �������
  display[5] =  temp_whole%10;      // � �������

  //������ �������� ������� ����� ���������� ���������
  temp_fraction  = temp2write << (4-RES_SHIFT);
  temp_fraction &= 0x000F;
  temp_fraction *= 625;
  // ������������ ������� �����  temp_fraction
  // �������� ������ - ��� ����� "��������" �������
  //������ ������� ���� �������
  display[7] =  temp_fraction/1000;

   //������� �� �������, ����������� ������

   for (i = 0; i <= 9; i++) {
     dig = display[i];
     digout();
   }
   refresh = 0;//���������� ������� �������� �����������
}
//------------------------------------------------------------

void timeup(char increment){
    pre_time_s += increment;
    if (pre_time_s>=64800) pre_time_s = 64800;
    time_s = pre_time_s;
    timeout();
}
//------------------------------------------------------------

void timedown (char decrement){
    pre_time_s -= decrement;
    if (pre_time_s>=64810) pre_time_s = 0;
    time_s = pre_time_s;
    timeout();
}
//------------------------------------------------------------

//������ ��������� �������
void beep(unsigned int length ){
  unsigned int i;
  for (i=0;i<=length;i++){
   PORTA.B2 = 1;
   Delay_Us(120);
   PORTA.B2 = 0;
   Delay_Us(120);
  }
}
//------------------------------------------------------------

//��������� ���������� ��� ������ �������
void interrupt(){
 unsigned char i,j;
  //���� ��������� ���������� �� ������������ ������� 1
  if (PIR1.TMR1IF) {
      PIR1.TMR1IF = 0;//����� �������� ���� ���������� ����� ����
    if (stoppressed) { //���� ������ ������ ����� �� ����� �������
      T1CON = 0b00110000;  //���������� ������
      PIE1.TMR1IE = 0; //��������� ���������� �� timer1
      PORTA.B3 = 0; //��������� �������������� ����������
      time_s = pre_time_s; //��������������� ������. ���� �������
      pausepressed = 0;
      timeout_int();
    } else {
      T1CON = 0b00110000;  //���������� ������
          //����� ��������� � ���� ��������
      TMR1H = 0x0B;
      TMR1L = 0xDC;
      T1CON = 0b00110001;  //��������� ������

      //�������� ������ ������ ������ ���������� (������ �������)
      intcount++;
      if (intcount.B0 & (~pausepressed)) {
          if (time_s == 0) {//��� �������� ��� ������ ��������
            T1CON = 0b00110000;  //���������� ������
            PIE1.TMR1IE = 0; //��������� ���������� �� timer1
            pausepressed = 0;
            PORTA.B3 = 0; //��������� �������������� ����������
            timeout_int();// ��������� ����� (�� �����)
            //�������� ������ (�������������. ������������ �������� J)
            for (j=0;j<=254;j++){
               for (i=0;i<=250;i++){
                 PORTA.B2 = 1;
                 Delay_Us(110);
                 PORTA.B2 = 0;
                 Delay_Us(110);
               }
               for (i=0;i<=250;i++){
                 PORTA.B2 = 1;
                 Delay_Us(130);
                 PORTA.B2 = 0;
                 Delay_Us(130);
               }
               //������� � ���������� ������� �����-����
               //��� ����������� ������
               if  (!PORTB.B1) {
                 do {} while (!PORTB.B1);
                 pausepressed = 0;
                 break;
               }
               //���������� ������� ��� �������������� �������� ���
               //� ����� ������� (���� ��� ��� ��������)
               if ((j==50)||(j==100)||(j==150)||(j==200)) timeout_int();
            }//�������� ������
            time_s = pre_time_s; //��������������� ������. ���� �������
            stoppressed = 1;
            pausepressed = 0;
          }  else  {
              time_s--;
             } //������ �� ��������

        timeout_int();
      }//��������� �������� ���������� (�������)
    }//������ ���� �� ������
  }//���������� �� ������� 1
}
//-------------------------------------------------------------

void main() {
char *pbyte;

CMCON = 0b00000111;//�������� �����������
INTCON.GIE = 0;    //����������
INTCON.PEIE = 1;   //���������� �� ��������� ���.
INTCON.T0IE = 0;   //������. �� �������� ������� 0 ����
INTCON.INTE = 0;   //��������� ���������� �� �������� �������
INTCON.RBIE = 0;   //� ��� ����� ��� ���� ���������� ����������
PIE1.EEIE = 0;
PIE1.CMIE = 0;
PIE1.RCIE = 0;
PIE1.TXIE = 0;
PIE1.CCP1IE = 0;
PIE1.TMR2IE = 0;
PIE1.TMR1IE = 0;
//INTEDG_bit = 0; //���������� ����������� �� �����
NOT_RBPU_bit = 0; //�������� ������.��������� �� PB
TRISB = 0xFF; //����� ������  - �� ����
TRISA = 0x0;//Port A �� �����
PORTA = 0;

//������������� ������� ��� �������� ������
//���������� 16 ��������� ������
TMR2ON_bit = 0; //������ 2 ��������
//������ 1 - ������������ 8, ��������-F/4, ������ ��������
T1CON = 0b00110000;
SK = 1;
DI = 1;
refresh = 0;

//������ ������������� �� �����
//��������� - ������������ �����, ������� ������ �������� � �������
//��������� �� ���� pbyte �������� �������� ������ ����������  pre_time_s
pbyte = &pre_time_s;//������ ��������� ��������� �� ������ ���� pre_time_s;
//pre_time_s = 60;
//������ ������ �� ������ ����� � ����� ���������� pre_time_s
*pbyte = EEPROM_Read(eead);
*(pbyte+1) = EEPROM_Read(eead+1);
//������ � ���������� pre_time_s ���������� �������� �� ������

time_s = pre_time_s;

//�������� �� ������������� �������
delay500();
timeout();
stoppressed = 1;//����� ���� ���������� ��� ������ �������
pausepressed = 0;
INTCON.GIE = 1;    //��������� ����������

//+++++ ����� ������� ++++ (�� ���������).
//�������� UP � DOWN �������� ������������� ������� �
//������������� 1 ���.
//������ ����� / ���� ��������� ��� ������������� �������� ������
//�������.



 //================================================================
while (1) { //�������� ����������� ����


 //������� ������ �����/����
   if (Button(&PORTB, 1, 50, 0)) {
   beep(50);
   do{
     //���� ��� ������� �����-���� ������ up ��� down
     //�� ���������� ��������� ���������
     //���� ������������ ������ ������ MODE
     //�� ���������� ����� � ����
     if (button(&PORTB,2,100,0)) {
       beep(50);
       timeup(1);
       stoppressed = 0;}
     else
     if (button(&PORTB,3,100,0)) {
       beep(50);
       timedown(1);
       stoppressed = 0;}
     else
     if (button(&PORTB,4,100,0)) {
       beep(50);
       pre_time_s = 0;
       time_s = pre_time_s;
       stoppressed = 0;
       timeout();
     }
   
   } while (Button(&PORTB, 1, 50, 0)); //���������� ����������

     //������ ������� �������
     if (stoppressed) {
       stoppressed = 0;
       pausepressed = 0;
       //��������� ������������� ������� � EEPROM
       EEPROM_Write(eead,Lo(pre_time_s));//������� ���� �� �������� ������
       EEPROM_Write(eead+1,Hi(pre_time_s));//������� ���� �� �������� ������
       PIE1.TMR1IE = 1; //��������� ���������� �� timer1
       TMR1H = 0x0B; //�������� �����. �������� � ������
       TMR1L = 0xDC;
       T1CON = 0b00110001;  //�������� ������ 1
       PORTA.B3 = 1;//��������� �������������� ����������
     }
     else {
       stoppressed = 1;
       pausepressed = 0;
     }

   }//������ �����/����
   
   else //������� ������ UP
   if (button(&PORTB,2,100,0) && stoppressed) {
     timeup(60);
     beep(50);}

   else //������� ������ DIWN  � ������ ��������� (���������� �������)
   if (button(&PORTB,3,100,0) && stoppressed) {
     timedown(60);
     beep(50);}
     
   else //������� ������ MODE  � ������ ������� (�����)
   if ((button(&PORTB,4,50,0)==255) & (stoppressed == 0 )) {
     while (Button(&PORTB, 4, 50, 0)); //���������� ����������
     pausepressed = ~pausepressed;
     beep(200);}
     
   else //������ MODE - � ����� ������ �����������
   if (button(&PORTB,4,50,0) && stoppressed) {
     beep(50);
     do{} while (Button(&PORTB, 4, 50, 0)); //���������� ����������
     //���������� ���� ��������� �����������
     do {
      Ow_Reset(&PORTA, 4);                 // Onewire reset signal
      Ow_Write(&PORTA, 4, 0xCC);           // Issue command SKIP_ROM
      Ow_Write(&PORTA, 4, 0x44);           // Issue command CONVERT_T
      Delay_us(120);
      Ow_Reset(&PORTA, 4);
      Ow_Write(&PORTA, 4, 0xCC);           // Issue command SKIP_ROM
      Ow_Write(&PORTA, 4, 0xBE);           // Issue command READ_SCRATCHPAD
      temp =  Ow_Read(&PORTA, 4);
      temp = (Ow_Read(&PORTA, 4) << 8) + temp;
      //����� ����������� �� ���
      tempout(temp);
      Delay500();
      
     } while (~button(&PORTB,4,50,0));//�� ���������� ������� MODE
       do{} while (Button(&PORTB, 4, 50, 0)); //���������� ����������
       beep(50);
       timeout(); //������ �������
        //� ��������� � ����� �������
   }

//================================================================

 //���� ��������� ������ �� ������� �� ����� �� ������� refresh
 //�������� ��������� �� ������� �������� (��������� ������� �����)
 //����� �����������, ��������� �������, ������������ ��� ����������������
 //����� � ����� �������(�� ��� �������)
 //refresh ������������ ���������� � ������������ ������ �� �����.
   refresh ++;
   if (refresh == 28000) timeout();//�������� ��������� ����������������
}// ����� ��������� ������������ �����


}// main()