/*
    The steps to implement SPI interrupts are:
	1. Configure the GPIO pins in Alternate Function mode.
	2. Configure the interrupt for the SPIx peripheral: SPI_IRQInterruptConfig().
	3. Set the interrupt priority (optional): SPI_IRQInterruptPriority().
	4. Use the functions SPI_SendDataIT() for data transmission and SPI_ReceiveDataIT() for data reception.
	5. Implement the function that is called when an interrupt occurs: SPIx_IRQHandler().
	6. Inside this interrupt handler, call only one intermediate function: SPI_IRQHandling().
	7. Implement the SPI_ApplicationEventCallback() function. This function contains the user-defined code that 
	will be executed when an interrupt event occurs. It takes two arguments: one object of class SPI2_Handle_t and 
	AppEv: Application Events (Due to RX, TX or OVR)
*/

#include "stm32f407xx.h"
#include <string.h>
#include <stdio.h>

/*
 * PB14 --> SPI2_MISO
 * PB15 --> SPI2_MOSI
 * PB13 -> SPI2_SCLK
 * PB12 --> SPI2_NSS
 * ALT function mode : 5
 */

#define MAX_BUFFER 		100

char rcvBuffer[MAX_BUFFER];
volatile uint8_t isDataAvailable = 0;
volatile uint8_t isContinue = 0;
volatile char readByte = 0;
volatile char sendByte = 0;

SPI_Handle_t SPI2Handle;
GPIO_Handle_t gpioLED, receivingButton;

void delay(volatile uint32_t count) {
    while(count--) {
        /* Do nothing */
    }
}

extern void initialise_monitor_handles(void);
 
// Configure the GPIO pin as alternative function mode
void SPI2_GPIOInit()
{
	GPIO_Handle_t spiGPIO;
	spiGPIO.pGPIOx = GPIOB;
	spiGPIO.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	spiGPIO.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	spiGPIO.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	spiGPIO.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	spiGPIO.GPIO_PinConfig.GPIO_PinAltFuncMode = 5;
	
	// SCLK
	spiGPIO.GPIO_PinConfig.GPIO_PinNumber = 13;
	GPIO_Init(&spiGPIO);
	
	// MISO
	spiGPIO.GPIO_PinConfig.GPIO_PinNumber = 14;
	GPIO_Init(&spiGPIO);
	
	// MOSI
	spiGPIO.GPIO_PinConfig.GPIO_PinNumber = 15;
	GPIO_Init(&spiGPIO);
	
	// NSS
	spiGPIO.GPIO_PinConfig.GPIO_PinNumber = 12;
	GPIO_Init(&spiGPIO);
}

// Initialize SPI2
void SPI2_Init()
{
	SPI2Handle.pSPIx = SPI2;
	SPI2Handle.SPI_Config.SPI_DeviceMode = SPI_MODE_MASTER;
	SPI2Handle.SPI_Config.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	SPI2Handle.SPI_Config.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV32;
	SPI2Handle.SPI_Config.SPI_DFF = SPI_DFF_8BITS;
	SPI2Handle.SPI_Config.SPI_CPOL = SPI_CPOL_LOW;
	SPI2Handle.SPI_Config.SPI_CPHA = SPI_CPHA_LOW;
	SPI2Handle.SPI_Config.SPI_SSM = SPI_SSM_DI;		// Hardware slave management enabled for NSS pin
	
	// NOTE: When enabling software slave management, we must set bit SSI = 1 to indicate that STM32 is in master mode.
	// If not setting SSI = 1, the program will issue the MODF error.
	// SPI_SSIConfig(SPI2Handle.pSPIx, ENABLE);

	SPI_Init(&SPI2Handle);
}

// Initialize three gpio pins for button, led
void GPIOPin_Init()
{
	// 1. Configure gpioLED, receivingButton
	gpioLED.pGPIOx = GPIOD;
	gpioLED.GPIO_PinConfig.GPIO_PinNumber = 12;
	gpioLED.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	gpioLED.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	gpioLED.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	gpioLED.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	
	// 2. Configure the interrupt mode for GPIO pins
	receivingButton.pGPIOx = GPIOE;
	receivingButton.GPIO_PinConfig.GPIO_PinNumber = 3;
	receivingButton.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;
	receivingButton.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	receivingButton.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;

	GPIO_Init(&gpioLED);
	GPIO_Init(&receivingButton);

	// 3. Configure the interrupt for the GPIO pin
	GPIO_IRQInterruptConfig(IRQ_NO_EXTI3, ENABLE);

	// 4. Set the interrupt priority
	GPIO_IRQPriorityConfig(IRQ_NO_EXTI3, NVIC_IRQ_PRIO15);

	GPIO_WriteToOutputPin(gpioLED.pGPIOx, gpioLED.GPIO_PinConfig.GPIO_PinNumber, 0);
}

int main(void)
{
	initialise_monitor_handles();

	/***************************Inits*************************/
	char message[] = "Hello. From STM32 with love!\n";
	uint8_t dataLen = strlen(message);

	uint8_t sendBuffer[MAX_BUFFER];
	memset(sendBuffer, 0, MAX_BUFFER);
	sendBuffer[0] = dataLen;
	memcpy(&sendBuffer[1], message, dataLen);

	uint16_t totalSendLen = dataLen + 1;
	uint16_t i = 0;
	uint8_t dummyByte = 0xFF;

	GPIOPin_Init();
	
	SPI2_GPIOInit();
	
	SPI2_Init();
	
	/*
	* making SSOE 1 does NSS output enable.
	* The NSS pin is automatically managed by the hardware.
	* i.e when SPE=1 , NSS will be pulled to low
	* and NSS pin will be high when SPE=0
	*/
	SPI_SSOEConfig(SPI2, ENABLE);
	
	// Configure the interrupt for the SPI2 peripheral
	SPI_IRQInterruptConfig(SPI2_IRQ_NUMBER, ENABLE);
	
	/***************************Loop*************************/
	while(1)
	{
		isContinue = 1;
		// Wait until button is pressed
		while(!isDataAvailable);

		delay(500000);

		//enable the SPI2 peripheral
		SPI_PeripheralControl(SPI2, ENABLE);

		// disable gpio interrupt pin to avoid the error
		GPIO_IRQInterruptConfig(IRQ_NO_EXTI3, DISABLE);
		
		i = 0;

		while(isContinue)
		{
			/* both fetching and sending the data byte by byte in interrupt mode */
			if (i < totalSendLen)
			{
				sendByte = sendBuffer[i];
				i++;
			}
			else
			{
				sendByte = dummyByte;
			}
			while (SPI_SendDataIT(&SPI2Handle,(uint8_t*)&sendByte,1) == SPI_BUSY_IN_TX);
			while (SPI_ReceiveDataIT(&SPI2Handle,(uint8_t*)&readByte,1) == SPI_BUSY_IN_RX);
		}

		// confirm SPI is not busy
		while( SPI_GetFlagStatus(SPI2,SPI_BUSY_FLAG));

		//Disable the SPI2 peripheral
		SPI_PeripheralControl(SPI2,DISABLE);

		printf("Rcvd data = %s\n",rcvBuffer);
		isDataAvailable = 0;

		// Clear the receiver buffer for the next reception	
		memset(rcvBuffer, 0, MAX_BUFFER);

		GPIO_IRQInterruptConfig(IRQ_NO_EXTI3, ENABLE);
	}
}

// The interrupt handler for spi peripheral
void SPI2_IRQHandler(void)
{
	SPI_IRQHandling(&SPI2Handle);
}

void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,uint8_t AppEv)
{
	static uint32_t count = 0;
	if (AppEv == SPI_EVENT_RX_CMPLT)
	{
		rcvBuffer[count++] = readByte;
		if(readByte == '\0' || (count == MAX_BUFFER))
		{
			isContinue = 0;
			rcvBuffer[count-1] = '\0';
			count = 0;
		}
	}
}

// The interrupt handler for gpio pin
void EXTI3_IRQHandler(void)
{
	GPIO_IRQHandling(GPIO_PIN_NO_3);
	GPIO_WriteToOutputPin(gpioLED.pGPIOx, gpioLED.GPIO_PinConfig.GPIO_PinNumber, 1);
	isDataAvailable = 1;
}
