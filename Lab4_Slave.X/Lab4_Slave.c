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
#include "ADC_lib.h"
#include "I2C.h"

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
uint8_t discard;
uint8_t temperatura;
/*-------------------------------- PROTOTYPES --------------------------------*/
void setup(void);
uint8_t map(uint8_t val, uint8_t min1, uint8_t max1, uint8_t min2, uint8_t max2);
/*------------------------------- RESET VECTOR -------------------------------*/

/*----------------------------- INTERRUPT VECTOR -----------------------------*/
void __interrupt() isr(void){
    if(SSPIF){ 

        CKP = 0; //Hold clock in low to ensure data setup time
       
        if (SSPOV || WCOL ){ //Receive overflow or Write collision
            discard = SSPBUF;// Discard value by reading the buffer
            SSPOV = 0;       // Clear the overflow flag
            WCOL = 0;        // Clear the collision bit
            CKP = 1;         // Enables SCL (Clock)
        }

        if(!D_nA && !R_nW) { //Received an Address and Write
            //__delay_us(7);
            discard = SSPBUF;   // Discard address by reading the buffer
            //__delay_us(2);
            SSPIF = 0;
            CKP = 1;
            while(!BF);     // Wait to receive data
            PORTD = SSPBUF;             // Store data
            __delay_us(250);

        }else if(!D_nA && R_nW){ //Received an Address and Read
            discard = SSPBUF;   // Discard address by reading the buffer
            BF = 0;
            SSPBUF = temperatura;     //Load data to buffer
            CKP = 1;
            __delay_us(250);
            while(BF);          //Wait until buffer is cleared
        }

        SSPIF = 0;    
    }
}

/*--------------------------- INTERRUPT SUBROUTINES --------------------------*/

/*---------------------------------- TABLES ----------------------------------*/

/*----------------------------------- MAIN -----------------------------------*/
int main(void) {
    setup();
    while(1){
        //Loop
        //Capture pot val
        __delay_ms(5);
        temperatura = map(adc_read()>>8,0,255,0,70);
        PORTD = temperatura;
    }
}
/*-------------------------------- SUBROUTINES -------------------------------*/
void setup(void){
    TRISA = 255;  //RA0 as input
    PORTA = 0;
    ANSEL = 1;  //AN0 - RA0 as analog
    ANSELH= 0;
    
    TRISD = 0;
    PORTD = 0;
    
    //OSCILLATOR CONFIG
    OSCCONbits.IRCF = 0b111;  //Internal clock frequency 8MHz
    SCS = 1;
    
    //Initialize ADC. Left, Vdd/Vss, 8MHz, AN0.
    adc_init(0, 0, 8, 0);
    //Initialize I2C with address 0b0101_000x
    I2C_Slave_Init(0x50);   
}

uint8_t map(uint8_t val, uint8_t min1, uint8_t max1, uint8_t min2, uint8_t max2){
    return ((val-min1)*(max2-min2)/(max1-min1))+min2;
}