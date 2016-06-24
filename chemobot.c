/*
 * chemobot.c
 *
 * Created: 20.06.2016 20:43:44
 * Author: Falcon
 */

#include <io.h>

  #asm
     .equ __w1_port=0x0B
     .equ __w1_bit=4
  #endasm

#include <ds18b20.h>
#include <delay.h>
#include <stdio.h>

// Voltage Reference: Int., cap. on AREF
#define ADC_VREF_TYPE ((1<<REFS1) | (1<<REFS0) | (0<<ADLAR))

#define RELAY_1_ON  PORTD.2=0
#define RELAY_1_OFF PORTD.2=1
#define RELAY_2_ON  PORTD.3=0
#define RELAY_2_OFF PORTD.3=1
#define RELAY_1_DDR DDRD.2
#define RELAY_2_DDR DDRD.3
#define BEEP_GND_PORT PORTD.5
#define BEEP_GND_DDR DDRD.5
#define BEEP PORTD.6
#define BEEP_DDR DDRD.5=DDRD.6

/* maximum number of DS18B20 connected to the 1 Wire bus */
#define MAX_DEVICES 1

/* DS18B20 devices ROM code storage area */
unsigned char rom_code[9];

unsigned int count;

unsigned long adc;

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
ADMUX=adc_input | ADC_VREF_TYPE;
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=(1<<ADSC);
// Wait for the AD conversion to complete
while ((ADCSRA & (1<<ADIF))==0);
ADCSRA|=(1<<ADIF);
return ADCW;
}

void main(void){

unsigned char devices;
signed long temp;
int number=0;

RELAY_1_OFF;
RELAY_2_OFF;

RELAY_1_DDR=RELAY_2_DDR=BEEP_DDR=1;

UCSR0B=(1<<TXEN0);
UCSR0C=(1<<UCSZ01) | (1<<UCSZ00);
UBRR0L=0x67;

DIDR0=(1<<ADC5D) | (1<<ADC4D) | (1<<ADC3D) | (1<<ADC2D) | (1<<ADC1D) | (0<<ADC0D);
ADMUX=ADC_VREF_TYPE;
ADCSRA=(1<<ADEN) | (0<<ADSC) | (1<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);

//detect devices are connected to the 1 Wire bus
  devices=w1_search(0xf0,rom_code);
  if (devices) printf("Temp sensor detected");
  else         printf("Sensor not found");
                               
ds18b20_init(&rom_code[0],20,30,DS18B20_12BIT_RES);

RELAY_1_ON;
RELAY_2_ON;
BEEP=1;
delay_ms(1000);
BEEP=0;

 while (1){  
 int i;
 
 printf("Heating started...");
 
 while ((temp = ds18b20_temperature(&rom_code[0])) < 50){ //ждем, пока не нагреется до 30, затем начинаем снимать данные
  if (temp > 45) printf("Sample Temp = 45.0 C");
  if (temp > 40) printf("Sample Temp = 40.0 C");
  if (temp > 35) printf("Sample Temp = 35.0 C");
  if (temp > 30) printf("Sample Temp = 30.0 C");
  if (temp > 25) printf("Sample Temp = 25.0 C");
 }
 number++;
 temp = 10 * ds18b20_temperature(&rom_code[0]);
 adc=0;
 for (i=0; i<50000; i++){
 adc += read_adc(0);
 }
 adc/=50000;
 
     if((adc > 700) || (temp > 800)) {
     count++;
      if(count == 10){
      BEEP=1;
      delay_ms(200);
      BEEP=0;
      count=0;
      }
     }
 printf("1)Numb 2)Temp 3)Visc :   %2u    %u,%u    %u", number, temp/10, temp%10, adc);
   if ((temp > 900) || (adc > 1020)) {
    //RELAY_1_OFF;
    //RELAY_2_OFF;
    delay_ms(1000);
    if (temp > 900) printf("1)Numb 2)Temp 3)Visc :   %2u    00    %u", number, adc);
    if (adc > 1020) printf("1)Numb 2)Temp 3)Visc :   %2u    %u,%u    00", number, temp/10, temp%10);
    while(1){
        BEEP=1;
        delay_ms(1000);
        BEEP=0;
        delay_ms(30000);
    }
   } 
 }
}