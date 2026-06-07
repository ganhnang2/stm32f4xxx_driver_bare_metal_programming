/*

*/

#include "stm32f407xx.h"
#include <string.h>
#include <stdio.h>

extern void initialise_monitor_handles(void);

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
GPIO_Handle_t gpioLED, receivingButton, sendingButton;

volatile uint8_t isDataAvailable;			

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
	initialise_monitor_handles();
	
	uint8_t commandCode;
	uint8_t len;
	uint8_t buffer[MAX_BUFFER];
	
	GPIOPin_Init();
	I2C2_GPIOInit();
	I2C2_Init();
	
	// Enable the I2C2 peripheral
	I2C_PeripheralControl(I2C2, ENABLE);
	
	// Enable ACK bit after enabling I2C peripheral
	I2C_ManageAck(i2cHandle.pI2Cx, I2C_ACK_ENABLE);
	
	/***************************Loop*************************/
	while(1)
	{
		// Wait until button is pressed
		while (!isDataAvailable);
		delay(50000);
		
		printf("Application is running!\n");
		
		commandCode = 0x51;
		// Send the command 0x51 to receive the length of data
		I2C_Master_SendData(&i2cHandle, &commandCode, 1, SLAVE_ADDRESS);
		delay(15000);
		I2C_Master_ReceiveData(&i2cHandle, &len, 1, SLAVE_ADDRESS);
		
		// Ngăn chặn Buffer Overflow (Tràn bộ đệm) nếu ESP32 gửi len quá lớn
		if (len > MAX_BUFFER - 1) {
			len = MAX_BUFFER - 1;
		}
		
		commandCode = 0x52;
		// Send the command 0x52 to receive the data
		I2C_Master_SendData(&i2cHandle, &commandCode, 1, SLAVE_ADDRESS);
		delay(15000);
		I2C_Master_ReceiveData(&i2cHandle, (uint8_t*)buffer, len, SLAVE_ADDRESS);
		
		// Thêm ký tự kết thúc chuỗi (NULL) để đảm bảo printf an toàn
		buffer[len] = '\0';
		isDataAvailable = 0;
		
		printf("Length of data: %u\n", len);
		printf("Bytes: %s\n", buffer);
	}
}

// The interrupt handler for gpio pin
void EXTI3_IRQHandler(void)
{
	GPIO_IRQHandling(GPIO_PIN_NO_3);
	GPIO_WriteToOutputPin(gpioLED.pGPIOx, gpioLED.GPIO_PinConfig.GPIO_PinNumber, 1);
	isDataAvailable = 1;
}
