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
uint8_t segundos;
uint8_t minutos;
uint8_t horas;
/*-------------------------------- PROTOTYPES --------------------------------*/
void setup(void);
void readPICslave(void);
void readRTCslave(void);
void printLCD(void);
/*------------------------------- RESET VECTOR -------------------------------*/

/*----------------------------- INTERRUPT VECTOR -----------------------------*/
void __interrupt() isr(void){
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
        readRTCslave();//Request data to RTC
        __delay_ms(10);
        printLCD();     //Display time
    }
}
/*-------------------------------- SUBROUTINES -------------------------------*/
void setup(void){
    ANSEL = 0;
    ANSELH= 0;
    
    TRISA = 0;  //I2C slave PIC Read
    PORTA = 0;
    
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
}

void readPICslave(void){    
        I2C_Master_Start();         //Start I2C
        I2C_Master_Write(0x51);     //Select slave 0b0101_000x
        PORTA = I2C_Master_Read(0);
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
        
        //Minutes
        I2C_Master_Start();             //Start I2C
        I2C_Master_Write(0b11010000);   //Write address to RTC DS3231
        I2C_Master_Write(0x02);         //Set register pointer to hours
        I2C_Master_RepeatedStart();     //Repeated start 
        I2C_Master_Write(0b11010001);   //Read from RTC
        horas = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(5);
}

void printLCD(void){
    //Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("Time");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_Char((horas>>4 & 0x0F) + 0x30);
    Lcd_Write_Char((horas & 0x0F) + 0x30);
    Lcd_Write_Char(':');
    Lcd_Write_Char((minutos>>4 & 0x0F) + 0x30);
    Lcd_Write_Char((minutos & 0x0F) + 0x30);
    Lcd_Write_Char(':');
    Lcd_Write_Char((segundos>>4 & 0x0F) + 0x30);
    Lcd_Write_Char((segundos & 0x0F) + 0x30);
}