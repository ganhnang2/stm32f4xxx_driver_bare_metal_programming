/*
 * stm32f407xx_gpio_driver.c
 *
 *  Created on: Apr 27, 2026
 *      Author: ASUS
 */

#include "stm32f407xxx_gpio_driver.h"

/****************************************************************
 * @fn			- GPIO_PeripheralClockControl
 *
 * @brief		- This function enables or disables peripheral clock for the given GPIO Port
 *
 * pGPIOx		- base addreess of the GPIO peripheral
 * EnorDi		- ENABLE or DISABLE marcos
 *
 * @return		- none
 *
 * @note		- none
 */
void GPIO_PeripheralClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		if (pGPIOx == GPIOA)
		{
			GPIOA_PCLK_EN();
		}
		else if (pGPIOx == GPIOB)
		{
			GPIOB_PCLK_EN();
		}
		else if (pGPIOx == GPIOC)
		{
			GPIOC_PCLK_EN();
		}
		else if (pGPIOx == GPIOD)
		{
			GPIOD_PCLK_EN();
		}
		else if (pGPIOx == GPIOE)
		{
			GPIOE_PCLK_EN();
		}
		else if (pGPIOx == GPIOF)
		{
			GPIOF_PCLK_EN();
		}
		else if (pGPIOx == GPIOG)
		{
			GPIOG_PCLK_EN();
		}
		else if (pGPIOx == GPIOH)
		{
			GPIOH_PCLK_EN();
		}
		else if (pGPIOx == GPIOI)
		{
			GPIOI_PCLK_EN();
		}
	}
	else
	{

	}
}

/****************************************************************
 * @fn			- GPIO_Init
 *
 * @brief		- This function enables or disables peripheral clock for the given GPIO Port
 *
 * pGPIOx		- base addreess of the GPIO peripheral
 * EnorDi		- ENABLE or DISABLE marcos
 *
 * @return		- none
 *
 * @note		- none
 */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle)
{

	// enable the peripheral clock
	GPIO_PeripheralClockControl(pGPIOHandle->pGPIOx, ENABLE);

	uint32_t temp;			// temp register
	// 1. Configure the mode of gpio pin
	if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG)
	{
		// the non-interrupt mode
		temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
		pGPIOHandle->pGPIOx->MODER &= ~(0x03 << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));		// clear bit
		pGPIOHandle->pGPIOx->MODER |= temp;			// configure the bit poisition
	}
	else
	{
		// note: The peripheral EXTI don't need to enable clock. B/c it has alway been enabled!
		// interrupt mode
		if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_FT)
		{
			//1. configure the FTSR
			EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);

			// clear the corresponding RTSR bit
			EXTI->RTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
		}
		else if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RT)
		{
			//1. configure the RTSR
			EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);

			// clear the corresponding RTSR bit
			EXTI->FTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
		}
		else if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RFT)
		{
			//1. configure both FTSR and RTSR
			EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
			EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
		}

		//2. configure the GPIO port selection in SYSCFG_EXTICR
		uint8_t temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 4;
		uint8_t temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 4;
		uint8_t portCode = GPIO_BASEADDR_TO_CODE(pGPIOHandle->pGPIOx);
		SYSCFG_PCLK_EN();		// clock enable
		SYSCFG->EXTICR[temp1] = portCode << (temp2 * 4);
		//3. enable the exti interrupt delivery using IMR
		EXTI->IMR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);

	}
	temp = 0;
	// 2. Configure the speed
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
	pGPIOHandle->pGPIOx->OSPEEDR &= ~(0x03 << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));		// clear bit
	pGPIOHandle->pGPIOx->OSPEEDR |= temp;
	temp = 0;

	// 3. Configure the pull-up, pull-down
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
	pGPIOHandle->pGPIOx->PUPDR &= ~(0x03 << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));		// clear bit
	pGPIOHandle->pGPIOx->PUPDR |= temp;
	temp = 0;

	// 4. Configure the ouput type
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinOPType << (pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
	pGPIOHandle->pGPIOx->OTYPER &= ~(0x03 << (pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));		// clear bit
	pGPIOHandle->pGPIOx->OTYPER |= temp;
	temp = 0;

	// 5. Confiugre the alternation functionality
	if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN)
	{
		// Configure the alt function registers
		uint32_t temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 8;
		uint32_t temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 8;
		pGPIOHandle->pGPIOx->AFR[temp1] &= ~(0x0F << (4 * temp2));		// clear bit
		pGPIOHandle->pGPIOx->AFR[temp1] |= pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFuncMode << (4 * temp2);
	}
}

/****************************************************************
 * @fn			- GPIO_DeInit
 *
 * @brief		-
 *
 * pGPIOx		- base addreess of the GPIO peripheral
 *
 * @return		- none
 *
 * @note		- none
 */
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx)
{
	if (pGPIOx == GPIOA)
	{
		GPIOA_REG_RESET();
	}
	else if (pGPIOx == GPIOB)
	{
		GPIOB_REG_RESET();
	}
	else if (pGPIOx == GPIOC)
	{
		GPIOC_REG_RESET();
	}
	else if (pGPIOx == GPIOD)
	{
		GPIOD_REG_RESET();
	}
	else if (pGPIOx == GPIOE)
	{
		GPIOE_REG_RESET();
	}
	else if (pGPIOx == GPIOF)
	{
		GPIOF_REG_RESET();
	}
	else if (pGPIOx == GPIOG)
	{
		GPIOG_REG_RESET();
	}
	else if (pGPIOx == GPIOH)
	{
		GPIOH_REG_RESET();
	}
	else if (pGPIOx == GPIOI)
	{
		GPIOI_REG_RESET();
	}
}

/****************************************************************
 * @fn			- GPIO_ReadFromInputPin
 *
 * @brief		-
 *
 * pGPIOx		- base addreess of the GPIO peripheral
 * pinNumber	-
 *
 * @return		- value
 *
 * @note		- none
 */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t pinNumber)
{
	uint8_t value;
	value = (uint8_t)((pGPIOx->IDR >> pinNumber) & 0x00000001);

	return value;
}

/****************************************************************
 * @fn			- GPIO_ReadFromInputPort
 *
 * @brief		- This function enables or disables peripheral clock for the given GPIO Port
 *
 * pGPIOx		- base addreess of the GPIO peripheral
 * pinNumber	-
 *
 * @return		- value
 *
 * @note		- none
 */
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx)
{
	uint16_t value;
	value = (uint16_t)pGPIOx->IDR;

	return value;
}

/****************************************************************
 * @fn			- GPIO_WriteToOutputPin
 *
 * @brief		-
 *
 * pGPIOx		- base addreess of the GPIO peripheral
 * pinNumber	-
 *
 * @return		- value
 *
 * @note		- none
 */
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t pinNumber, uint8_t value)
{
	if (value == GPIO_PIN_SET)
	{
		// write 1 to the ouput dataregister at the bit filed corresponding to the pin number
		pGPIOx->ODR |= (1 << pinNumber);
	}
	else
	{
		// write 0
		pGPIOx->ODR &= ~(1 << pinNumber);
	}
}

/****************************************************************
 * @fn			- GPIO_WriteToOutputPort
 *
 * @brief		- This function enables or disables peripheral clock for the given GPIO Port
 *
 * pGPIOx		- base addreess of the GPIO peripheral
 * pinNumber	-
 *
 * @return		- value
 *
 * @note		- none
 */
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t value)
{
	pGPIOx->ODR = value;
}

/****************************************************************
 * @fn			- GPIO_ToggleOutputPin
 *
 * @brief		- This function enables or disables peripheral clock for the given GPIO Port
 *
 * pGPIOx		- base addreess of the GPIO peripheral
 * pinNumber	-
 *
 * @return		- value
 *
 * @note		- none
 */
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t pinNumber)
{
	pGPIOx->ODR ^= (1 << pinNumber);
}

/****************************************************************
 * @fn			- GPIO_IRQInterruptConfig
 *
 * @brief		- This function enables or disables peripheral clock for the given GPIO Port
 *
 * pGPIOx		- base addreess of the GPIO peripheral
 * pinNumber	-
 *
 * @return		- value
 *
 * @note		- none
 */
void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		if (IRQNumber <= 31)
		{
			// program ISER0 register
			*NVIC_ISER0 |= (1 << IRQNumber);
		}
		else if (IRQNumber > 31 && IRQNumber < 64)		// 32 to 63
		{
			// program ISER1 register
			*NVIC_ISER1 |= (1 << (IRQNumber % 32));
		}
		else if (IRQNumber >= 64 && IRQNumber < 96)		// 64 to 95
		{
			// program ISER2 register
			*NVIC_ISER2 |= (1 << (IRQNumber % 32));
		}
	}
	else
	{
		if (IRQNumber <= 31)
		{
			*NVIC_ICER0 |= (1 << IRQNumber);
		}
		else if (IRQNumber > 31 && IRQNumber < 64)
		{
			*NVIC_ICER1 |= (1 << (IRQNumber % 32));
		}
		else if (IRQNumber >= 64 && IRQNumber < 96)
		{
			*NVIC_ICER2 |= (1 << (IRQNumber % 32));
		}
	}
}

void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority)
{
	//1. first lets find out the IPR
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	//2. Calculate the number of bit-shifted.
	//note: Because the ARM Cortex-M4 architecture only supports the upper 4 bits in a
	//byte for storing priority, we need to shift an additional (8 – 4) bits.
	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);

	*(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);
}

void GPIO_IRQHandling(uint8_t pinNumber)
{
	//clear the exti pr register corresponding to the pin number
	if (EXTI->PR & (1 << pinNumber))
	{
		EXTI->PR |= (1 << pinNumber);
	}
}
