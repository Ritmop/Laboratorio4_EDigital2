/* 
 * File:   Prelab3_Slave.c
 * Device: PIC16F887
 * Author: Judah Pérez - 21536
 *Compiler: XC8 (v2.41)
 * 
 * Program: Slave PIC
 * Hardware:
 * 
 * Created: Aug 7, 2023
 * Last updated:
 */

/*--------------------------------- LIBRARIES --------------------------------*/
#include <xc.h>
#include "I2C.h"
#include "LCD4b.h"
#include "iocb_init.h"
/*---------------------------- CONFIGURATION BITS ----------------------------*/
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF         // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

/*----------------------- GLOBAL VARIABLES & CONSTANTS -----------------------*/
#define _XTAL_FREQ 8000000
uint8_t temperatura;
char    temp_s[] = {0,0,'\0'};  //temperatura as string
uint8_t segundos;
uint8_t minutos;
uint8_t horas;
uint8_t dia;
uint8_t mes;
uint8_t anio;
uint8_t selector;   //RTC H:M:S & D/M/Y selector
unsigned setting;   //Set time flag
unsigned sendRTC;   //Send set time to RTC
/*-------------------------------- PROTOTYPES --------------------------------*/
void setup(void);
void readPICslave(void);
void readRTCslave(void);
void printLCD(void);
void separar_digitos8(uint8_t num, char dig8[]);
void setTime(void);
/*------------------------------- RESET VECTOR -------------------------------*/

/*----------------------------- INTERRUPT VECTOR -----------------------------*/
void __interrupt() isr(void){
    if(RBIF){
        switch(PORTB){
            case 0b00011110:   //ADD           
                //Assign increment to selected parameter (H:M:S & D/M/Y)
                switch(selector){
                    case 0x00:
                        segundos++;
                        break;
                    case 0x01:
                        minutos++;
                        break;
                    case 0x02:
                        horas++;
                        break;
                    case 0x03:
                        selector = 0x04;
                        break;
                    case 0x04:
                        dia++;
                        break;
                    case 0x05:
                        mes++;
                        break;
                    case 0x06:
                        anio++;
                        break;
                }
                break;
            case 0b00011101:   //L                
                selector++;
                break;
            case 0b00011011:   //R
                selector--;
                break;
            case 0b00010111:      //Start RTC setting
                setting = 1;
                selector = 0;  //Start selector at seconds
                //Debug initial values
                segundos = 0x55;
                minutos = 0x59;
                horas = 0x23;
                dia = 0x31;
                mes = 0x12;
                anio = 0x22;
                break;
            case 0b00001111:    //Done setting
                sendRTC = setting?1:0;  //Send values to RTC only if setting has started
                break;
            default:
                
                break;
        }
        RBIF = 0;
    }
}

/*--------------------------- INTERRUPT SUBROUTINES --------------------------*/

/*---------------------------------- TABLES ----------------------------------*/

/*----------------------------------- MAIN -----------------------------------*/
int main(void) {
    setup();
    while(1){
        //Loop
        readPICslave();//Request data to slave PIC
        __delay_ms(10);
        
        if(setting)
            setTime();      //Set time on IOCB
        else
            readRTCslave();//Request data to RTC        
        __delay_ms(10);
        
        printLCD();     //Display time
        __delay_ms(10);
    }
}
/*-------------------------------- SUBROUTINES -------------------------------*/
void setup(void){
    ANSEL = 0;
    ANSELH= 0;
    
    TRISB = 0;  //I2C RTC Read
    PORTB = 0;
    
    TRISD = 0;  //LCD Output
    PORTD = 0;
    
    //OSCILLATOR CONFIG
    OSCCONbits.IRCF = 0b111;  //Internal clock frequency 8MHz
    SCS = 1;
    
    //Initialize I2C
    I2C_Master_Init(100000);
    
    //Initialize LCB 4bit mode
    Lcd_Init();
    
    //IOCB
    iocb_init(0x1F);    //RB0 - RB5 as pull-up inputs
}

void readPICslave(void){    
        I2C_Master_Start();         //Start I2C
        I2C_Master_Write(0x51);     //Select slave 0b0101_000x
        temperatura = I2C_Master_Read(0); //Read temperature from "sensor"
        I2C_Master_Stop();
}

void readRTCslave(void){
        //Seconds
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x00);         //Set register pointer to seconds
        I2C_Master_RepeatedStart();     //Repeated start 
        I2C_Master_Write(0b11010001);   //Read from RTC
        segundos = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(5);
        
        //Minutes
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x01);         //Set register pointer to minutes
        I2C_Master_RepeatedStart();     //Repeated start 
        I2C_Master_Write(0b11010001);   //Read from RTC
        minutos = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(5);
        
        //Hours
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x02);         //Set register pointer to hours
        I2C_Master_RepeatedStart();     //Repeated start 
        I2C_Master_Write(0b11010001);   //Read from RTC
        horas = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(5);
        
        //Date
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x04);         //Set register pointer to date
        I2C_Master_RepeatedStart();     //Repeated start 
        I2C_Master_Write(0b11010001);   //Read from RTC
        dia = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(5);
        
        //Month
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x05);         //Set register pointer to month
        I2C_Master_RepeatedStart();     //Repeated start 
        I2C_Master_Write(0b11010001);   //Read from RTC
        mes = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(5);
        
        //Year
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x06);         //Set register pointer to year
        I2C_Master_RepeatedStart();     //Repeated start 
        I2C_Master_Write(0b11010001);   //Read from RTC
        anio = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(5);
}

void printLCD(void){
    //Time
    Lcd_Set_Cursor(1,1);
    Lcd_Write_Char((horas>>4 & 0x0F) + 0x30);
    Lcd_Write_Char((horas & 0x0F) + 0x30);
    Lcd_Write_Char(':');
    Lcd_Write_Char((minutos>>4 & 0x0F) + 0x30);
    Lcd_Write_Char((minutos & 0x0F) + 0x30);
    Lcd_Write_Char(':');
    Lcd_Write_Char((segundos>>4 & 0x0F) + 0x30);
    Lcd_Write_Char((segundos & 0x0F) + 0x30);
    
    //Date
    Lcd_Set_Cursor(2,1);
    Lcd_Write_Char((dia>>4 & 0x0F) + 0x30);
    Lcd_Write_Char((dia & 0x0F) + 0x30);
    Lcd_Write_Char('/');
    Lcd_Write_Char((mes>>4 & 0x0F) + 0x30);
    Lcd_Write_Char((mes & 0x0F) + 0x30);
    Lcd_Write_Char('/');
    Lcd_Write_Char((anio>>4 & 0x0F) + 0x30);
    Lcd_Write_Char((anio & 0x0F) + 0x30);
    
    //Temperature
    separar_digitos8(temperatura,temp_s);
    Lcd_Set_Cursor(1,13);
    Lcd_Write_String(temp_s);
    Lcd_Write_String("'C");    
}

void separar_digitos8(uint8_t num, char dig8[]){
    uint8_t div1,div2,decenas,unidades;
    div1 = num / 10;
    unidades = num % 10;
    div2 = div1 / 10;
    decenas = div1 % 10;
    
    dig8[1] = unidades + 0x30;
    dig8[0] = decenas  + 0x30;
}

void setTime(void){
    //Increments done in IOCB
    //Check limits for selector and parameters
    if(selector == 255)
        selector = 6;
    else if (selector > 6)
        selector = 0;    
    
    if(segundos > 0x59)
        segundos = 0x00;
    else if((segundos & 0x0F) > 0x09)
        segundos = (segundos & 0xF0) + 0x10;
    
    if(minutos > 0x59)
        minutos = 0x00;
    else if((minutos & 0x0F) > 0x09)
        minutos = (minutos & 0xF0) + 0x10;
    
    if(horas > 0x23)
        horas = 0x00;
    else if((horas & 0x0F) > 0x09)
        horas = (horas & 0xF0) + 0x10;
    //horas &= 0x3F;   //24 hour format
    
    if(dia > 0x31)
        dia = 0x00;
    else if((dia & 0x0F) > 0x09)
        dia = (dia & 0xF0) + 0x10;
    
    if(mes > 0x12)
        mes = 0x00;
    else if((mes & 0x0F) > 0x09)
        mes = (mes & 0xF0) + 0x10;
    
    if(anio > 0x99)
        anio = 0x00;
    else if((anio & 0x0F) > 0x09)
        anio = (anio & 0xF0) + 0x10;
    
    //Send values to RTC
    if (sendRTC){
        //Seconds
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x00);         //Set register pointer to seconds
        I2C_Master_Write(segundos);     //Write seconds
        I2C_Master_Stop();
        __delay_ms(5);
        //Minutes
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x01);         //Set register pointer to minutes
        I2C_Master_Write(minutos);      //Write minutes
        I2C_Master_Stop();
        __delay_ms(5);
        //Hours
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x02);         //Set register pointer to hours
        I2C_Master_Write(horas);        //Write hours
        I2C_Master_Stop();
        __delay_ms(5);
        //Date
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x04);         //Set register pointer to date
        I2C_Master_Write(dia);        //Write date
        I2C_Master_Stop();
        __delay_ms(5);
        //Month
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x05);         //Set register pointer to month
        I2C_Master_Write(mes);        //Write month
        I2C_Master_Stop();
        __delay_ms(5);
        //Year
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x06);         //Set register pointer to year
        I2C_Master_Write(anio);        //Write year
        I2C_Master_Stop();
        __delay_ms(5);        
        
        setting = 0;    //Clear flags.
        sendRTC = 0;    //
    }        
}