/*
 * Smart Ventilator.c
 *
 * Created: 11/27/2021 1:26:16 PM
 * Author : Yesitha Sathsara
 */
#define F_CPU 8000000UL
#include <avr/io.h>
#include <stdbool.h>
#include <math.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <millis.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "i2c.h"
#include "keypad.h"
#include "string.h"



#define ISP "mobitel"
#define APN "dialog"
#include "USART.h"

bool checkStatus();

int oxygenTankPercentage();

void notifyGSM(const char *string, int percentage);

void notifyDisplay(const char *string);
void delay_ms(double ms);

bool automationOn();

void getParametersFromKnobs();

bool checkPatientTemp();

int PatientTemp();

int checkBloodOxygenLevel();



void startAirSupply();
void controlOxygenPercentage(int bloodOxygenLevel);

void notifySpeaker();

bool turnOn();

void startStepperMotor(int breathPerMin, int BreathLength);

void startOxygenAndAirSupply(int percentage);

void controlSolenoidValve(double oxygenPercentage, int breathPerMin);
void rotateFullBackward(int breathPerMin);
void rotateFullForward(int breathPerMin);




double getOxygenTankPressure();



void openSolenoidValves(double air, double oxygen);
void GSMConnect();




void sendSMS(char no[], const char *string);

const char *concatS(const char *string, char sPercentage[4]);
char *boolstring( _Bool b );
int Average_Blood_Oxygen_level=97;
int Average_Breath_length=50;
int Average_Breath_Per_Min=12;
int Oxygen_percentage=90;
char buff[160];
char status_flag = 0;
char Mobile_no[12];
volatile int buffer_pointer;
unsigned char x;
bool power;
bool OxygenAutomation;
unsigned long prev_millis0;
unsigned long need_millis0;
int case_num0;
unsigned long prev_millis1;
unsigned long need_millis1;
unsigned long case_num1;


int main(void)
{
    /* Replace with your application code */
    DDRC = DDRC | (1<<2); // solenoid valve
    DDRC = DDRC | (1<<3); // solenoid valve
    DDRC = DDRC | (1<<4); // stepper motor
    DDRC = DDRC | (1<<5); // stepper motor
    DDRC = DDRC | (1<<6); // stepper motor
    DDRC = DDRC | (1<<7); // stepper motor
	//DDRB=0x0F;            //Make PB0 to PB3 = output and PB4 to PB6=input for key pad
	 DDRB=0x8B; // 0,1,3,7--->1 2,4,5,6-->0
	init_millis(8000000UL);
	sei();
     i2c_init();
	 i2c_start();
	 i2c_write(0x70);
	 lcd_init();
    USART_Init(9600);
    _delay_ms(1000);
    GSMConnect();
	
    
	 //lcd clear funtion
	// while(!=press ok)
	 //	{
	 //	PORTB =0xF0;   //Make all columns 1 and all rows 0.
	 //	if(PINB!=0xF0)
	 //	{
	 //		x=Keypad();
	 //		LCD_Char(x);
	 //	}
	 PORTB = PORTB | (1<<4);
	 PORTB = PORTB | (1<<5);
	 PORTB = PORTB | (1<<6);
	 PORTB = PORTB & (~(1<<0));
	 PORTB = PORTB & (~(1<<1));
	 PORTB = PORTB & (~(1<<3));
	 PORTB = PORTB & (~(1<<7));
	 while(1)
		{
			
			//keypad
		
		   //(PINB&(1<<PINB5))
		
		//	
			if((PINB&(1<<PINB4))==0|(PINB&(1<<PINB5))==0|(PINB&(1<<PINB6))==0){
			x=Keypad();
				
				
		
		lcd_msg(x);
			}
	 	
		}
	 
    while (0)
    {   
		
     startOxygenAndAirSupply(60);



        if(checkStatus()){
            if(automationOn()){
                if(checkPatientTemp()){

                    if(checkBloodOxygenLevel() < Average_Blood_Oxygen_level){
                        startOxygenAndAirSupply(Oxygen_percentage);
                    }else{
                        startAirSupply();
                    }
                }
            }else{
                checkPatientTemp();
                getParametersFromKnobs();
                startOxygenAndAirSupply(Oxygen_percentage);
            }
        }else{return 0;}
    }
}
void GSMConnect(){
    USART_SendString("ATE0\r");
    _delay_ms(10);
    char data_buffer[100];
    sprintf(data_buffer,"AT+CSTT=\"%s\",\"%s\",\"%s\"\r",APN,ISP,ISP);
    USART_SendString(data_buffer);
    memset(USART_BUFFER, 0, 100);
    _delay_ms(10);
    USART_SendString("AT+SAPBR=1,1\r");
}
char *boolstring( _Bool b ) { return b ? "true" : "false"; }
void startOxygenAndAirSupply(int percentage) {
    controlOxygenPercentage(checkBloodOxygenLevel());
	controlSolenoidValve(Oxygen_percentage, Average_Breath_Per_Min);
    startStepperMotor(Average_Breath_Per_Min, Average_Breath_length);
    
}
void controlOxygenPercentage(int bloodOxygenLevel){
    //update variable Oxygen Percentage According to Blood Oxygen Level
}

void controlSolenoidValve(double oxygenPercentage, int breathPerMin) {
    double inflationTime=30.0000/breathPerMin;
    double tAir;//Air Solenoid Valve Time
    double tOxygen;//Oxygen Solenoid Valve Time
    double constValue;
    double Pressure=101325;
    double airDensity=1.225;
    double OxygenDensity=1.355;


   constValue=(0.79*(1.266*oxygenPercentage-26.67)/(100-oxygenPercentage))*sqrt(Pressure*OxygenDensity/getOxygenTankPressure()/airDensity);
   if(constValue>1){
       tOxygen=inflationTime;
       tAir=tOxygen/constValue;
       openSolenoidValves(tAir,tOxygen);// values in s
   } else{
       tAir=inflationTime;
       tOxygen=tAir*constValue;
       openSolenoidValves(tAir,tOxygen);//values in s
   }
}


void openSolenoidValves(double oxygen, double air) {
	air=air*1000;
	oxygen=oxygen*1000;
	
	
	  
	
	if(air>oxygen){
		if(prev_millis1==NULL){
			
			prev_millis1=millis();
			need_millis1=oxygen+prev_millis1;
			case_num1=1;
			PORTC = PORTC | (1<<2);  //open oxygen(normally closed valve)
			PORTC = PORTC & (~(1<<3)); //open air (normally open valve)
			case_num1++;
			}else if(need_millis1<millis()){
			
			switch(case_num1){
				
				case 2:PORTC = PORTC & (~(1<<2));case_num1++;need_millis1=need_millis1+air-oxygen;break; //close oxygen
				case 3:PORTC = PORTC | (1<<3);case_num1++;need_millis1=need_millis1+air;break;//close air
				default:case_num1=NULL;prev_millis1=NULL;
			}
			
			
			
			}else {
			if(prev_millis1==NULL){
				
				prev_millis1=millis();
				need_millis1=air+prev_millis1;
				case_num1=1;
				PORTC = PORTC | (1 << 2); //open oxygen(normally closed valve)
				PORTC = PORTC & (~(1 << 3)); //open air (normally open valve)
				case_num1++;
				}else if(need_millis1<millis()){
				
				switch(case_num1){
					
					case 2:PORTC = PORTC | (1<<3);case_num1++;need_millis1=need_millis1+oxygen-air;break; //close air
					case 3:PORTC = PORTC & (~(1<<2));case_num1++;need_millis1=need_millis1+oxygen;break;//close oxygen
					default:case_num1=NULL;prev_millis1=NULL;
				}
				
			  }

			}

		}
	}
			




double getOxygenTankPressure() {
    return 12000;//return Oxygen tank pressure in pascal
}



void startAirSupply() {
    startStepperMotor(Average_Breath_Per_Min, Average_Breath_length);
    controlSolenoidValve(100, Average_Breath_Per_Min);
}





int checkBloodOxygenLevel() {

    return 0;//return Blood Oxygen Level
}

int PatientTemp() {
    return 37;//return temperature value
}

void notifySpeaker() {

}

bool checkPatientTemp() {
    if(PatientTemp()>37.2||PatientTemp()<36.1){
        notifyGSM("Temperature Not Normal-",PatientTemp());
		char Spercentage[4];
		itoa(PatientTemp(),Spercentage,10);//convert int to string
		notifyDisplay(concatS("Temperature Not Normal-", Spercentage)); 
        notifySpeaker();
        return 0;
    }else{
        return 1;
    }
    //if normal return 1 else notify speaker and gsm
}

void getParametersFromKnobs() {

    //get values and update  Breath per min,Oxygen Percentage,Breath Length
}

bool automationOn() {
    return 1;//Check Automation On/Off
}

bool turnOn() {
    return 1;//return 1 if power on switched pressed else return 0
}

bool checkStatus() {
    if (turnOn()) {
        if (oxygenTankPercentage() < 10) {
            notifyGSM("Oxygen Tank Percentage : ", oxygenTankPercentage());
			char Spercentage[4];
			itoa(oxygenTankPercentage(),Spercentage,10);//convert int to string
            notifyDisplay(concatS("Oxygen Tank Percentage ", Spercentage));
            return 0;
        } else { return 1; }
    }else{return 0;}
}

void notifyDisplay(const char *string) {
   lcd_msg(string);
}

void notifyGSM(const char *string, int percentage) {
    char Spercentage[4];
    itoa(percentage,Spercentage,10);//convert int to string


    sendSMS(Mobile_no, concatS(string,Spercentage));
}
const char *concatS(const char *string, char sPercentage[4]) {
    char *result = malloc(strlen(string) + strlen(sPercentage) + 1);
    strcpy(result, string);
    strcat(result, sPercentage);
    return result;
}




void sendSMS(char no[], const char *string) {
    USART_SendString("AT\r");
    _delay_ms(10);
    USART_SendString("AT+CMGF=1\r");
    _delay_ms(10);
    char data_buffer[100];
    sprintf(data_buffer,"AT+CMGS=%s\r",no);
    USART_SendString(data_buffer);
    memset(USART_BUFFER, 0, 100);
    sprintf(data_buffer,"%s\r",string);
    USART_SendString(data_buffer);
    memset(USART_BUFFER, 0, 100);//clear data buffer

}

int oxygenTankPercentage() {
    return 80;
}
void startStepperMotor(int breathPerMin, int BreathLength) {
	if(prev_millis0==NULL){
		
		prev_millis0=millis();
		need_millis0=30000/(breathPerMin*10)+prev_millis0;
		case_num0=1;
		PORTC = PORTC | (1<<4);case_num0++;
		}else if(need_millis0<millis()){
		
		switch(case_num0){
			
			case 2:PORTC = PORTC | (1<<6);case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 3:PORTC = PORTC & (~(1<<4));case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 4:PORTC = PORTC | (1<<5);case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 5:PORTC = PORTC & (~(1<<6));case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 6:PORTC = PORTC | (1<<7);case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 7:PORTC = PORTC & (~(1<<5));case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 8:PORTC = PORTC | (1<<4);case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 9:PORTC = PORTC & (~(1<<7));case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 10:PORTC = PORTC & (~(1<<4));case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 11:PORTC = PORTC | (1<<4);case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 12:PORTC = PORTC | (1<<7);case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 13:PORTC = PORTC & (~(1<<4));case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 14:PORTC = PORTC | (1<<5);case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 16:PORTC = PORTC & (~(1<<7));case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 17:PORTC = PORTC | (1<<6);case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 18:PORTC = PORTC & (~(1<<5));case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 19:PORTC = PORTC & (~(1<<6));case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			case 20:PORTC = PORTC & (~(1<<4));case_num0++;need_millis0=need_millis0+30000/(breathPerMin*10);break;
			default:case_num0=NULL;prev_millis0=NULL;
		}
	}
	



	
}

