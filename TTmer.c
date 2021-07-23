/*  Таймер / термометр

21.01.2011:
Добавлена отработка исполнительного устройства на порту A3
08.02.2011:
Собираюсь добавить звуковую сигнализацию превышения (и пренижения )
температуры
-------------------------------------------------------------


RA2 (pin 1) - порт пъезо-звонка
RA3 (pin 2) - порт исполнительного устройства
RA1 (pin 18) - сигнал SK ЖКИ
RA0 (pin 17) - сигнал DI ЖКИ
RA4 (pin 3) - порт термометра 18B20
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
void timeout(void);//вывод времени по прерыванию
void s_to_time(void);//переводит секунды в hh-mm-cc
void tempout(unsigned int temp2write);
void timeup(char increment);
void timedown (char decrement);


//------------------------------------------------------------

#include <built_in.h>

#define D_zero 10    //Цифра 0
#define D_L    13    //Символ L
#define D_RL   12    //Символ L наоборот
#define D_F    11    //F
#define D_P    14    //P
#define D_M    15    //симфол минус -
#define blank  0    //Пробел

#include <built_in.h>
//------------------------------------------------------------

const unsigned char eead = 0; //адрес начала блока данных в еепром

sbit SK at RA1_bit;
sbit DI at RA0_bit;

unsigned char lcd_code [16] = {D_zero, 1, 2, 3, 4, 5, 6, 7, 8 ,9,D_L,D_RL,D_F,D_P,D_M,blank};
unsigned char display [10]; //МАССИВ СТРОКИ ДИСПЛЕЯ - будем выводить сразу 10 символов за цикл
unsigned char intcount;//для выделения каждого второго прерывания (то есть секунды)
unsigned char hh,mm,ss;//счетчики для вывода часов
unsigned int time_s;//счетчик таймера в секундах (65536 = макс 18 часов)
unsigned int pre_time_s;//предуст таймера в секундах (65536 = макс 18 часов)
unsigned int refresh;//нужна для периодического обновления
//дисплея в режиме мала, так как этот дисплей, если программа
//ничего на него не выводит, через 10 секнд самопроизвольно
//переходит в режим таймера - а это нам не надо.
unsigned char stoppressed; //флаг для остановки отсчета времени
unsigned char pausepressed; //флаг для прерывания отсчета времени


// for "timeout()"
unsigned char ti;
unsigned char DDC,DDA,DDB; //десятичные разряды
// for "digout()"
unsigned char lcode,dig;
// for "s_to_time"
unsigned int tmpr;
//Что касается термометра
const unsigned short TEMP_RESOLUTION = 12;//разрешение - может быть 9..12
unsigned int temp;//температура

//------------------------------------------------------------


void s_to_time(void){
  hh = time_s / 3600;
  tmpr = time_s % 3600;
  mm = tmpr / 60;
  ss = (tmpr % 60)% 60;

}
//------------------------------------------------------------
//Новая версия компилятора вообще никак не позволяет Reentrancy
//для pic16. Чтобы код работал тупо копирую функцию с другим именем
//для вызова из прерываний. Делать флаги в  цикле лень. место в памяти есть:
void s_to_time_int(void){
  hh = time_s / 3600;
  tmpr = time_s % 3600;
  mm = tmpr / 60;
  ss = (tmpr % 60)% 60;

}
//------------------------------------------------------------


void digout(){
  //Сперва декодируем данные
  lcode = lcd_code[dig];
  DI = lcode.B3; //Выводим данные;
  delay15u();
  SK = 0;      //Стробируем
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

//задержка 0.5 сек в виде потпрограммы для экономии памяти
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
   //char DC,DA,DB; //десятичные разряды
    //дополнительные символы
   display[0] = 15;//Пробел
   display[3] = 14; // "-"
   display[6] = 14; // "-"
   display[9] = 15;//Пробел
   
   s_to_time(); //преобразуем секунды во время
   //теперь цифры - часы
    //с младшего разряда к старшему
   DDC = hh % 10;
   DDB = hh - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[1] = DDB;
   display[2] = DDC;
   //Минуты
   DDC = mm % 10;
   DDB = mm - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[4] = DDB;
   display[5] = DDC;
   //Секунды
   DDC = ss % 10;
   DDB = ss - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[7] = DDB;
   display[8] = DDC;
   //Обновляем дисплей, проталкивая строку
   for (ti = 0; ti <= 9; ti++) {
    dig = display[ti];
    digout();
   }
   refresh = 0;//сбрасываем счетчик задержки бездействия
}
//------------------------------------------------------------
//Новая версия компилятора вообще никак не позволяет Reentrancy
//для pic16. Чтобы код работал тупо копирую функцию с другим именем
//для вызова из прерываний. Делать флаги в  цикле лень. место в памяти есть:
void timeout_int(){
   //char ti;
   //char DC,DA,DB; //десятичные разряды
    //дополнительные символы
   display[0] = 15;//Пробел
   display[3] = 14; // "-"
   display[6] = 14; // "-"
   display[9] = 15;//Пробел

   s_to_time_int(); //преобразуем секунды во время
   //теперь цифры - часы
    //с младшего разряда к старшему
   DDC = hh % 10;
   DDB = hh - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[1] = DDB;
   display[2] = DDC;
   //Минуты
   DDC = mm % 10;
   DDB = mm - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[4] = DDB;
   display[5] = DDC;
   //Секунды
   DDC = ss % 10;
   DDB = ss - DDC;
   //DDA = DDB / 100;
   DDB = DDB / 10;
   DDB = DDB % 10;
   display[7] = DDB;
   display[8] = DDC;
   //Обновляем дисплей, проталкивая строку
   for (ti = 0; ti <= 9; ti++) {
    dig = display[ti];
    digout();
   }
   refresh = 0;//сбрасываем счетчик задержки бездействия
}
//------------------------------------------------------------


//Вывод на дисплей строки с температурой
//Для правильного вывода длина строкм всегда должна быть равна 10 символам
void tempout(unsigned int temp2write) {
  char i;
  const unsigned short RES_SHIFT = TEMP_RESOLUTION - 8;
  char temp_whole;
  unsigned int temp_fraction;

   //дополнительные символы
   display[0] = 15;//Пробел
   display[1] = 15;//Пробел
   //Этот ЖКИ не умеет рисовать десятичную точку, заменяем
   // ее минусом "-"
   display[6] = 14;//"-"
   display[8] = 15;//Пробел
   display[9] = 15;//Пробел

   //Преобразования

  // Прверка если температура отрицательная
  if (temp2write & 0x8000) {
    display[2] = 14;               //"-"
    temp2write = ~temp2write + 1;
  } else  display[2] = 15;         //" ";

  //Выделение целой части результата измерения температуры
  temp_whole = temp2write >> RES_SHIFT;

  // Конвертируем целую часть результата temp_whole
  //Старший разряд результата
  if (temp_whole/100) display[3] = temp_whole/100; else  //Сотни
     display[3] = 0; //Либо "0"
  //Второй и третий разряды результата
  display[4] = (temp_whole/10)%10;  // ВЫделяем десятки
  display[5] =  temp_whole%10;      // И еденицы

  //Теперь выделяем дробную часть результата измерения
  temp_fraction  = temp2write << (4-RES_SHIFT);
  temp_fraction &= 0x000F;
  temp_fraction *= 625;
  // Конвертируем дробную часть  temp_fraction
  // Выделяем тысячи - они бубут "десятыми" градуса
  //Разряд десятых доле градуса
  display[7] =  temp_fraction/1000;

   //Выводим на дисплей, проталкивая строку

   for (i = 0; i <= 9; i++) {
     dig = display[i];
     digout();
   }
   refresh = 0;//сбрасываем счетчик задержки бездействия
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

//Подача звукового сигнала
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

//Обоаботка прерывания для вывода времени
void interrupt(){
 unsigned char i,j;
  //если произошло прерывание от переполнения таймера 1
  if (PIR1.TMR1IF) {
      PIR1.TMR1IF = 0;//нужно сбросить флаг прерывания иначе жопа
    if (stoppressed) { //если нажата кнопка старт во время отсчета
      T1CON = 0b00110000;  //Остановили таймер
      PIE1.TMR1IE = 0; //запретить прерывания от timer1
      PORTA.B3 = 0; //Отключаем исполнительное устройство
      time_s = pre_time_s; //восстанавливаем исходн. знач времени
      pausepressed = 0;
      timeout_int();
    } else {
      T1CON = 0b00110000;  //Остановили таймер
          //Снова загружаем в него значения
      TMR1H = 0x0B;
      TMR1L = 0xDC;
      T1CON = 0b00110001;  //Запустили таймер

      //обратный отсчет каждое второе прерывание (каждую секунду)
      intcount++;
      if (intcount.B0 & (~pausepressed)) {
          if (time_s == 0) {//это означает что отсчет закончен
            T1CON = 0b00110000;  //Остановили таймер
            PIE1.TMR1IE = 0; //запретить прерывания от timer1
            pausepressed = 0;
            PORTA.B3 = 0; //Отключаем исполнительное устройство
            timeout_int();// обновляем экран (по нулям)
            //звуковой сигнал (двухтональный. Длительность задается J)
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
               //нажатие и отпускание клавиши старт-стоп
               //для прекращения звонка
               if  (!PORTB.B1) {
                 do {} while (!PORTB.B1);
                 pausepressed = 0;
                 break;
               }
               //обновление дисплея для предотвращения перехода его
               //в режим таймера (этот ЖКИ так работает)
               if ((j==50)||(j==100)||(j==150)||(j==200)) timeout_int();
            }//Звуковой сигнал
            time_s = pre_time_s; //восстанавливаем исходн. знач времени
            stoppressed = 1;
            pausepressed = 0;
          }  else  {
              time_s--;
             } //отсчет НЕ закончен

        timeout_int();
      }//обработка рабочего прерывания (секунда)
    }//Кнопка стоп не нажата
  }//прерывание от таймера 1
}
//-------------------------------------------------------------

void main() {
char *pbyte;

CMCON = 0b00000111;//Отключим компараторы
INTCON.GIE = 0;    //прерывания
INTCON.PEIE = 1;   //Прерывания от периферии вкл.
INTCON.T0IE = 0;   //прерыв. по переполн таймера 0 откл
INTCON.INTE = 0;   //запретить прерывания от внешнего сигнала
INTCON.RBIE = 0;   //И так далее для всех оставшихся прерываний
PIE1.EEIE = 0;
PIE1.CMIE = 0;
PIE1.RCIE = 0;
PIE1.TXIE = 0;
PIE1.CCP1IE = 0;
PIE1.TMR2IE = 0;
PIE1.TMR1IE = 0;
//INTEDG_bit = 0; //Прерывание прроисходит по спаду
NOT_RBPU_bit = 0; //Включить подтяг.резисторы на PB
TRISB = 0xFF; //порты кнопок  - на ввод
TRISA = 0x0;//Port A На вывод
PORTA = 0;

//инициализация таймера для подсчета секунд
//используем 16 разрядный таймер
TMR2ON_bit = 0; //таймер 2 отключен
//таймер 1 - предделитель 8, источник-F/4, таймер отключен
T1CON = 0b00110000;
SK = 1;
DI = 1;
refresh = 0;

//Чтение предустановки из ЕПРОМ
//Установка - двухбайтовое число, функция чтения работает с байтами
//Указателю на байт pbyte присвоим значение адреса переменной  pre_time_s
pbyte = &pre_time_s;//теперь указатель указывает на первый байт pre_time_s;
//pre_time_s = 60;
//теперь читаем из ЕЕПРОМ прямо в байты переменной pre_time_s
*pbyte = EEPROM_Read(eead);
*(pbyte+1) = EEPROM_Read(eead+1);
//теперь в переменной pre_time_s содержится значение из ЕЕПРОМ

time_s = pre_time_s;

//Задержка на инициализвцию дисплея
delay500();
timeout();
stoppressed = 1;//кроме того показывает что таймер запущен
pausepressed = 0;
INTCON.GIE = 1;    //разрешили прерывания

//+++++ режим таймера ++++ (по умолчанию).
//Кнопками UP и DOWN изменяем предустановку времени с
//дискретностью 1 сек.
//Кнопка старт / стоп запускает или останавливает обратный отсчет
//времени.



 //================================================================
while (1) { //основной бесконечный цикл


 //нажатие Кнопки старт/стоп
   if (Button(&PORTB, 1, 50, 0)) {
   beep(50);
   do{
     //если при нажатой старт-стоп нажать up или down
     //то произойдет установка секундами
     //если одновременно нажать кнопку MODE
     //То произойдет сброс в ноль
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
   
   } while (Button(&PORTB, 1, 50, 0)); //Дожидаемся отпускания

     //запуск отсчета времени
     if (stoppressed) {
       stoppressed = 0;
       pausepressed = 0;
       //Сохраняем предустановеи времени в EEPROM
       EEPROM_Write(eead,Lo(pre_time_s));//младший байт по млпдшему адресу
       EEPROM_Write(eead+1,Hi(pre_time_s));//старший байт по старшему адресу
       PIE1.TMR1IE = 1; //разрешить прерывания от timer1
       TMR1H = 0x0B; //загрузка предв. значений в таймер
       TMR1L = 0xDC;
       T1CON = 0b00110001;  //запусить таймер 1
       PORTA.B3 = 1;//запускаем исполнительное устройство
     }
     else {
       stoppressed = 1;
       pausepressed = 0;
     }

   }//Кнопка старт/стоп
   
   else //нажатие кнопки UP
   if (button(&PORTB,2,100,0) && stoppressed) {
     timeup(60);
     beep(50);}

   else //нажатие кнопки DIWN  в режиме остановки (уменьшение времени)
   if (button(&PORTB,3,100,0) && stoppressed) {
     timedown(60);
     beep(50);}
     
   else //нажатие кнопки MODE  в режиме отсчета (Пауза)
   if ((button(&PORTB,4,50,0)==255) & (stoppressed == 0 )) {
     while (Button(&PORTB, 4, 50, 0)); //Дожидаемся отпускания
     pausepressed = ~pausepressed;
     beep(200);}
     
   else //кнопка MODE - в режим вывода температуры
   if (button(&PORTB,4,50,0) && stoppressed) {
     beep(50);
     do{} while (Button(&PORTB, 4, 50, 0)); //Дожидаемся отпускания
     //внутренний цикл измерения температуры
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
      //Вывод результатов на ЖКИ
      tempout(temp);
      Delay500();
      
     } while (~button(&PORTB,4,50,0));//до повторного нажатия MODE
       do{} while (Button(&PORTB, 4, 50, 0)); //Дожидаемся отпускания
       beep(50);
       timeout(); //обновл дисплей
        //и переходим в режим таймера
   }

//================================================================

 //если программа ничего не выводит на экран то счетчик refresh
 //успевает досчитать до нужного значения (подобрать опытным путем)
 //Когда досчитывает, обновляем дисплей, предотвращая его самопроизвольный
 //выход в режим таймера(он так устроен)
 //refresh автоматичеки обнуляется в подпрограмме вывода на экран.
   refresh ++;
   if (refresh == 28000) timeout();//значение подобрать эксперементально
}// конец основного бесконечного цикла


}// main()