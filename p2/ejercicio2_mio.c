#include <stdint.h>
#include <stdbool.h>

#include "driverlib2.h"
/************************************************************
 * Primer ejemplo de manejo de pines de e/s, usando el HW de la placa
 * Los pines se definen para usar los leds y botones:
 * 		LEDS: F0, F4, N0, N1
 * 		BOTONES: J0, J1
 * Cuando se pulsa (y se suelta)un bot�n, cambia de estado,
 * entre los definidos en la matriz LED. El primer bot�n incrementa el estado
 * y el segundo lo decrementa. Al llegar al final, se satura.
 ************************************************************/


#define MSEC 40000 //Valor para 1ms con SysCtlDelay()
#define MaxEst 3


#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))


int estado;

//rutina que modifica el estado al pulsar el interruptor por interrupcion
void rutina_interrupcion(void)
{
    if(B1_ON)
    {
        while(B1_ON);
        SysCtlDelay(20*MSEC);
        estado++;
        if(estado==MaxEst+1) estado=1;
        GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);
    }
    if(B2_ON)
      {
          while(B2_ON);
          SysCtlDelay(20*MSEC);
          estado--; if(estado==0) estado=3;      //Decrementa el estado. Si menor que cero, satura.
          GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_1);
      }

}


uint32_t reloj=0;

int main(void)
{

    //Fijar velocidad a 120MHz
    reloj=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    //Habilitar los perif�ricos implicados: GPIOF, J, N
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //Definir tipo de pines
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 |GPIO_PIN_4);	//F0 y F4: salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 |GPIO_PIN_1);	//N0 y N1: salidas
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);	//J0 y J1: entradas
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1

    estado=1; // Variable para los modos de funcionamientos E1, E2, E3.


//    SysCtlPeripheralClockGating(true);                      //Habilitar el apagado selectivo de perif�ricos
    GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);  //Habilitar pines de interrupci�n J0, J1
    GPIOIntRegister(GPIO_PORTJ_BASE, rutina_interrupcion);  //Registrar (definir) la rutina de interrupci�n
    IntEnable(INT_GPIOJ);                                   //Habilitar interrupci�n del pto J
    IntMasterEnable();                                      //Habilitar globalmente las ints

    while(1){
       if (estado == 1)
       {
           //encendemos todos los pines de los leds
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
           //esperamos 0.1s
           SysCtlDelay(100*MSEC);
           //apagamos todos los pines de los leds
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1*0);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0*0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4*0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0*0);
           //esperamos los 0.9s restantes
           SysCtlDelay(900*MSEC);
       }
       else if (estado == 2)
       {
           //comenzamos con todos los pines de los leds apagados
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
           SysCtlDelay(1000*MSEC);
           //comenzamos a encender cada pin cada segundo
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
           SysCtlDelay(1000*MSEC);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
           SysCtlDelay(1000*MSEC);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
           SysCtlDelay(1000*MSEC);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
           SysCtlDelay(3000*MSEC);
       }
       else if (estado == 3)
       {
           //establecemos la primera configuracion de pines (1010)
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
           //esperamos 0.5s
           SysCtlDelay(500*MSEC);
           //establecemos la segunda configuracion de pines (0101)
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
           //esperamos los 0.5s restantes
           SysCtlDelay(500*MSEC);
       }
       else
           estado == 1;

        }

}


