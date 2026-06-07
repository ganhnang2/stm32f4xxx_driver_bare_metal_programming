/*
 * stm32f4xx_spi_driver.c
 *
 *  Created on: May 22, 2026
 *      Author: ASUS
 */

#include "stm32f407xxx_spi_driver.h"
#include <stddef.h>

// helper function. Keyword static indicate that the user must not allow to
// call these functions.
static void spi_txe_interrupt_handle(SPI_Handle_t* pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t* pSPIHandle);
static void spi_ovr_interrupt_handle(SPI_Handle_t* pSPIHandle);

/****************************************************************
 * @fn			- SPI_PeripheralClockControl
 *
 * @brief		- This function enables or disables peripheral clock for the given SPI
 *
 *
 * @return		- none
 *
 * @note		- none
 */
void SPI_PeripheralClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		if (pSPIx == SPI1)
		{
			SPI1_PCLK_EN();
		}
		else if (pSPIx == SPI2)
		{
			SPI2_PCLK_EN();
		}
		else if (pSPIx == SPI3)
		{
			SPI3_PCLK_EN();
		}
	}
	else
	{
	}
}

/****************************************************************
 * @fn			- SPI_Init
 *
 * @brief		-
 *
 *
 * @return		- none
 *
 * @note		- none
 */
void SPI_Init(SPI_Handle_t *pSPIHandle)
{
	SPI_PeripheralClockControl(pSPIHandle->pSPIx, ENABLE);

	// first lets configure the SPI_CR1 register
	uint32_t tempReg = 0;

	//1. configure the device mode
	tempReg |= pSPIHandle->SPI_Config.SPI_DeviceMode << 2;

	//2. configure the bus config
	if (pSPIHandle->SPI_Config.SPI_BusConfig == SPI_BUS_CONFIG_FD)
	{
		// bidi mode should be cleared
		tempReg &= ~(1 << SPI_CR1_BIDIMODE);
	}
	else if (pSPIHandle->SPI_Config.SPI_BusConfig == SPI_BUS_CONFIG_HD)
	{
		// bidi mode should be set
		tempReg |= (1 << SPI_CR1_BIDIMODE);
	}
	else if (pSPIHandle->SPI_Config.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_ONLY)
	{
		// bidi mode should be cleared
		// RXONLY bit must be set
		tempReg &= ~(1 << SPI_CR1_BIDIMODE);
		tempReg |= (1 << SPI_CR1_RXONLY);
	}

	//3. configure the spi serial clock speed (baud rate)
	tempReg &= ~(1 << SPI_CR1_BR2_0);
	tempReg |= (pSPIHandle->SPI_Config.SPI_SclkSpeed << SPI_CR1_BR2_0);
	//4. configure the DFF
	tempReg &= ~(1 << SPI_CR1_DFF);
	tempReg |= (pSPIHandle->SPI_Config.SPI_DFF<< SPI_CR1_DFF);
	//5. configure the CPOL
	tempReg &= ~(1 << SPI_CR1_CPOL);
	tempReg |= (pSPIHandle->SPI_Config.SPI_CPOL << SPI_CR1_CPOL);
	//6. configure the CPHA
	tempReg &= ~(1 << SPI_CR1_CPHA);
	tempReg |= (pSPIHandle->SPI_Config.SPI_CPHA << SPI_CR1_CPHA);
	// 7. Configure the SSM (Software Slave Management)
	tempReg &= ~(1 << SPI_CR1_SSM);
	tempReg |= (pSPIHandle->SPI_Config.SPI_SSM  << SPI_CR1_SSM);

	pSPIHandle->pSPIx->CR1 = tempReg;
}

uint8_t SPI_GetFlagStatus(SPI_RegDef_t* pSPIx, uint32_t FlagName)
{
	if (pSPIx->SR & FlagName)
	{
		return FLAG_SET;
	}
	return FLAG_RESET;
}

/****************************************************************
 * @fn			- SPI_SendData
 *
 * @brief		-
 *
 *
 * @return		- none
 *
 * @note		- none
 */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t* pTxBuffer, uint32_t len)
{
	while(len > 0)
	{
		//1. wait until TXE is set
		while (SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);
		//2. check DFF bit in CR1
		if ((pSPIx->CR1 & (1 << SPI_CR1_DFF)) && len >= 2)
		{
			//16-bit DFF
			//1. load the data in to the DR
			pSPIx->DR = *((uint16_t*)pTxBuffer);
			len -= 2;
			pTxBuffer += 2;
		}
		else
		{
			//8-bit DFF
			pSPIx->DR = *pTxBuffer;
			len--;
			pTxBuffer++;
		}
	}
}

/****************************************************************
 * @fn			- SPI_ReceiveData
 *
 * @brief		-
 *
 *
 * @return		- none
 *
 * @note		- none
 */
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t* pRxBuffer, uint8_t len)
{
	while(len > 0)
	{
		// 1. Wait until bit RXNE = 1 (i.e: Shift Register has thrown the data into RX buffer)
		while(SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);

		// 2. Check the DFF bit in CR1
		if ((pSPIx->CR1 & (1 << SPI_CR1_DFF)) && len >= 2)
		{
			//16-bit DFF
			//1. load the data from SPI_DR
			*((uint16_t*)pRxBuffer) = pSPIx->DR;
			len -= 2;
			pRxBuffer += 2;
		}
		else
		{
			//8-bit DFF
			*pRxBuffer = pSPIx->DR;
			len--;
			pRxBuffer++;
		}
	}
}

/****************************************************************
 * @fn			- SPI_TransmitReceiveData
 *
 * @brief		-
 *
 *
 * @return		- none
 *
 * @note		- none
 */
void SPI_TransmitReceive(SPI_RegDef_t *pSPIx, uint8_t* pTxBuffer, uint8_t* pRxBuffer, uint16_t len)
{
	while(len > 0)
	{	
		// Check the DFF bit in CR1
		if ((pSPIx->CR1 & (1 << SPI_CR1_DFF)) && len >= 2)
		{
			// 16-bit DFF
			//1. Wait until TXE is set
			while(SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);
			pSPIx->DR = *((uint16_t*)pTxBuffer);
			//2. Wait until bit RXNE = 1 (i.e: Shift Register has thrown the data into RX buffer)
			while(SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);
			*((uint16_t*)pRxBuffer) = pSPIx->DR;
			len -= 2;
			pTxBuffer += 2;
			pRxBuffer += 2;
		}
		else
		{
			// 8-bit DFF
			//1. Wait until TXE is set
			while(SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);
			pSPIx->DR = *pTxBuffer;
			//2. Wait until bit RXNE = 1 (i.e: Shift Register has thrown the data into RX buffer)
			while(SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);
			*pRxBuffer = pSPIx->DR;
			len --;
			pTxBuffer += 1;
			pRxBuffer += 1;
		}

	}
}

/*
 *
 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		pSPIx->CR1 |=  (1 << SPI_CR1_SPE); // enable SPE bit
	}
	else
	{
		pSPIx->CR1 &= ~(1 << SPI_CR1_SPE); // disable SPE bit
	}
}

void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		pSPIx->CR1 |=  (1 << SPI_CR1_SSI); // enable SSI bit
	}
	else
	{
		pSPIx->CR1 &= ~(1 << SPI_CR1_SSI); // disable SSI bit
	}
}

void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		pSPIx->CR2 |=  (1 << SPI_CR2_SSOE); // enable SSOE bit
	}
	else
	{
		pSPIx->CR2 &= ~(1 << SPI_CR2_SSOE); // disable SSOE bit
	}
}

void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
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

void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority)
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

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t* pTxBuffer, uint32_t len)
{
	uint8_t state = pSPIHandle->TxState;
	if (state != SPI_BUSY_IN_TX)
	{
		// 1. Save the Tx buffer address and len information
		pSPIHandle->pTxBuffer = pTxBuffer;
		pSPIHandle->TxLen = len;

		// 2. Mark the SPI state as busy in transmission so that
		//	  no other code can take over same SPI peripheral until transmission is over
		pSPIHandle->TxState = SPI_BUSY_IN_TX;

		// 3. Enable the TXEIE control bit to get interrupt whenever TXE flag is set in SR
		pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_TXEIE);
	}

	return state;	
}

uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t* pRxBuffer, uint8_t len)
{
	uint8_t state = pSPIHandle->RxState;
	if (state != SPI_BUSY_IN_RX)
	{
		// 1. Save the Rx buffer address and len information
		pSPIHandle->pRxBuffer = pRxBuffer;
		pSPIHandle->RxLen = len;

		// 2. Mark the SPI state as busy in transmission so that
		//	  no other code can take over same SPI peripheral until transmission is over
		pSPIHandle->RxState = SPI_BUSY_IN_RX;

		// 3. Enable the RXNEIE control bit to get interrupt whenever RXNE flag is set in SR
		pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_RXNEIE);
	}
	return state;
}

void SPI_IRQHandling(SPI_Handle_t* pSPIHandle)
{
	uint8_t temp1, temp2;

	// first lets check for TXE
	temp1 = pSPIHandle->pSPIx->SR & (1 << SPI_SR_TXE);
	temp2 = pSPIHandle->pSPIx->CR2 & (1 << SPI_CR2_TXEIE);

	if (temp1 && temp2)
	{
		// handle TXE
		spi_txe_interrupt_handle(pSPIHandle);
	}

	// lets check for RXNE
	temp1 = pSPIHandle->pSPIx->SR & (1 << SPI_SR_RXNE);
	temp2 = pSPIHandle->pSPIx->CR2 & (1 << SPI_CR2_RXNEIE);

	if (temp1 && temp2)
	{
		// handle RXNE
		spi_rxne_interrupt_handle(pSPIHandle);
	}

	// check for ovr flag
	temp1 = pSPIHandle->pSPIx->SR & (1 << SPI_SR_OVR);
	temp2 = pSPIHandle->pSPIx->CR2 & (1 << SPI_CR2_ERRIE);

	if (temp1 && temp2)
	{
		// handle TXE
		spi_ovr_interrupt_handle(pSPIHandle);
	}
}

static void spi_txe_interrupt_handle(SPI_Handle_t* pSPIHandle)
{
	// check DFF bit in CR1
	if ((pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF)) && pSPIHandle->TxLen >= 2)
	{
		//16-bit DFF
		//1. load the data in to the DR
		pSPIHandle->pSPIx->DR = *((uint16_t*)pSPIHandle->pTxBuffer);
		pSPIHandle->TxLen -= 2;
		pSPIHandle->pTxBuffer += 2;
	}
	else
	{
		//8-bit DFF
		pSPIHandle->pSPIx->DR = *pSPIHandle->pTxBuffer;
		pSPIHandle->TxLen -= 1;
		pSPIHandle->pTxBuffer += 1;
	}

	if (pSPIHandle->TxLen == 0)
	{
		// Txlen is zero, so close the spi transmission and inform the applicaton
		// that TX is over.
		// this prevents interrupts from setting up of TXE flag
		pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_TXEIE);
		pSPIHandle->pTxBuffer = NULL;
		pSPIHandle->TxLen = 0;
		pSPIHandle->TxState = SPI_READY;
		SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);
	}
}

static void spi_rxne_interrupt_handle(SPI_Handle_t* pSPIHandle)
{
	// check DFF bit in CR1
	if ((pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF)) && pSPIHandle->RxLen >= 2)
	{
		//16-bit DFF
		//1. load the data in to the DR
		*((uint16_t*)pSPIHandle->pRxBuffer)= (uint16_t) pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen -= 2;
		pSPIHandle->pRxBuffer += 2;
	}
	else
	{
		//8-bit DFF
		*(pSPIHandle->pRxBuffer)= (uint8_t) pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen -= 1;
		pSPIHandle->pRxBuffer += 1;
	}

	if (pSPIHandle->RxLen == 0)
	{
		// Txlen is zero, so close the spi transmission and inform the applicaton
		// that TX is over.
		// this prevents interrupts from setting up of TXE flag
		pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_RXNEIE);
		pSPIHandle->pRxBuffer = NULL;
		pSPIHandle->RxLen = 0;
		pSPIHandle->RxState = SPI_READY;
		SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_RX_CMPLT);
	}
}

static void spi_ovr_interrupt_handle(SPI_Handle_t* pSPIHandle)
{
	uint8_t temp;
	// 1. clear the ovr flag
	if (pSPIHandle->TxState != SPI_BUSY_IN_TX)
	{
		temp = pSPIHandle->pSPIx->DR;
		temp = pSPIHandle->pSPIx->SR;
	}
	(void)temp;
	// 2. inform the application
	SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_OVR_ERR);
}

__weak void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,uint8_t AppEv)
{
	//This is a weak implementation . the user application may override this function.
}
