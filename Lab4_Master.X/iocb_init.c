#include "iocb_init.h"

//Función para inicializar IOCB
void iocb_init(uint8_t pinesB){
    TRISB |= pinesB;    //RBx as Input    
    nRBPU = 0;          //Enable PORTB pull-up's
    WPUB |= pinesB;     //Set RBx pull-up's
    RBIE = 1;           //PORTB Interrupt on Change
    IOCB |= pinesB;     //RBx Interrupt Enable
    GIE  = 1;           //Global Interrupt Enable
}
