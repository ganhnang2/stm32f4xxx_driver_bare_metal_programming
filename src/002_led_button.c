#include "stm32f407xx.h"

/* Hàm tạo trễ đơn giản bằng vòng lặp (vì chưa dùng Timer) */
void delay(volatile uint32_t count) {
    while(count--) {
        /* Không làm gì cả, chỉ tốn thời gian CPU */
    }
}	

int main()
{
	/*----------Khởi tạo---------*/
	// 1. Cấu hình gpioLED, gpioButton
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
	
	// 2. Khởi tạo cho cả 2 đối tượng gpioLED, gpioButton
	GPIO_Init(&gpioLED);
	GPIO_Init(&gpioButton);
	
	/*---------Vòng lặp chính----------*/
	while (1)
	{
		// Đọc trạng thái từ nút chấn
		if (GPIO_ReadFromInputPin(gpioButton.pGPIOx, gpioButton.GPIO_PinConfig.GPIO_PinNumber) == 0)
		{
			delay(500000);		// Chống dội phím
			// Đảo trạng thái LED
			GPIO_ToggleOutputPin(gpioLED.pGPIOx, gpioLED.GPIO_PinConfig.GPIO_PinNumber);
		}
	}
}