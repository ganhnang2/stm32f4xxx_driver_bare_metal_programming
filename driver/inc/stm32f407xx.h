/*
 * stm32f407xx.h
 *
 *  Created on: Apr 26, 2026
 *      Author: ASUS
 */

#ifndef INC_STM32F407XX_H_
#define INC_STM32F407XX_H_

#include <stdint.h>

#define __vo		volatile
#define __weak   	__attribute__((weak))

/*************************************START: Provessor Specific Details************************************/
/*
 * ARM Cortex Mx Processor NVIC ISERx register Addresses
 */
#define	NVIC_ISER0				((__vo uint32_t*) 0xE000E100UL)
#define	NVIC_ISER1				((__vo uint32_t*) 0xE000E104UL)
#define	NVIC_ISER2				((__vo uint32_t*) 0xE000E108UL)
#define	NVIC_ISER3				((__vo uint32_t*) 0xE000E10CUL)
#define NVIC_ISER4				((__vo uint32_t*) 0xE000E110UL)
#define NVIC_ISER5				((__vo uint32_t*) 0xE000E114UL)
#define NVIC_ISER6				((__vo uint32_t*) 0xE000E118UL)
#define NVIC_ISER7				((__vo uint32_t*) 0xE000E11CUL)


/*
 * ARM Cortex Mx Processor NVIC ICERx register Addresses
 */
#define NVIC_ICER0				((__vo uint32_t*) 0xE000E180UL)
#define NVIC_ICER1				((__vo uint32_t*) 0xE000E184UL)
#define NVIC_ICER2				((__vo uint32_t*) 0xE000E188UL)
#define NVIC_ICER3				((__vo uint32_t*) 0xE000E18CUL)
#define NVIC_ICER4				((__vo uint32_t*) 0xE000E190UL)
#define NVIC_ICER5				((__vo uint32_t*) 0xE000E194UL)
#define NVIC_ICER6				((__vo uint32_t*) 0xE000E198UL)
#define NVIC_ICER7				((__vo uint32_t*) 0xE000E19CUL)

/*
 * ARM Cortex Mx Processor Priority Register Address Calculation
 */
#define NVIC_PR_BASE_ADDR 		((__vo uint32_t*) 0xE000E400)

#define NO_PR_BITS_IMPLEMENTED	4

/*	Base addresses of FLASH and SRAM memories	*/
#define FLASH_BASEADDR			0x08000000U
#define SRAM1_BASEADDR			0x20000000U
#define SRAM2_BASEADDR			0x2001C000U
#define ROM_BASEADDR			0x1FFF0000
#define SRAM					SRAM1_BASEADDR

/*	AHBx and APBx Bus Peripheral base addresses */

#define PERIPH_BASE				0x40000000U
#define APB1PERIPH_BASE			PERIPH_BASE
#define APB2PERIPH_BASE			0x40010000U
#define AHB1PERIPH_BASE			0x40020000U
#define AHB2PERIPH_BASE			0x50000000U

/*	Base addresses of peripherals which are hanging on AHB1 bus	*/
#define GPIOA_BASEADDR			(AHB1PERIPH_BASE + 0x0000)
#define GPIOB_BASEADDR			(AHB1PERIPH_BASE + 0x0400)
#define GPIOC_BASEADDR			(AHB1PERIPH_BASE + 0x0800)
#define GPIOD_BASEADDR			(AHB1PERIPH_BASE + 0x0C00)
#define GPIOE_BASEADDR			(AHB1PERIPH_BASE + 0x1000)
#define GPIOF_BASEADDR			(AHB1PERIPH_BASE + 0x1400)
#define GPIOG_BASEADDR			(AHB1PERIPH_BASE + 0x1800)
#define GPIOH_BASEADDR			(AHB1PERIPH_BASE + 0x1C00)
#define GPIOI_BASEADDR			(AHB1PERIPH_BASE + 0x2000)
#define GPIOJ_BASEADDR			(AHB1PERIPH_BASE + 0x2400)
#define GPIOK_BASEADDR			(AHB1PERIPH_BASE + 0x2800)
#define RCC_BASEADDR			(AHB1PERIPH_BASE + 0x3800)

/*	Base addresses of peripherals which are hanging on APB1 bus	*/
#define I2C1_BASEADDR			(APB1PERIPH_BASE + 0x5400)
#define I2C2_BASEADDR			(APB1PERIPH_BASE + 0x5800)
#define I2C3_BASEADDR			(APB1PERIPH_BASE + 0x5C00)

#define SPI1_BASEADDR 			(APB2PERIPH_BASE + 0x3000)
#define SPI2_BASEADDR			(APB1PERIPH_BASE + 0x3800)
#define SPI3_BASEADDR			(APB1PERIPH_BASE + 0x3C00)

#define USART2_BASEADDR			(APB1PERIPH_BASE + 0x4400)
#define USART3_BASEADDR			(APB1PERIPH_BASE + 0x4800)
#define UART4_BASEADDR			(APB1PERIPH_BASE + 0x4C00)
#define UART5_BASEADDR			(APB1PERIPH_BASE + 0x5000)

/*	Base addresses of peripherals which are hanging on APB2 bus	*/
#define EXTI_BASEADDR			(APB2PERIPH_BASE + 0x3C00)
#define	SPI_BASEADDR			(APB2PERIPH_BASE + 0x3000)
#define	SYSCFG_BASEADDR			(APB2PERIPH_BASE + 0x3800)
#define	USART1_BASEADDR			(APB2PERIPH_BASE + 0x1000)
#define	USART6_BASEADDR			(APB2PERIPH_BASE + 0x1400)

/*************************************************PERIPHERAL REGISTER DEFINITION STRUCUTRES*********************************/
typedef struct
{
	__vo uint32_t MODER;			// Address offset: 0x00
	__vo uint32_t OTYPER;			// Address offset: 0x04
	__vo uint32_t OSPEEDR;			// ...
	__vo uint32_t PUPDR;
	__vo uint32_t IDR;
	__vo uint32_t ODR;
	__vo uint32_t BSRR;
	__vo uint32_t LCKR;
	__vo uint32_t AFR[2];			// AFR[0]: AFRL, AFR[1]: AFRH
}GPIO_RegDef_t;

typedef struct
{
	__vo uint32_t CR1;				// Address offset: 0x00
	__vo uint32_t CR2;				// Address offset: 0x04
	__vo uint32_t SR;				// Address offset: 0x08
	__vo uint32_t DR;				// Address offset: 0x0C
	__vo uint32_t CRCPR;			// Address offset: 0x10
	__vo uint32_t RXCRCR;			// Address offset: 0x14
	__vo uint32_t TXCRCR;			// Address offset: 0x18
	__vo uint32_t I2SCFGR;			// Address offset: 0x1C
	__vo uint32_t I2SPR;			// Address offset: 0x20
}SPI_RegDef_t;

typedef struct
{
	__vo uint32_t CR;					// Address offset: 0x00
	__vo uint32_t PLLCFGR;				// Address offset: 0x04
	__vo uint32_t CFGR;					// Address offset: 0x08
	__vo uint32_t CIR;					// Address offset: 0x0C
	__vo uint32_t AHB1RSTR;				// Address offset: 0x10
	__vo uint32_t AHB2RSTR;				// Address offset: 0x14
	__vo uint32_t AHB3RSTR;				// Address offset: 0x18
	__vo uint32_t REVERSED0;			// Reversed, 0x1C
	__vo uint32_t APB1RSTR;				// Address offset: 0x20
	__vo uint32_t APB2RSTR;				// Address offset: 0x04
	__vo uint32_t REVERSED1[2];			// Reversed, 0x28 - 0x2C
	__vo uint32_t AHB1ENR;				// Address offset: 0x30
	__vo uint32_t AHB2ENR;				// Address offset: 0x34
	__vo uint32_t AHB3ENR;				// Address offset: 0x38
	__vo uint32_t REVERSED2;			// Reversed, 0x3C
	__vo uint32_t APB1ENR;				// Address offset: 0x40
	__vo uint32_t APB2ENR;				// Address offset: 0x44
	__vo uint32_t REVERSED3[2];			// Reversed, 0x48 - 0x4C
	__vo uint32_t AHB1LPENR;			// Address offset: 0x50
	__vo uint32_t AHB2LPENR;			// Address offset: 0x54
	__vo uint32_t AHB3LPENR;			// Address offset: 0x58
	__vo uint32_t REVERSED4;			// Reversed, 0x5C
	__vo uint32_t APB1LPENR;			// Address offset: 0x60
	__vo uint32_t APB2LPENR;			// Address offset: 0x64
	__vo uint32_t REVERSED5[2];			// Reversed, 0x68 - 0x6C
	__vo uint32_t BDCR;					// Address offset: 0x70
	__vo uint32_t CSR;					// Address offset: 0x74
	__vo uint32_t REVERSED6[2];			// Reversed, 0x78 - 0x7C
	__vo uint32_t SSCGR;				// Address offset: 0x80
	__vo uint32_t PLLI2SCFGR;			// Address offset: 0x84
}RCC_RegDef_t;

typedef struct 
{
	__vo uint32_t CR1;					// Address offset: 0x00
	__vo uint32_t CR2;					// Address offset: 0x04
	__vo uint32_t OAR1;					// Address offset: 0x08					
	__vo uint32_t OAR2;					// Address offset: 0x0C			
	__vo uint32_t DR;					// Address offset: 0x10
	__vo uint32_t SR1;					// Address offset: 0x14
	__vo uint32_t SR2;					// Address offset: 0x18
	__vo uint32_t CCR;					// Address offset: 0x1C
	__vo uint32_t TRISE;				// Address offset: 0x20
	__vo uint32_t FLTR;					// Address offset: 0x24
} I2C_RegDef_t;

typedef struct
{
	__vo uint32_t IMR;					// Address offset: 0x00
	__vo uint32_t EMR;					// Address offset: 0x04
	__vo uint32_t RTSR;					// Address offset: 0x08
	__vo uint32_t FTSR;					// Address offset: 0x0C
	__vo uint32_t SWIER;				// Address offset: 0x10
	__vo uint32_t PR;					// Address offset: 0x14
}EXTI_RegDef_t;

typedef struct
{
	__vo uint32_t MEMRMP;				// Address offset: 0x00
	__vo uint32_t PMC;					// Address offset: 0x04
	__vo uint32_t EXTICR[4];			// Address offset: 0x08 - 0x14
	uint32_t REVERSED1[2];				// Address offset: 0x18 - 0x1C
	__vo uint32_t CMPCR;				// Address offset: 0x20
	uint32_t REVERSED2[2];				// Address offset: 0x24 - 0x28
	__vo uint32_t CFGR;					// Address offset: 0x2C
}SYSCFG_RegDef_t;

/*	Peripheral definition (Peripheral base addresses typecasted to xxx_RegDef_t	*/
#define GPIOA					((GPIO_RegDef_t*)GPIOA_BASEADDR)
#define GPIOB					((GPIO_RegDef_t*)GPIOB_BASEADDR)
#define GPIOC					((GPIO_RegDef_t*)GPIOC_BASEADDR)
#define GPIOD					((GPIO_RegDef_t*)GPIOD_BASEADDR)
#define GPIOE					((GPIO_RegDef_t*)GPIOE_BASEADDR)
#define GPIOF					((GPIO_RegDef_t*)GPIOF_BASEADDR)
#define GPIOG					((GPIO_RegDef_t*)GPIOG_BASEADDR)
#define GPIOH					((GPIO_RegDef_t*)GPIOH_BASEADDR)
#define GPIOI					((GPIO_RegDef_t*)GPIOI_BASEADDR)
#define GPIOJ					((GPIO_RegDef_t*)GPIOJ_BASEADDR)
#define GPIOK					((GPIO_RegDef_t*)GPIOK_BASEADDR)

#define RCC						((RCC_RegDef_t*)RCC_BASEADDR)
#define EXTI 					((EXTI_RegDef_t*)EXTI_BASEADDR)
#define SYSCFG					((SYSCFG_RegDef_t*)SYSCFG_BASEADDR)

#define SPI1					((SPI_RegDef_t*)SPI1_BASEADDR)
#define SPI2					((SPI_RegDef_t*)SPI2_BASEADDR)
#define SPI3					((SPI_RegDef_t*)SPI3_BASEADDR)

#define I2C1					((I2C_RegDef_t*)I2C1_BASEADDR)						
#define I2C2					((I2C_RegDef_t*)I2C2_BASEADDR)
#define	I2C3					((I2C_RegDef_t*)I2C3_BASEADDR)

/*	Clock Enable Marcos for GPIOx peripherals	*/
#define GPIOA_PCLK_EN()		(RCC->AHB1ENR |= (1 << 0))
#define GPIOB_PCLK_EN()		(RCC->AHB1ENR |= (1 << 1))
#define GPIOC_PCLK_EN()		(RCC->AHB1ENR |= (1 << 2))
#define GPIOD_PCLK_EN()		(RCC->AHB1ENR |= (1 << 3))
#define GPIOE_PCLK_EN()		(RCC->AHB1ENR |= (1 << 4))
#define GPIOF_PCLK_EN()		(RCC->AHB1ENR |= (1 << 5))
#define GPIOG_PCLK_EN()		(RCC->AHB1ENR |= (1 << 6))
#define GPIOH_PCLK_EN()		(RCC->AHB1ENR |= (1 << 7))
#define GPIOI_PCLK_EN()		(RCC->AHB1ENR |= (1 << 8))

/*	Clock Enable Marcos for I2Cx peripherals	*/
#define I2C1_PCLK_EN()		(RCC->APB1ENR |= (1 << 21))
#define I2C2_PCLK_EN()		(RCC->APB1ENR |= (1 << 22))
#define I2C3_PCLK_EN()		(RCC->APB1ENR |= (1 << 23))

/*	Clock Enable Marcos for SPIx peripherals	*/
#define SPI1_PCLK_EN()		(RCC->APB2ENR |= (1 << 12))
#define SPI2_PCLK_EN()		(RCC->APB1ENR |= (1 << 14))
#define SPI3_PCLK_EN()		(RCC->APB1ENR |= (1 << 15))

/*	Clock Enable Marcos for USARTx peripherals	*/
#define USART1_PCLK_EN()		(RCC->APB2ENR |= (1 << 4))
#define USART2_PCLK_EN()		(RCC->APB1ENR |= (1 << 17))
#define USART3_PCLK_EN()		(RCC->APB1ENR |= (1 << 18))
#define UART4_PCLK_EN()			(RCC->APB1ENR |= (1 << 19))
#define UART5_PCLK_EN()			(RCC->APB1ENR |= (1 << 20))
#define USART6_PCLK_EN()		(RCC->APB2ENR |= (1 << 5))

/*	Clock Enable Marcos for SYSCF peripherals 	*/
#define SYSCFG_PCLK_EN()			(RCC->APB2ENR |= (1 << 14))

/*	Clock Enable Disable for GPIOx peripherals	*/
#define GPIOA_PCLK_DI()		(RCC->AHB1ENR &= ~(1 << 0))

/*	Configure HSE as system clock 	*/
#define HSE_CLOCK_ENABLE	do { (RCC->CR |= (1 << 16)); while(!(RCC->CR & (1 << 17))); }; while(0)

/*	Marcos to reset GPIOx peripheral 	*/
#define GPIOA_REG_RESET()		do { (RCC->AHB1RSTR |= (1 << 0));  (RCC->AHB1RSTR &= ~(1 << 0)); } while(0)
#define GPIOB_REG_RESET()		do { (RCC->AHB1RSTR |= (1 << 1));  (RCC->AHB1RSTR &= ~(1 << 1)); } while(0)
#define GPIOC_REG_RESET()		do { (RCC->AHB1RSTR |= (1 << 2));  (RCC->AHB1RSTR &= ~(1 << 2)); } while(0)
#define GPIOD_REG_RESET()		do { (RCC->AHB1RSTR |= (1 << 3));  (RCC->AHB1RSTR &= ~(1 << 3)); } while(0)
#define GPIOE_REG_RESET()		do { (RCC->AHB1RSTR |= (1 << 4));  (RCC->AHB1RSTR &= ~(1 << 4)); } while(0)
#define GPIOF_REG_RESET()		do { (RCC->AHB1RSTR |= (1 << 5));  (RCC->AHB1RSTR &= ~(1 << 5)); } while(0)
#define GPIOG_REG_RESET()		do { (RCC->AHB1RSTR |= (1 << 6));  (RCC->AHB1RSTR &= ~(1 << 6)); } while(0)
#define GPIOH_REG_RESET()		do { (RCC->AHB1RSTR |= (1 << 7));  (RCC->AHB1RSTR &= ~(1 << 7)); } while(0)
#define GPIOI_REG_RESET()		do { (RCC->AHB1RSTR |= (1 << 8));  (RCC->AHB1RSTR &= ~(1 << 8)); } while(0)

#define GPIO_BASEADDR_TO_CODE(x)	((x == GPIOA) ? 0:\
									 (x == GPIOB) ? 1:\
									 (x == GPIOC) ? 2:\
									 (x == GPIOD) ? 3:\
									 (x == GPIOE) ? 4:\
									 (x == GPIOF) ? 5:\
									 (x == GPIOG) ? 6:\
									 (x == GPIOH) ? 7:\
									 (x == GPIOI) ? 8:\
									 (x == GPIOJ) ? 9:\
									 (x == GPIOH) ? 10 : 0)

/*	IRQ(Interrupt Request) Numbers of STM32F407x MCU	*/
#define IRQ_NO_EXTI0  			6
#define IRQ_NO_EXTI1  			7
#define IRQ_NO_EXTI2  			8
#define IRQ_NO_EXTI3  			9
#define IRQ_NO_EXTI4  			10
#define IRQ_NO_EXTI9_5  		23
#define IRQ_NO_EXTI15_10  		40

#define SPI1_IRQ_NUMBER 		35
#define SPI2_IRQ_NUMBER			36
#define SPI3_IRQ_NUMBER			51


// Priority
#define NVIC_IRQ_PRIO0			0
#define NVIC_IRQ_PRIO1			1
#define NVIC_IRQ_PRIO2			2
#define NVIC_IRQ_PRIO3			3
#define NVIC_IRQ_PRIO4			4
#define NVIC_IRQ_PRIO5			5
#define NVIC_IRQ_PRIO6			6
#define NVIC_IRQ_PRIO7			7
#define NVIC_IRQ_PRIO8			8
#define NVIC_IRQ_PRIO9			9
#define NVIC_IRQ_PRIO10			10
#define NVIC_IRQ_PRIO11			11
#define NVIC_IRQ_PRIO12			12
#define NVIC_IRQ_PRIO13			13
#define NVIC_IRQ_PRIO14			14
#define NVIC_IRQ_PRIO15			15

// Some generic marcos
#define ENABLE			1
#define DISABLE 		0
#define SET				ENABLE
#define	RESET			DISABLE
#define GPIO_PIN_SET	SET
#define GPIO_PIN_RESET	RESET
#define FLAG_SET		SET
#define FLAG_RESET		RESET

#include "stm32f407xxx_gpio_driver.h"
#include "stm32f407xxx_spi_driver.h"
#include "stm32f407xxx_i2c_driver.h"

#endif /* INC_STM32F407XX_H_ */
