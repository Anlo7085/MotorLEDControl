
#include "F28x_Project.h"

//Hello - From Drew
//Globals

volatile struct DAC_REGS* DAC_PTR[4] = {0x0,&DacaRegs,&DacbRegs,&DaccRegs};
Uint16 dacval = 1586;
bool up = true;


int rpm;
float targetRPMF = 100.0;
int targetRPM =100;
float targetDACF;
int targetDAC;
int overDAC;
int counter1 = 0;
int counter2 = 0;
int counter3 = 0;
//unsigned long refresh_window;
float refresh_windo;
//int begin = 1;
int reset = 0;
//int stop =0;


//
// Defines
//
#define REFERENCE_VDAC      0
#define REFERENCE_VREF      1
#define DACA         1
#define DACB         2
#define DACC         3
#define REFERENCE            REFERENCE_VREF
#define DAC_NUM                DACA



__interrupt void cpu_timer0_isr(void);
__interrupt void cpu_timer1_isr(void);
__interrupt void xint1_isr(void);
void spi_fifo_init(void);
void spi_xmita(Uint16);
void spi_xmitb(Uint16);
void spi_xmitc(Uint16);
void configureDAC(Uint16 dac_num);


void main(void)

{
    InitSysCtrl();
    InitSpiaGpio();
    spi_fifo_init();
    int cl = -1;

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

		spi_xmitc(0x0000);
		spi_xmitc(0x0000);
		DELAY_US(3);

	for(cl=0;cl<88;cl++) {
		spi_xmitc(0xFFFF);
		spi_xmitc(0x0000);
		DELAY_US(3);
			}

		spi_xmitc(0xFFFF);
		spi_xmitc(0xFFFF);


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

		spi_xmitb(0x0000);
		spi_xmitb(0x0000);
		DELAY_US(3);

	for(cl=0;cl<88;cl++) {
		spi_xmitb(0xFF00);
		spi_xmitb(0x00FF);
		DELAY_US(3);
		}

		spi_xmitb(0xFFFF);
		spi_xmitb(0xFFFF);
		DELAY_US(3);

		spi_xmita(0x0000);
		spi_xmita(0x0000);
		DELAY_US(3);

	//clear leds of any data

	for(cl=0;cl<88;cl++) {
		spi_xmita(0xE000);
		spi_xmita(0x0000);
		DELAY_US(3);
	}

		spi_xmita(0xFFFF);
		spi_xmita(0xFFFF);
		DELAY_US(3);

		spi_xmita(0x0000);
		spi_xmita(0x0000);
		DELAY_US(3);

	for(cl=0;cl<88;cl++) {
		spi_xmita(0xFF00);
		spi_xmita(0x00FF);
		DELAY_US(3);
	}

		spi_xmita(0xFFFF);
		spi_xmita(0xFFFF);
		DELAY_US(3);









    InitCpuTimers();   // For this example, only initialize the Cpu Timers
    ConfigCpuTimer(&CpuTimer0, 200, 50);
    CpuTimer0Regs.TCR.all = 0x4000;

    DINT;
    InitPieCtrl();

    IER = 0x0000;
    IFR = 0x0000;

    InitPieVectTable();

    configureDAC(DAC_NUM);

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

    DAC_PTR[DAC_NUM]->DACVALS.all = dacval;
    targetDACF = (0.001388)*(targetRPMF)*(targetRPMF) +(0.6093)*(targetRPMF)+1549.0;
    targetDAC = (int) targetDACF+201;

    for(;;)
    {
    	if (reset==1)
    	{
    		DINT;
			ConfigCpuTimer(&CpuTimer1, 200, refresh_windo);
			CpuTimer1Regs.TCR.all = 0x4000;
			IER |= M_INT13;
			EINT;
			reset =0;
    	}

	}

}


__interrupt void cpu_timer0_isr(void)
{
   counter1++;
   counter3++;
   if(counter3 ==2500)
   {

	   if (dacval <targetDAC)
	   {
		   dacval++;
		   DAC_PTR[DAC_NUM]->DACVALS.all = dacval;
		   DELAY_US(2);
	   }


	   /* if(rpm < target)
	   {
		   dacval= dacval +2;
	   }
	   else if(rpm > target+5)
	   {
		   dacval--;
	   }
	   else
		   dacval = dacval + 2;
		   */



	   counter3=0;


   }

   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

__interrupt void cpu_timer1_isr(void)
{

	counter2++;
	int i;

	if (counter2==54)
		{
			spi_xmita(0x0000);
			spi_xmita(0x0000);
			DELAY_US(4);
			for (i = 0; i < 88; i++) {
				spi_xmita(0xFF00);
				spi_xmita(0x00FF);
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

		}

	if (counter2==108)
			{
				spi_xmita(0x0000);
				spi_xmita(0x0000);
				DELAY_US(4);
				for (i = 0; i < 88; i++) {
					spi_xmita(0xFF00);
					spi_xmita(0xFF00);
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

			}
	if (counter2==162)
			{
				spi_xmita(0x0000);
				spi_xmita(0x0000);
				DELAY_US(4);
				for (i = 0; i < 88; i++) {
					spi_xmita(0xFFFF);
					spi_xmita(0x0000);
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

			}
	if (counter2==216)
	{
		spi_xmita(0x0000);
		spi_xmita(0x0000);
		DELAY_US(4);
		for (i = 0; i < 88; i++) {
			spi_xmita(0xFFFF);
			spi_xmita(0xFFFF);
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

		counter2=0;
	}



	   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

 }



__interrupt void xint1_isr(void)
{
	float uts = 20000.0;											//This is the conversion from counter ticks to seconds.
	float seconds = counter1/uts; //time in seconds per rotation
	//float refresh_windo = seconds*1000000.0/216.0;   //microseconds per refresh limit (216 refreshes total)
	float rotations_per_second = 1/seconds;
	float stm = 60.0;
	float rpmf = rotations_per_second * stm; //rotations per minute.
	if(rpmf < 0 || rpmf > targetRPMF*(5));
	else
	{
		rotations_per_second = rpmf/60.0;
		refresh_windo = (1/rotations_per_second)*1000000.0/216.0;
		rpm = (int)rpmf+ 1;

	//	refresh_window = (unsigned long)refresh_windo;				//This is the number that goes to the post.
	//	if (rpm > targetRPM-10 && rpm < targetRPM + 10)
	//	{
	//		targetDAC = targetDAC-200;
	//		dacval= targetDAC;


		reset =1;


	//	}
	}
		counter1 = 0;
/*
		int i;

			spi_xmita(0x0000);
			spi_xmita(0x0000);
			DELAY_US(4);
			for (i = 0; i < 88; i++) {
				spi_xmita(0xFF00);
				spi_xmita(0x00FF);
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

*/

	//}
	//counter1 = 0;


	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}



void spi_xmita(Uint16 a)		//Transmit via SPIA
{
   SpiaRegs.SPITXBUF = a;
}

void spi_xmitb(Uint16 a)		//Transmit via SPIB
{
   SpibRegs.SPITXBUF = a;
}

void spi_xmitc(Uint16 a)		//Transmit via SPIC
{
   SpicRegs.SPITXBUF = a;
}

void spi_fifo_init()
{
   SpiaRegs.SPIFFTX.all = 0xE040;	//Initialize SPIA FIFO Registers
   SpiaRegs.SPIFFRX.all = 0x2044;
   SpiaRegs.SPIFFCT.all = 0x0;

   SpibRegs.SPIFFTX.all = 0xE040;	//Initialize SPIB FIFO Registers
   SpibRegs.SPIFFRX.all = 0x2044;
   SpibRegs.SPIFFCT.all = 0x0;

   SpicRegs.SPIFFTX.all = 0xE040;	//Initialize SPIC FIFO Registers
   SpicRegs.SPIFFRX.all = 0x2044;
   SpicRegs.SPIFFCT.all = 0x0;

   InitSpi();	//Initialize Core SPI Registers
}


//
// configureDAC - Configure specified DAC output
//
void configureDAC(Uint16 dac_num)
{
    EALLOW;
    DAC_PTR[dac_num]->DACCTL.bit.DACREFSEL = REFERENCE;
    DAC_PTR[dac_num]->DACOUTEN.bit.DACOUTEN = 1;
    DAC_PTR[dac_num]->DACVALS.all = 0;
    DELAY_US(10); // Delay for buffered DAC to power up
    EDIS;
}


//
// End of file
//
