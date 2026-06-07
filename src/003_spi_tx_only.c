/*
	This sketch has been implemented for transmitting string "Hello World!" to Arduino Nano through SPI protocol
*/

#include "stm32f407xx.h"
#include <string.h>

/*
 * PB14 --> SPI2_MISO
 * PB15 --> SPI2_MOSI
 * PB13 -> SPI2_SCLK
 * PB12 --> SPI2_NSS
 * ALT function mode : 5
 */

void delay(volatile uint32_t count) {
    while(count--) {
        /* Do nothing */
    }
}
 
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
	SPI_Handle_t SPI2Handle;
	
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

// Initialize three gpio pins for button ,led
// button: To start sending data from STM32 to Arduino Nano
// led: To display the state of the transmission (ON: Sending data, OFF: Finishing the tranmission)
// CS pin: To signal the slave that the tranmission is readvoid GPIOPin_Init()
void GPIOPin_Init()
{
	// 1. Configure gpioLED, gpioButton
	GPIO_Handle_t gpioLED, gpioButton;
	gpioLED.pGPIOx = GPIOD;
	gpioLED.GPIO_PinConfig.GPIO_PinNumber = 12;
	gpioLED.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	gpioLED.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	gpioLED.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	gpioLED.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	
	gpioButton.pGPIOx = GPIOE;
	gpioButton.GPIO_PinConfig.GPIO_PinNumber = 4;
	gpioButton.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	gpioButton.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	gpioButton.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
	
	// 2. Initialize two objects gpioLED, gpioButton
	GPIO_Init(&gpioLED);
	GPIO_Init(&gpioButton);
	
	GPIO_WriteToOutputPin(gpioLED.pGPIOx, gpioLED.GPIO_PinConfig.GPIO_PinNumber, 0);
}

int main(void)
{
	/***************************Inits*************************/
	char str[] = "Hello. My name is Nguyen Hai Nam\n";
	
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
	
	/***************************Loop*************************/
	while(1)
	{
		// Wait until button is pressed
		while(GPIO_ReadFromInputPin(GPIOE, 4));
		
		// Debounce
		delay(500000);
		
		// Turn on LED to indicate that starting the tranmission
		GPIO_WriteToOutputPin(GPIOD, 12, 1);
		
		// Enable the SPI2 peripheral
		SPI_PeripheralControl(SPI2, ENABLE); 
		
		uint8_t dataLen = strlen(str);
		SPI_SendData(SPI2, &dataLen, 1);
		
		// To send data
		SPI_SendData(SPI2,(uint8_t*)str, dataLen);
		
		// Lets confirm SPI is not busy
		while(SPI_GetFlagStatus(SPI2, SPI_TXE_FLAG) == FLAG_RESET);
		while(SPI_GetFlagStatus(SPI2,SPI_BUSY_FLAG));
		
		// Disable SPI2 peripheral
		SPI_PeripheralControl(SPI2, DISABLE);
		
		// Turn off LED to indicate that finishing the tranmission
		GPIO_WriteToOutputPin(GPIOD, 12, 0);
	}
}