/*
 * stm32f4xx_spi_driver.h
 *
 *  Created on: May 22, 2026
 *      Author: ASUS
 */

#ifndef INC_STM32F407XXX_SPI_DRIVER_H_
#define INC_STM32F407XXX_SPI_DRIVER_H_

#include "stm32f407xx.h"

/*
 * Configuration structure for SPIx peripheral
 * */
typedef struct
{
	uint8_t SPI_DeviceMode;
	uint8_t SPI_BusConfig;
	uint8_t SPI_SclkSpeed;
	uint8_t SPI_DFF;
	uint8_t SPI_CPOL;
	uint8_t SPI_CPHA;
	uint8_t SPI_SSM;
}SPI_Config_t;

/*
 * Handle structure for SPIx peripheral
 */
typedef struct
{
	SPI_RegDef_t* pSPIx;		// This hold the base address of SPIx (x: 0, 1, 2)
	SPI_Config_t SPI_Config;
	uint8_t *pTxBuffer;			// TxBuffer address
	uint8_t *pRxBuffer;			// RxBuffer address
	uint32_t TxLen;				// Store Tx len
	uint32_t RxLen;				// Store Rx len
	uint8_t TxState;			// Store Rx state
	uint8_t RxState;			// Store Tx state
}SPI_Handle_t;

/*
 * SPI_DeviceMode
 */
#define SPI_MODE_MASTER						1
#define SPI_MODE_SLAVE						0

/*
 * SPI_BusConfig
 */
#define SPI_BUS_CONFIG_FD					0
#define SPI_BUS_CONFIG_HD					1
#define SPI_BUS_CONFIG_SIMPLEX_ONLY			2

/*
 * SPI_SclkSpeed
 */
#define SPI_SCLK_SPEED_DIV2						0
#define SPI_SCLK_SPEED_DIV4						1
#define SPI_SCLK_SPEED_DIV8						2
#define SPI_SCLK_SPEED_DIV16					3
#define SPI_SCLK_SPEED_DIV32					4
#define SPI_SCLK_SPEED_DIV64					5
#define SPI_SCLK_SPEED_DIV128					6
#define SPI_SCLK_SPEED_DIV256					7

/*
 * SPI_DFF
 */
#define SPI_DFF_8BITS		0
#define SPI_DFF_16BITS		1

/*
 * SPI_CPOL
 */
#define SPI_CPOL_HIGH		1
#define SPI_CPOL_LOW		0

/*
 * SPI_CPHA
 */
#define SPI_CPHA_HIGH		1
#define SPI_CPHA_LOW		0

/*
 * SPI_SSM
 */
#define SPI_SSM_EN			ENABLE
#define SPI_SSM_DI			DISABLE

#define SPI_TXE_FLAG		(1 << 1)
#define SPI_RXNE_FLAG		(1 << 0)
#define SPI_BUSY_FLAG		(1 << 7)

/*
 * CR1 Register
 */
#define SPI_CR1_BIDIMODE		15
#define SPI_CR1_BIDIOE			14
#define SPI_CR1_CRCEN			13
#define SPI_CR1_CRCNEXT			12
#define SPI_CR1_DFF				11
#define SPI_CR1_RXONLY			10
#define SPI_CR1_SSM				9
#define SPI_CR1_SSI				8
#define SPI_CR1_LSBFIRST		7
#define SPI_CR1_SPE				6
#define SPI_CR1_BR2_0			3
#define SPI_CR1_MSTR			2
#define SPI_CR1_CPOL			1
#define SPI_CR1_CPHA			0

/*
 * CR2 Register
 */
#define SPI_CR2_TXEIE			7
#define SPI_CR2_RXNEIE			6
#define SPI_CR2_ERRIE			5
#define SPI_CR2_FRF				4
#define SPI_CR2_SSOE			2
#define SPI_CR2_TXDMAEN			1
#define SPI_CR2_RXDMAEN			0

/*
 * SR Register
 */
#define SPI_SR_FRE				8
#define SPI_SR_BSY				7
#define SPI_SR_OVR				6
#define SPI_SR_MODF				5
#define SPI_SR_CRCERR			4
#define SPI_SR_UDR				3
#define SPI_SR_CHSIDE			2
#define SPI_SR_TXE				1
#define SPI_SR_RXNE				0

/*	SPI Application States	*/
#define SPI_READY				0
#define SPI_BUSY_IN_RX			1
#define SPI_BUSY_IN_TX			2

/*	Possible SPI Application Events	*/
#define SPI_EVENT_TX_CMPLT		1
#define SPI_EVENT_RX_CMPLT		2
#define SPI_EVENT_OVR_ERR		3
#define SPI_EVENT_CRC_ERR		4

/***************************************************************************************
 *						APIs support by this driver
 *		For more information about the APIs check the function definitions
 ***************************************************************************************/

/*	Peripheral clock	*/
void SPI_PeripheralClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi);

/*	Init and De-init	*/
void SPI_Init(SPI_Handle_t *pSPIHandle);
void SPI_DeInit(SPI_RegDef_t *pSPIx);

/*
 * Data send and receive
 */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t* pTxBuffer, uint32_t len);
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t* pRxBuffer, uint8_t len);
void SPI_TransmitReceive(SPI_RegDef_t *pSPIx, uint8_t* pTxBuffer, uint8_t* pRxBuffer, uint16_t len);

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t* pTxBuffer, uint32_t len);
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t* pRxBuffer, uint8_t len);
void SPI_TransmitReceive_IT(SPI_RegDef_t *pSPIx, uint8_t* pTxBuffer, uint8_t* pRxBuffer, uint16_t len);

/*
 * IRQ Configurattion and ISR handling
 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);
void SPI_IRQHandling(SPI_Handle_t* pHandle);

/*
 * Other peripheral Control APIs
 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi);
void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnorDi);
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnorDi);
uint8_t SPI_GetFlagStatus(SPI_RegDef_t* pSPIx, uint32_t FlagName);
void SPI_ClearOVRFlag(SPI_RegDef_t* pSPIx);

/*
 * Application callback
 */
void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,uint8_t AppEv);
#endif /* INC_STM32F407XXX_SPI_DRIVER_H_ */
