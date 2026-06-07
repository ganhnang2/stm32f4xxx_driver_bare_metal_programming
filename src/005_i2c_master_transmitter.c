/*

*/

#include "stm32f407xx.h"
#include <string.h>

/* STM32
 * I2C_SDA: 11
 * I2C_SCL: 10
 */
 
/* ESP32
 * I2C_SDA: 21
 * I2C_SCL: 22
 */
 

#define MAX_BUFFER 		100
#define SLAVE_ADDRESS	0x08
#define STM32_ADDRESS	0x10

I2C_Handle_t i2cHandle;
GPIO_Handle_t gpioLED, receivingButton;

volatile uint8_t isDataAvailable = 0;

void delay(volatile uint32_t count) {
    while(count--) {
        /* Do nothing */
    }
}

void I2C2_GPIOInit()
{
	GPIO_Handle_t i2cGPIO;
	i2cGPIO.pGPIOx = GPIOB;
	i2cGPIO.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	i2cGPIO.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	i2cGPIO.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	i2cGPIO.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	i2cGPIO.GPIO_PinConfig.GPIO_PinAltFuncMode = 4;
	
	// SCL
	i2cGPIO.GPIO_PinConfig.GPIO_PinNumber = 10;
	GPIO_Init(&i2cGPIO);
	
	// SDA
	i2cGPIO.GPIO_PinConfig.GPIO_PinNumber = 11;
	GPIO_Init(&i2cGPIO);
}

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

void I2C2_Init()
{
	i2cHandle.pI2Cx = I2C2;
	i2cHandle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;
	i2cHandle.I2C_Config.I2C_AckControl = I2C_ACK_ENABLE;
	i2cHandle.I2C_Config.I2C_DeviceAddr = STM32_ADDRESS;
	i2cHandle.I2C_Config.I2C_AddrMode = I2C_ADDRESS_MODE_7_BITS;
	
	I2C_Init(&i2cHandle);
}

int main()
{
	/***************************Inits*************************/
	char buffer[] = "Hello. STM32 is in master transmitter mode and sending data to ESP32!";
	GPIOPin_Init();
	I2C2_GPIOInit();
	I2C2_Init();
	
	/***************************Loop*************************/
	while(1)
	{
		// Wait until button is pressed
		while (!isDataAvailable);
		delay(50000);
		
		// disable gpio interrupt pin to avoid the error
		GPIO_IRQInterruptConfig(IRQ_NO_EXTI3, DISABLE);
		
		// Enable the I2C2 peripheral
		I2C_PeripheralControl(I2C2, ENABLE);
		
		// Send data to ESP32
		I2C_Master_SendData(&i2cHandle, (uint8_t*)buffer, strlen(buffer), SLAVE_ADDRESS);
		
		GPIO_IRQInterruptConfig(IRQ_NO_EXTI3, ENABLE);

		GPIO_WriteToOutputPin(gpioLED.pGPIOx, gpioLED.GPIO_PinConfig.GPIO_PinNumber, 0);
		isDataAvailable = 0;
	}
}

// The interrupt handler for gpio pin
void EXTI3_IRQHandler(void)
{
	GPIO_IRQHandling(GPIO_PIN_NO_3);
	GPIO_WriteToOutputPin(gpioLED.pGPIOx, gpioLED.GPIO_PinConfig.GPIO_PinNumber, 1);
	isDataAvailable = 1;
}
