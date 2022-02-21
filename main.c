#include "msp.h"

char Data_In;

void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	UCB1CTLW0 |= EUSCI_B_CTLW0_SWRST;//put into software reset
	UCB1CTLW0 |= EUSCI_B_CTLW0_SSEL__SMCLK;//choose SMCLK
	UCB1BRW = 30;//set prescalar to 30
	UCB1CTLW0 |= EUSCI_B_CTLW0_MODE_3;//put into I2C mode
	UCB1CTLW0 |= EUSCI_B_CTLW0_MST;//puts into master mode
	UCB1I2CSA = 0x68;//set slave address

	UCB1TBCNT = 1;//byte counter = 1
	UCB1CTLW1 |= EUSCI_B_CTLW1_ASTP_2;//auto Stop mode

	P6SEL1 &= ~BIT5;//P6.5 = SCL
	P6SEL0 |= BIT5;//P6.5 = SCL

	P6SEL1 &= ~BIT4;//P6.5 = SDA
	P6SEL0 |= BIT4;//P6.5 = SDA

	UCB1CTLW0 &= ~EUSCI_B_CTLW0_SWRST;//take out of software reset

	//===================Setup IRQ=========================================================
	UCB1IE |= EUSCI_B_IE_TXIE0;//TX IRQ
	UCB1IE |= EUSCI_B_IE_RXIE0;//RX IRQ

	NVIC_EnableIRQ(EUSCIB1_IRQn);
	__enable_interrupt();

	while(1){
	    //transmit register address with write message
	    UCB1CTLW0 |= EUSCI_B_CTLW0_TR;//put into tx mode
	    UCB1CTLW0 |= EUSCI_B_CTLW0_TXSTT;//GENERATE START

	    while((UCB1IFG & EUSCI_B_IFG_STPIFG) == 0x0000){}
	    UCB1IFG &= ~EUSCI_B_IFG_STPIFG;//clear stop flag


	    //recieve data from slave with a read message
	    UCB1CTLW0 &= ~EUSCI_B_CTLW0_TR;//put into rx mode
        UCB1CTLW0 |= EUSCI_B_CTLW0_TXSTT;//GENERATE START

        while((UCB1IFG & EUSCI_B_IFG_STPIFG) == 0x0000){}
        UCB1IFG &= ~EUSCI_B_IFG_STPIFG;//clear stop flag

	}
}

//================================================ISR==============================================
void EUSCIB1_IRQHandler(void){
    switch(UCB1IV){
        case 0x16://RXIFG
            Data_In = UCB1RXBUF;//read data from RXBUF
            break;

        case 0x18://TXIFG
            UCB1TXBUF = 0x03;//SEND REG ADDRESS
            break;

        default:
            break;

    }
}
