#include <stdint.h>
#include <stdbool.h>

#include "driverlib2.h"


#define MSEC 40000 // Multiplicador para que los delays est�n en ms
//Definici�n de los pines de los botones
#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))
//Definicion estados, que utilizaremos dentro del while 1
#define reposo 0
#define ang_min 1
#define ang_max 2
#define espera 3
//Definici�n de SLEEP para poder debugear el programa
#define SLEEP SysCtlSleep()
//#define SLEEP SysCtlSleepFake()
//Valores m�ximo y m�nimo que llegar�n PWM, y ser�n utilizados en la funci�n giro
volatile int Max_pos = 4700;//4200; //3750
volatile int Min_pos = 1000; //1875

int RELOJ, PeriodoPWM, Flag_ints;

uint32_t g_ui32SysClock;
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif
//Funci�n que coloca el servo en el �ngulo deseado, introduciendo como parametro el porcentaje del rango posible.
void giro (int pos)
{
    int posicion=Min_pos+((Max_pos-Min_pos)*pos)/100;
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, posicion);   //Inicialmente, 1ms
}
//Funci�n del SLEEP fake, utilizar en caso de que d� problemas al usar el debugger
void SysCtlSleepFake(void)
{
 while(!Flag_ints);
 Flag_ints=0;
}

int contador; //Variable que utlizamos en la rutina de interrupci�n del timer para contar las veces que se activa

unsigned int tiempo=0;
//Interrupci�n del timer: contador para el modo espera, y tiempo para saber el tiempo transcurrido desde la ejecuci�n del programa
void IntTimer0(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    Flag_ints=1;
    contador ++;
    tiempo++;
}
// Variables para contar piezas A y B, as� como de horas minutos y segundos
unsigned int contA = 0, contB = 0;
int seg, min, hora;
// Funci�n que imprime los mensajes por UART y da un formato a las variables de tiempo
void imprimir (int pieza)
{
    seg = tiempo/20;
    hora = seg/3600;
    seg = seg%3600;
    min = seg/60;
    seg = seg%60;
    if (pieza == 0) UARTprintf("\n [%02d:%02d:%02d] Pieza tipo A detectada",hora, min, seg );
    else if (pieza == 1) UARTprintf("\n [%02d:%02d:%02d] Pieza tipo B detectada",hora, min, seg);
    UARTprintf("\n%02d piezas tipo A / %02d piezas tipo B", contA, contB);
}
int main(void)
 {   //Inicializaci�n de la variable estado para la m�quina de estados
    int estado = reposo;

    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    //Activaci�n de los pines necesarios
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);   //J0 y J1: entradas
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1


    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);       //Habilita T0
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);   //T0 a 120MHz
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);    //T0 periodico y conjunto (32b)
    TimerLoadSet(TIMER0_BASE, TIMER_A, RELOJ/20 -1);
    TimerIntRegister(TIMER0_BASE,TIMER_A,IntTimer0);
    IntEnable(INT_TIMER0A); //Habilitar las interrupciones globales de los timers
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);    // Habilitar las interrupciones de timeout
    IntMasterEnable();  //Habilitacion global de interrupciones
    TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitar Timer0, 1, 2A y 2B

    PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_64);   // al PWM le llega un reloj de 1.875MHz

    GPIOPinConfigure(GPIO_PG0_M0PWM4);          //Configurar el pin a PWM
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_0);

    //Configurar el PWM4, contador descendente y sin sincronizaci�n (actualizaci�n autom�tica)
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    PeriodoPWM=37499; // 50Hz  a 1.875MHz
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, PeriodoPWM); //frec:50Hz
    giro(50); //Posicion inicial del servo
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);     //Habilita el generador 0
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT , true);    //Habilita la salida 1
    //Activaci�n de los perif�ricos necesarios durante el modo SLEEP
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_TIMER0);


    //Configuraci�n del reloj para la UART
    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    //Inicializaci�n
    //Configuraci�n para el uso de la UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig(0, 115200, g_ui32SysClock);

    while(1){

        SLEEP;
        //M�quina de estados
      switch(estado)
        {
        case reposo: //En el caso de que estemos en la posici�n inicial de reposo
            if(B1_ON) estado = ang_min;//Pulsando el bot�n 1 el servo se mueve a la izquierda
            else if (B2_ON) estado = ang_max; //Pulsando el bot�n 2 el servo se mueve a la derecha
            contador = 0;
            break;

        case ang_min: //En el caso de que estemos en la posici�n izquierda, de �ngulo m�nimo
            giro (0);
            //SysCtlDelay (1000*MSEC);
            estado = espera;
            contador = 0;
            contA++; //Aumentamos el contador de piezas A
            imprimir(0);//Llamada a la funci�n
            break;
        case ang_max: //En el caso de que estemos en la posici�n derecha, de �ngulo m�ximo
            giro (100);
            //SysCtlDelay (1000*MSEC);
            estado = espera;
            contador = 0;
            contB++; //Aumentamos el contador de piezas B
            imprimir(1);//Llamada a la funci�n
            break;
        case espera:
            if (contador >= 20)
            {
                estado = reposo;
                giro(50);
            }
        }


    }
}


