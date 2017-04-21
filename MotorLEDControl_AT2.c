#include "F28x_Project.h"






//#################################################### User changes brightness value here ###################################################

#define brightnessScalar 		3

//#################################################### User changes brightness value here ###################################################




//Globals


int rpm;
int counter1 = 0;
int counter2 = 0;
float rpmS = -1;        //stable rpm
float refresh_windo;
int reset = 0;


char globalBrt = 0x1F;   // SPI Channel B set to transmit Teal
char blueA = 0xFF;
char greenA = 0xFF;
char redA = 0x00;
Uint16 greenRedA;
Uint16 pixelstartA;

char blueB = 0xFF;     // SPI Channel B set to transmit Purple
char greenB = 0x00;
char redB = 0xFF;
Uint16 greenRedB;
Uint16 pixelstartB;

char blueC = 0x00;     // SPI Channel C set to transmit Orange
char greenC = 0x0F;
char redC = 0xFF;
Uint16 greenRedC;
Uint16 pixelstartC;




__interrupt void cpu_timer0_isr(void);
__interrupt void cpu_timer1_isr(void);
__interrupt void xint1_isr(void);
void spi_fifo_init(void);
void spi_xmita(Uint16);
void spi_xmitb(Uint16);
void spi_xmitc(Uint16);



void main(void)

{
    InitSysCtrl();
    InitSpiaGpio();
    spi_fifo_init();

    blueA = blueA>> brightnessScalar;                   //Uses brightness scalar to scale the brightness of each R, G and B Byte
    greenA = greenA>> brightnessScalar;
    redA = redA>> brightnessScalar;
    greenRedA = ((greenA)<<8)|redA;                     // combines bytes into 16 bit packets
    pixelstartA = ((0b11100000|globalBrt)<<8)|blueA;

    blueB = blueB>> brightnessScalar;                  //Uses brightness scalar to scale the brightness of each R, G and B Byte
    greenB = greenB>> brightnessScalar;
    redB = redB>> brightnessScalar;
    greenRedB = ((greenB)<<8)|redB;                     // combines bytes into 16 bit packets
    pixelstartB = ((0b11100000|globalBrt)<<8)|blueB;

    blueC = blueC>> brightnessScalar;                   //Uses brightness scalar to scale the brightness of each R, G and B Byte
    greenC = greenC>> brightnessScalar;
    redC = redC>> brightnessScalar;
    greenRedC = ((greenC)<<8)|redC;                     // combines bytes into 16 bit packets
    pixelstartC = ((0b11100000|globalBrt)<<8)|blueC;

    int cl = -1;

// TEST ALL LEDS WITH STATIC DEVICE

    //A: clear all LEDs
    spi_xmita(0x0000);
    spi_xmita(0x0000);
    DELAY_US(3);

    for(cl=0;cl<88;cl++) {
        spi_xmita(0xE000);
        spi_xmita(0x0000);
        DELAY_US(3);
    }

    spi_xmita(0xFFFF);
    spi_xmita(0xFFFF);
    DELAY_US(3);
/*
    //A: set all LEDs Teal
    spi_xmita(0x0000);
    spi_xmita(0x0000);
    DELAY_US(3);

    for(cl=0;cl<88;cl++) {
        spi_xmita(pixelstartA );
        spi_xmita(greenRedA);
        DELAY_US(3);
    }

    spi_xmita(0xFFFF);
    spi_xmita(0xFFFF);
    DELAY_US(3);
*/
    //B: clear all LEDs
    spi_xmitb(0x0000);
    spi_xmitb(0x0000);
    DELAY_US(3);

    for(cl=0;cl<88;cl++) {
        spi_xmitb(0xE000);
        spi_xmitb(0x0000);
        DELAY_US(3);
    }

    spi_xmitb(0xFFFF);
    spi_xmitb(0xFFFF);
    DELAY_US(3);

    //B: set all LEDs purple
/*
    spi_xmitb(0x0000);
    spi_xmitb(0x0000);
    DELAY_US(3);

    for(cl=0;cl<88;cl++) {
        spi_xmitb(pixelstartB);
        spi_xmitb(greenRedB);
        DELAY_US(3);
    }

    spi_xmitb(0xFFFF);
    spi_xmitb(0xFFFF);
    DELAY_US(3);
*/
    //C: clear all LEDs

    spi_xmitc(0x0000);
    spi_xmitc(0x0000);
    DELAY_US(3);

    for(cl=0;cl<88;cl++) {
        spi_xmitc(0xE000);
        spi_xmitc(0x0000);
        DELAY_US(3);
    }

    spi_xmitc(0xFFFF);
    spi_xmitc(0xFFFF);
    DELAY_US(3);
/*
    //C: set all LEDs Orange
    spi_xmitc(0x0000);
    spi_xmitc(0x0000);
    DELAY_US(3);

    for(cl=0;cl<88;cl++) {
        spi_xmitc(pixelstartC);
        spi_xmitc(greenRedC);
        DELAY_US(3);
    }

    spi_xmitc(0xFFFF);
    spi_xmitc(0xFFFF);
    DELAY_US(3);
*/


//////  START ACTUAL MOTOR/LED CODE


    InitCpuTimers();   // For this example, only initialize the Cpu Timers
    ConfigCpuTimer(&CpuTimer0, 200, 50);
    CpuTimer0Regs.TCR.all = 0x4000;

    DINT;
    InitPieCtrl();

    IER = 0x0000;
    IFR = 0x0000;

    InitPieVectTable();


    EALLOW; // This is needed to write to EALLOW protected registers
    PieVectTable.TIMER1_INT = &cpu_timer1_isr;
    PieVectTable.XINT2_INT = &xint1_isr;

    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    EDIS;   // This is needed to disable write to EALLOW protected registers

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;



    EALLOW;
    GpioCtrlRegs.GPCMUX1.bit.GPIO72 = 0;         // GPIO
    GpioCtrlRegs.GPCDIR.bit.GPIO72 = 0;          // input
    GpioCtrlRegs.GPCQSEL1.bit.GPIO72 = 0;        // XINT1 Synch to SYSCLKOUT only
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    GPIO_SetupXINT2Gpio(72);
    XintRegs.XINT2CR.bit.POLARITY = 0;          // Falling edge interrupt
    XintRegs.XINT2CR.bit.ENABLE = 1;            // Enable XINT1

    IER |= M_INT3;
    IER |= M_INT1;



    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;          // Enable the PIE block
    //PieCtrlRegs.PIEIER1.bit.INTx4 = 1;
    PieCtrlRegs.PIEIER3.bit.INTx2 = 1;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    PieCtrlRegs.PIEIER1.bit.INTx5 = 1;          // Enable PIE Group 1 INT5



    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM



    for(;;)
    {
        if (reset==1)                                         // Reconfigures timer ISR which controls the rate at which the LED strips
        {													  // are updated. refresh_windo calculated in x_int1 ISR and is based on
            DINT;										      // the calculated RPM value.
            ConfigCpuTimer(&CpuTimer1, 200, refresh_windo);
            CpuTimer1Regs.TCR.all = 0x4000;
            IER |= M_INT13;
            EINT;
            counter2=0;                                       // Must be reset in order to prevent image drift
            reset =0;
        }

    }

}


__interrupt void cpu_timer0_isr(void)
{
   counter1++;



   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

__interrupt void cpu_timer1_isr(void)
{

    counter2++;
    int i;

/*
    if (counter2 >=60 && counter2 <72)                       // Displays 12 consecutive rows of LEDS in a static location
   {

       spi_xmita(0x0000);
       spi_xmita(0x0000);
       DELAY_US(4);
       for (i = 0; i < 88; i++) {
           spi_xmita(0xFF00);
           spi_xmita(0x00F0);
           DELAY_US(4);
       }
       spi_xmita(0xFFFF);
       spi_xmita(0xFFFF);
       DELAY_US(4);

       spi_xmita(0x0000);
       spi_xmita(0x0000);
       DELAY_US(4);
       for (i = 0; i < 88; i++) {
           spi_xmita(0xE000);
           spi_xmita(0x0000);
           DELAY_US(4);
       }
       spi_xmita(0xFFFF);
       spi_xmita(0xFFFF);
       DELAY_US(4);

       spi_xmitb(0x0000);
       spi_xmitb(0x0000);
       DELAY_US(4);
       for (i = 0; i < 88; i++) {
           spi_xmitb(0xFF00);
           spi_xmitb(0xFFF0);
           DELAY_US(4);
       }
       spi_xmitb(0xFFFF);
       spi_xmitb(0xFFFF);
       DELAY_US(4);

       spi_xmitb(0x0000);
       spi_xmitb(0x0000);
       DELAY_US(4);
       for (i = 0; i < 88; i++) {
           spi_xmitb(0xE000);
           spi_xmitb(0x0000);
           DELAY_US(4);
       }
       spi_xmitb(0xFFFF);
       spi_xmitb(0xFFFF);
       DELAY_US(4);


       spi_xmitc(0x0000);
       spi_xmitc(0x0000);
       DELAY_US(4);
       for (i = 0; i < 88; i++) {
           spi_xmitc(0xFF00);
           spi_xmitc(0x00F0);
           DELAY_US(4);
       }

       spi_xmitc(0xFFFF);
       spi_xmitc(0xFFFF);
       DELAY_US(4);

       spi_xmitc(0x0000);
       spi_xmitc(0x0000);
       DELAY_US(4);
       for (i = 0; i < 88; i++) {
           spi_xmitc(0xE000);
           spi_xmitc(0x0000);
           DELAY_US(4);
       }
       spi_xmitc(0xFFFF);
       spi_xmitc(0xFFFF);
       DELAY_US(4);


   }
*/
    if (counter2==30)                           // Blinks the LEDs at a specific point in the rotation
    {
        spi_xmita(0x0000);
        spi_xmita(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmita(pixelstartA);
            spi_xmita(greenRedA);
            DELAY_US(4);
        }
        spi_xmita(0xFFFF);
        spi_xmita(0xFFFF);
        DELAY_US(4);

        spi_xmita(0x0000);
        spi_xmita(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmita(0xE000);
            spi_xmita(0x0000);
            DELAY_US(4);
        }
        spi_xmita(0xFFFF);
        spi_xmita(0xFFFF);
        DELAY_US(4);

        spi_xmitb(0x0000);
        spi_xmitb(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitb(pixelstartB);
            spi_xmitb(greenRedB);
            DELAY_US(4);
        }
        spi_xmitb(0xFFFF);
        spi_xmitb(0xFFFF);
        DELAY_US(4);

        spi_xmitb(0x0000);
        spi_xmitb(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitb(0xE000);
            spi_xmitb(0x0000);
            DELAY_US(4);
        }
        spi_xmitb(0xFFFF);
        spi_xmitb(0xFFFF);
        DELAY_US(4);

        spi_xmitc(0x0000);
        spi_xmitc(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitc(pixelstartC);
            spi_xmitc(greenRedC);
            DELAY_US(4);
        }
        spi_xmitc(0xFFFF);
        spi_xmitc(0xFFFF);
        DELAY_US(4);

        spi_xmitc(0x0000);
        spi_xmitc(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitc(0xE000);
            spi_xmitc(0x0000);
            DELAY_US(4);
        }
        spi_xmitc(0xFFFF);
        spi_xmitc(0xFFFF);
        DELAY_US(4);

    }


    if (counter2==50)                                // Blinks the LEDs at a specific point in the rotation
    {
        spi_xmita(0x0000);
        spi_xmita(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmita(pixelstartA);
            spi_xmita(greenRedA);
            DELAY_US(4);
        }
        spi_xmita(0xFFFF);
        spi_xmita(0xFFFF);
        DELAY_US(4);

        spi_xmita(0x0000);
        spi_xmita(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmita(0xE000);
            spi_xmita(0x0000);
            DELAY_US(4);
        }
        spi_xmita(0xFFFF);
        spi_xmita(0xFFFF);
        DELAY_US(4);

        spi_xmitb(0x0000);
        spi_xmitb(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitb(pixelstartB);
            spi_xmitb(greenRedB);
            DELAY_US(4);
        }
        spi_xmitb(0xFFFF);
        spi_xmitb(0xFFFF);
        DELAY_US(4);

        spi_xmitb(0x0000);
        spi_xmitb(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitb(0xE000);
            spi_xmitb(0x0000);
            DELAY_US(4);
        }
        spi_xmitb(0xFFFF);
        spi_xmitb(0xFFFF);
        DELAY_US(4);

        spi_xmitc(0x0000);
        spi_xmitc(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitc(pixelstartC);
            spi_xmitc(greenRedC);
            DELAY_US(4);
        }
        spi_xmitc(0xFFFF);
        spi_xmitc(0xFFFF);
        DELAY_US(4);

        spi_xmitc(0x0000);
        spi_xmitc(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitc(0xE000);
            spi_xmitc(0x0000);
            DELAY_US(4);
        }
        spi_xmitc(0xFFFF);
        spi_xmitc(0xFFFF);
        DELAY_US(4);


    }

    if (counter2==70)                                 // Blinks the LEDs at a specific point in the rotation
    {
        spi_xmita(0x0000);
        spi_xmita(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmita(pixelstartA);
            spi_xmita(greenRedA);
            DELAY_US(4);
        }
        spi_xmita(0xFFFF);
        spi_xmita(0xFFFF);
        DELAY_US(4);

        spi_xmita(0x0000);
        spi_xmita(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmita(0xE000);
            spi_xmita(0x0000);
            DELAY_US(4);
        }
        spi_xmita(0xFFFF);
        spi_xmita(0xFFFF);
        DELAY_US(4);

        spi_xmitb(0x0000);
        spi_xmitb(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitb(pixelstartB);
            spi_xmitb(greenRedB);
            DELAY_US(4);
        }
        spi_xmitb(0xFFFF);
        spi_xmitb(0xFFFF);
        DELAY_US(4);

        spi_xmitb(0x0000);
        spi_xmitb(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitb(0xE000);
            spi_xmitb(0x0000);
            DELAY_US(4);
        }
        spi_xmitb(0xFFFF);
        spi_xmitb(0xFFFF);
        DELAY_US(4);

        spi_xmitc(0x0000);
        spi_xmitc(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitc(pixelstartC);
            spi_xmitc(greenRedC);
            DELAY_US(4);
        }
        spi_xmitc(0xFFFF);
        spi_xmitc(0xFFFF);
        DELAY_US(4);

        spi_xmitc(0x0000);
        spi_xmitc(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitc(0xE000);
            spi_xmitc(0x0000);
            DELAY_US(4);
        }
        spi_xmitc(0xFFFF);
        spi_xmitc(0xFFFF);
        DELAY_US(4);

    }


    if (counter2==90)                              // Blinks the LEDs at a specific point in the rotation
    {
        spi_xmita(0x0000);
        spi_xmita(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmita(pixelstartA);
            spi_xmita(greenRedA);
            DELAY_US(4);
        }
        spi_xmita(0xFFFF);
        spi_xmita(0xFFFF);
        DELAY_US(4);

        spi_xmita(0x0000);
        spi_xmita(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmita(0xE000);
            spi_xmita(0x0000);
            DELAY_US(4);
        }
        spi_xmita(0xFFFF);
        spi_xmita(0xFFFF);
        DELAY_US(4);

        spi_xmitb(0x0000);
        spi_xmitb(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitb(pixelstartB);
            spi_xmitb(greenRedB);
            DELAY_US(4);
        }
        spi_xmitb(0xFFFF);
        spi_xmitb(0xFFFF);
        DELAY_US(4);

        spi_xmitb(0x0000);
        spi_xmitb(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitb(0xE000);
            spi_xmitb(0x0000);
            DELAY_US(4);
        }
        spi_xmitb(0xFFFF);
        spi_xmitb(0xFFFF);
        DELAY_US(4);

        spi_xmitc(0x0000);
        spi_xmitc(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitc(pixelstartC);
            spi_xmitc(greenRedC);
            DELAY_US(4);
        }
        spi_xmitc(0xFFFF);
        spi_xmitc(0xFFFF);
        DELAY_US(4);

        spi_xmitc(0x0000);
        spi_xmitc(0x0000);
        DELAY_US(4);
        for (i = 0; i < 88; i++) {
            spi_xmitc(0xE000);
            spi_xmitc(0x0000);
            DELAY_US(4);
        }
        spi_xmitc(0xFFFF);
        spi_xmitc(0xFFFF);
        DELAY_US(4);


    }

    if (counter2==216)                       // resets counter to indicate a full rotation has occurred
    {
    	 counter2=0;

    }



       PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

 }



__interrupt void xint1_isr(void)
{
    float uts = 20000.0;                                            //This is the conversion from counter ticks to seconds.
    float seconds = counter1/uts; //time in seconds per rotation
    //float refresh_windo = seconds*1000000.0/216.0;   //microseconds per refresh limit (216 refreshes total)
    float rotations_per_second = 1/seconds;
    float stm = 60.0;
    float rpmf = rotations_per_second * stm; //rotations per minute calc, including noise.
    if (rpmS == -1 && rpmf > 60 && rpmf < 150) {   //initial set of stable RPM
        rpmS = rpmf;
    }
    if(rpmf < rpmS*.8 || rpmf > rpmS*1.2);   //throw out rpm calcs greater than 10% off previous value
    else
    {
        rpmS = rpmf;  //set most recent read to stable RPM decimal to be used
        rotations_per_second = rpmf/60.0;
        refresh_windo = (1.0/rotations_per_second)*1000000.0/216.0;
        rpm = (int)rpmf+ 1;

        reset =1;

    }


        counter1 = 0;


    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}



void spi_xmita(Uint16 a)        //Transmit via SPIA
{
   SpiaRegs.SPITXBUF = a;
}

void spi_xmitb(Uint16 a)        //Transmit via SPIB
{
   SpibRegs.SPITXBUF = a;
}

void spi_xmitc(Uint16 a)        //Transmit via SPIC
{
   SpicRegs.SPITXBUF = a;
}

void spi_fifo_init()
{
   SpiaRegs.SPIFFTX.all = 0xE040;   //Initialize SPIA FIFO Registers
   SpiaRegs.SPIFFRX.all = 0x2044;
   SpiaRegs.SPIFFCT.all = 0x0;

   SpibRegs.SPIFFTX.all = 0xE040;   //Initialize SPIB FIFO Registers
   SpibRegs.SPIFFRX.all = 0x2044;
   SpibRegs.SPIFFCT.all = 0x0;

   SpicRegs.SPIFFTX.all = 0xE040;   //Initialize SPIC FIFO Registers
   SpicRegs.SPIFFRX.all = 0x2044;
   SpicRegs.SPIFFCT.all = 0x0;

   InitSpi();   //Initialize Core SPI Registers
}



//
// End of file
//







