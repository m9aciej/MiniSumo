#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "UART/uart.h"
char x ='t';

#define PWM_A    (1<<PB1)
#define PWM_B    (1<<PB2)

int pop_blad = 0,Kp=3,Kd=1; // reg PD
int czujnik0,czujnik1,czujnik2=0;
int stan = 1;
int pwmp=0,pwml=0;
int ADC4 =0;
int ADC5 =0;
int czujniki = 0;
int flaga=1;
/* FUNKCJA G£ÓWNA */
void pwm(int lewy,int prawy){
    if(lewy>255)lewy=255;
    if(prawy>255)prawy=255;
    if(lewy<-255)lewy=-255;
    if(prawy<-255)prawy=-255;


    if(lewy>0 && prawy>0)
    {
        PORTD &=~(1<<PD3); //przód
        PORTD &=~(1<<PD4);
        PORTD |=(1<<PD2);
        PORTD |=(1<<PD5);

        OCR1A = lewy;   // lewy
        OCR1B = prawy; // prawy
    }

    if(lewy<0 && prawy<0)
    {

        PORTD |= (1<<PD3);   //do tylu
        PORTD |= (1<<PD4);
        PORTD &= ~(1<<PD2) ;
        PORTD &= ~(1<<PD5) ;

        OCR1A = lewy*-1;   // lewy
        OCR1B = prawy*-1; // prawy
    }

    if(lewy>0 && prawy<0)
    {

        PORTD |= (1<<PD2);
        PORTD |= (1<<PD4);
        PORTD &= ~(1<<PD3);
        PORTD &= ~(1<<PD5); // kreci siê (prawe ko³o do ty³u, lewe do przodu)

        OCR1A = lewy;   // lewy
        OCR1B = prawy*-1; // prawy
    }

    if(lewy<0 && prawy>0)
    {

        PORTD |= (1<<PD3);
        PORTD |= (1<<PD5);
        PORTD &= ~(1<<PD2);
        PORTD &= ~(1<<PD4); // kreci siê(prawe do przodu, lewe do tyu)
        OCR1A = lewy*-1;   // lewy
        OCR1B = prawy; // prawy
    }


     pwml=lewy;pwmp=prawy;

}




int czytaj_czujniki()
{

    if(!(PINC & (1<<PINC0)))        // reakcja na czujnik nr 0
    {
        czujnik0 = 0;
    }
    else
    {
        czujnik0 = 1;
    }


    if(!(PINC & (1<<PINC2)))        // reakcja na czujnik nr 2
    {
        czujnik2 = 0;
    }
    else
    {
        czujnik2 = 1;
    }

    if(czujnik0||czujnik2)
    {
        return 1;
    }
    return 0;

}

uint16_t pomiar (uint8_t kanal){
ADMUX = (ADMUX & 0b11111000) | kanal;
ADCSRA |= (1<<ADSC); //start konwersji

  while(ADCSRA & (1<<ADSC));
return ADCW;
}


int PD(){
int blad = 0;
int left=ADC4;
int right=ADC5;


blad=-(left - right);

int rozniczka = blad - pop_blad;
pop_blad = blad;
return Kp*blad + Kd*rozniczka;
}


int main(void)

{
 uart_Init(__UBRR );
//adc

ADCSRA |= (1<<ADEN); // w³ ADC
ADCSRA |= (1<<ADPS2);// preskaler =16
ADMUX  =  (1<<REFS1) | (1<<REFS0); //REFS1:0: Reference Selection Bits

/* USTAWIANIE WYJŒÆ */
    DDRB |= (PWM_A|PWM_B);    //wyjœcia pwm

    DDRD |= (1<<PD2); //kierunek obrotów silników
    DDRD |= (1<<PD3);
    DDRD |= (1<<PD4);
    DDRD |= (1<<PD5);

    DDRD |= (1<<PD6); //dioda

   TCCR1A |= (1<<COM1A1) //Zmiana stanu wyjœcia OC1A na niski przy porównaniu A
          |  (1<<COM1B1) //Zmiana stanu wyjœcia OC1B na niski przy porównaniu B
          |  (1<<WGM11); //Tryb 14 (FAST PWM, TOP=ICR1)

   TCCR1B |= (1<<WGM13) | (1<<WGM12)  //Tryb 14 (FAST PWM, TOP=ICR1)
          |  (1<<CS10);//|_BV(CS12);  //Brak preskalera

   ICR1 = 255; //Wartoœæ maksymalna (dla trybu 14)
               //a wiêc czêstotliwoœæ = CLK/ICR1=8kHz


    OCR1A = 0;         //kana³ A = 0
    OCR1B = 0;         //kana³ B = 0


    /* PÊTLA G£ÓWNA */
   if(uart_getc()=='x'){flaga=1;}
   else{flaga=0;}

    while(1) {


    ADC4 =  pomiar(PC4);//lewy
    czujniki = czytaj_czujniki();
    ADC5 =  pomiar(PC5);//prawy
    switch(stan)
    {
         case 0:

 
         if((ADC4>260&&ADC5>260)&&czujniki==0) {
         pwm(255,255);
         }
         else if((ADC4<260&&ADC5<260)&&czujniki==0) {stan = 1;}
         else if(czujniki==1) {stan = 3;}

         break;

         case 1:

         pwm(255,-255);
         if(czujniki==1){stan=3;}
         else if((ADC5>260||ADC4>260)&&czujniki==0&&flaga)   {stan = 2;}
         else if((ADC5>260&&ADC4>260)&&czujniki==0&&flaga==0)   {stan = 0;}
         break;

         case 2:


         if((ADC4>100||ADC5>100)&&czujniki==0) {
         pwm(255+PD(),255-PD());
         }
         else if((ADC4<260&&ADC5<260)&&czujniki==0) {stan = 1;}
         else if(czujniki==1) {stan = 3;}



         break;

         case 3:


         pwm(-255,-255);

         if(czujniki==0){stan=1;}
         break;

    }

     uart_putint(stan);uart_puts(",");
     uart_putint(pwml);uart_puts(",");
     uart_putint(pwmp);uart_puts(",");
     uart_putint(ADC4);uart_puts(",");
     uart_putint(ADC5);uart_puts(",");
     uart_putint(czujnik0);uart_puts(",");
     uart_putint(czujnik2);uart_puts(".");

      if(stan==2)
      {
       PORTD |= (1<<PD6);
      }
      else{
      PORTD &= ~(1<<PD6) ;
      }



    }
}
