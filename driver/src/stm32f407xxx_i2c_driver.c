/*
*/
#include "stm32f407xxx_i2c_driver.h"
#include <stddef.h>

static void I2C_ClearAddrFlag(I2C_RegDef_t* pI2Cx)
{
	uint32_t dummyRead;
	dummyRead = pI2Cx->SR1;
	dummyRead = pI2Cx->SR2;
	(void)dummyRead;		
}

uint16_t AHB_Prescaler[8] = {2, 4, 8, 16, 64, 128, 256, 512};
uint16_t APB1_Prescaler[4] = {2, 4, 8, 16};

uint32_t RCC_GetPLLOutputClock()
{
	return 0;
}

uint32_t RCC_GetPCLK1Value()
{
	uint8_t clockSrc, temp;
	
	uint16_t ahbPrescal, apb1Prescal;
	
	uint32_t systemClk;
	// 1. Check the clock source
	clockSrc = (RCC->CFGR >> 2) & 0x3;
	
	if (clockSrc == 0)
	{
		// HSI: 16MHz -> Default
		systemClk = 16000000;
	}
	else if (clockSrc == 1)
	{
		// HSE: 8MHz
		systemClk = 8000000;
	}
	else if (clockSrc == 2)
	{
		systemClk = RCC_GetPLLOutputClock();
	}
	
	// AHB Prescaler
	temp = (RCC->CFGR >> 4) & 0xF;
	if (temp < 8)
	{
		ahbPrescal = 1;
	}
	else
	{
		ahbPrescal = AHB_Prescaler[temp - 8];
	}
	
	// APB1 Prescaler
	temp = (RCC->CFGR >> 10) & 0x7;
	if (temp < 4)
	{
		apb1Prescal = 1;
	}
	else
	{
		apb1Prescal = APB1_Prescaler[temp - 4];
	}
	
	return systemClk / ahbPrescal / apb1Prescal;
	
}

static void I2C_GenerateStartCondition(I2C_RegDef_t* pI2Cx)
{
	pI2Cx->CR1 |= (1 << 8);
}

static void I2C_GenerateStopCondition(I2C_RegDef_t* pI2Cx)
{
	pI2Cx->CR1 |= (1 << 9);
}

static void I2C_ExecuteAddressPhaseWrite(I2C_Handle_t* pI2CHandle, uint32_t slaveAddr)
{
	uint8_t byteSend;
	
	// 7-bit mode
	if (pI2CHandle->I2C_Config.I2C_AddrMode == I2C_ADDRESS_MODE_7_BITS)
	{
		byteSend = (uint8_t)(slaveAddr << 1);
		byteSend &= ~(1 << 0);							// Address format = slaveAddr + bit R/W (0 - Write)
		pI2CHandle->pI2Cx->DR = byteSend;				// Send the address format to slave
	}
	// 10-bit mode: Address format = Header + 8-bit address
	else
	{
		uint8_t byteSend;
		// 1. Send the header: 11110xx0 (xx: Bit 9, 8 of slaveAddress)
		byteSend = 0xF0;
		byteSend |= ((slaveAddr >> 8) & 0x03) << 1;
		pI2CHandle->pI2Cx->DR = byteSend;
		
		// 2. Wait until the flag ADD10 = 1
		while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADD10) == FLAG_RESET);
		
		// 3. Cleared by reading the SR1 register followed by a write in the DR register of the second address byte
		byteSend = (uint8_t)(slaveAddr & 0xFF);
		uint32_t dummyRead = pI2CHandle->pI2Cx->SR1;
		(void)dummyRead;
		pI2CHandle->pI2Cx->DR = byteSend;
	}
}

static void I2C_ExecuteAddressPhaseRead(I2C_Handle_t* pI2CHandle, uint32_t slaveAddr)
{
	uint8_t byteSend;
	
	// 7-bit mode
	if (pI2CHandle->I2C_Config.I2C_AddrMode == I2C_ADDRESS_MODE_7_BITS)
	{
		byteSend = (uint8_t)(slaveAddr << 1);
		byteSend |= (1 << 0);							// Address format = slaveAddr + bit R/W (1 - Read)
		pI2CHandle->pI2Cx->DR = byteSend;				// Send the address format to slave
	}
	// 10-bit mode: Address format = Header + 8-bit address
	else
	{
		uint8_t byteSend;
		// 1. Send the header: 11110xx0 (xx: Bit 9, 8 of slaveAddress)
		byteSend = 0xF0;
		byteSend |= ((slaveAddr >> 8) & 0x03) << 1;
		pI2CHandle->pI2Cx->DR = byteSend;
		
		// 2. Wait until the flag ADD10 = 1
		while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADD10) == FLAG_RESET);
		
		// 3. Clear flag ADD10 by reading the SR1 register followed by a write in the DR register of the second address byte
		byteSend = (uint8_t)(slaveAddr & 0xFF);
		uint32_t dummyRead = pI2CHandle->pI2Cx->SR1;
		(void)dummyRead;
		pI2CHandle->pI2Cx->DR = byteSend;
		
		// 4. Confirm that address phase is completed by checking ADDR flag in SR1
		while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR) == FLAG_RESET);
		
		// 5. Clear the ADDR flag according to its software sequence
		// 	  NOTE: Until ADDR is cleared SCL will be stretched (pulled to LOW)
		I2C_ClearAddrFlag(pI2CHandle->pI2Cx);
		
		// 6. Send Repeated Start 
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);
		
		// 7. Wait until bit SB = 1
		while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB) == FLAG_RESET);
		
		// 8. Send a new header to signal that STM32 is in master receiver mode (header = 11110xx1)
		byteSend = 0xF1;
		byteSend |= ((slaveAddr >> 8) & 0x03) << 1;
		pI2CHandle->pI2Cx->DR = byteSend;
	}
}

void I2C_ManageAck(I2C_RegDef_t* pI2Cx, uint8_t EnorDi)
{
	if(EnorDi == I2C_ACK_ENABLE)
	{
		// Enable the Ack
		pI2Cx->CR1 |= ( 1 << 10);
	}
	else
	{
		// Disable the ack
		pI2Cx->CR1 &= ~( 1 << 10);
	}
}

/****************************************************************
 * @fn			- I2C_PeripheralClockControl
 *
 * @brief		- This function enables or disables peripheral clock for the given I2C
 *
 *
 * @return		- none
 *
 * @note		- none
 */
void I2C_PeripheralClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		if (pI2Cx == I2C1)
		{
			I2C1_PCLK_EN();
		}
		else if (pI2Cx == I2C2)
		{
			I2C2_PCLK_EN();
		}
		else if (pI2Cx == I2C3)
		{
			I2C3_PCLK_EN();
		}
	}
	else
	{
	}
}

/****************************************************************
 * @fn			- I2C_PeripheralControl
 *
 * @brief		- This function enables or disables peripheral clock for the given I2C
 *
 *
 * @return		- none
 *
 * @note		- none
 */
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		pI2Cx->CR1 |=  (1 << 0); 	// enable PE bit
	}
	else
	{
		pI2Cx->CR1 &= ~(1 << 0); 	// disable PE bit
	}
}

/****************************************************************
 * @fn			- I2C_Init
 *
 * @brief		-
 *
 *
 * @return		- none
 *
 * @note		- none
 */
void I2C_Init(I2C_Handle_t *pI2CHandle)
{
	uint32_t tempReg = 0;
	// 1. Enable peripheral clock
	I2C_PeripheralClockControl(pI2CHandle->pI2Cx, ENABLE);
	
	// 2. ACK Control Bit
	tempReg |= pI2CHandle->I2C_Config.I2C_AckControl << 10;
	pI2CHandle->pI2Cx->CR1 = tempReg;
	
	// 3. Configure the FREQ field of CR2
	tempReg = 0;
	tempReg = RCC_GetPCLK1Value() / 1000000U;
	pI2CHandle->pI2Cx->CR2 = tempReg;
	
	// 4. Program the device own address
	tempReg = 0;
	tempReg |= (1 << 14);		// Always set bit 14 = 1 (from Reference Manual)
	if (pI2CHandle->I2C_Config.I2C_AddrMode == I2C_ADDRESS_MODE_7_BITS)
	{
		// 7-bit mode
		tempReg &= ~(1 << 15);
		tempReg |= ((uint8_t)pI2CHandle->I2C_Config.I2C_DeviceAddr << 1);
	}
	else if (pI2CHandle->I2C_Config.I2C_AddrMode == I2C_ADDRESS_MODE_10_BITS)
	{
		// 10-bit mode
		tempReg |= (1 << 15);
		tempReg |= pI2CHandle->I2C_Config.I2C_DeviceAddr & 0x3FF;
	}
	pI2CHandle->pI2Cx->OAR1 = tempReg;
	
	// 5. CCR Calculations
	uint16_t ccrValue;
	tempReg = 0;
	if (pI2CHandle->I2C_Config.I2C_SCLSpeed == I2C_SCL_SPEED_SM)	// standard mode
	{
		ccrValue = RCC_GetPCLK1Value() / (2 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
	}
	else		// fast mode
	{
		tempReg |= (1 << 15);
		tempReg |= (pI2CHandle->I2C_Config.I2C_FMDutyCycle << 14);
		if (pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2)
		{
			// Duty mode: tlow/thigh = 2
			ccrValue = RCC_GetPCLK1Value() / (3 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
		}
		else
		{
			// Duty mode: tlow/thigh = 16/9
			ccrValue = RCC_GetPCLK1Value() / (25 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
		}
	}
	tempReg |= (ccrValue) & 0xFFF;
	pI2CHandle->pI2Cx->CCR = tempReg;
	
	// 6. Configure the TRISE register (Calculate t_rise
	tempReg = 0;
	if (pI2CHandle->I2C_Config.I2C_SCLSpeed == I2C_SCL_SPEED_SM)
	{
		// Standard mode: TRISE = (PCLK1 in MHz) + 1
        tempReg = (RCC_GetPCLK1Value() / 1000000U) + 1;
	}
	else
	{
		// Fast mode: TRISE = (PCLK1 in MHz * 300 / 1000) + 1
		tempReg = ((RCC_GetPCLK1Value() / 1000000U) * 300 / 1000) + 1;
	}
	pI2CHandle->pI2Cx->TRISE = (tempReg & 0x3F);
}

/****************************************************************
 * @fn			- I2C_DeInit
 *
 * @brief		-
 *
 *
 * @return		- none
 *
 * @note		- none
 */
void I2C_DeInit(I2C_Handle_t *pI2CHandle)
{
	
}

uint8_t I2C_GetFlagStatus(I2C_RegDef_t* pI2Cx, uint32_t FlagName)
{
	if (pI2Cx->SR1 & FlagName)
	{
		return FLAG_SET;
	}
	return FLAG_RESET;
}

/****************************************************************
 * @fn			- I2C_Master_SendData
 *
 * @brief		-
 *
 *
 * @return		- none
 *
 * @note		- none
 */
void I2C_Master_SendData(I2C_Handle_t* pI2CHandle, uint8_t* pTxBuffer, uint32_t len, uint32_t slaveAddr)
{
	// 1. Generate the START condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);
	
	// 2. Confirm that start generation is completed bt checking the SB flag in the SR1
	// 	 	NOTE: Until SB is cleared, SCL will be stretched (pulled to LOW)
	while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB) == FLAG_RESET);
	
	// 3. Send the address of slave with r/nw bit
	I2C_ExecuteAddressPhaseWrite(pI2CHandle, slaveAddr);
	
	// 4. Confirm that address phase is completed by checking ADDR flag in SR1
	while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR) == FLAG_RESET);
	
	// 5. Clear the ADDR flag according to its software sequence
	// 	  NOTE: Until ADDR is cleared SCL will be stretched (pulled to LOW)
	I2C_ClearAddrFlag(pI2CHandle->pI2Cx);
	
	// 6. Send the data until Len becomes 0
	while (len > 0)
	{
		// Wait until TXE = 1 -> DR is empty and ready to receive data
		while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE) == FLAG_RESET); 
		pI2CHandle->pI2Cx->DR = *pTxBuffer;
		pTxBuffer++;
		len--;
	}
	
	// 7. When len becomes zero, waiting for TXE = 1 and BTF = 1 before generating the STOP condition
	//    NOTE:
	//	  1. TXE = 1, BTF = 1 means that both DR và Shift Register is empty and next tranmission should begin
	//	  2. When BTF = 1, SCL will be stretched (pulled to LOW)
	//	  3. Bit BTF is only set by hardware when DR và Shift Register is empty
	while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE) == FLAG_RESET);
	while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF) == FLAG_RESET); 
	
	// 8. Generate STOP condition and master not to wait for the completion of stop condition.
	//    NOTE: Generating STOP, bit BTF is automatically clear by hardware
	I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
}

/****************************************************************
 * @fn			- I2C_Master_ReceiveData
 *
 * @brief		-
 *
 *
 * @return		- none
 *
 * @note		- none
 */
void I2C_Master_ReceiveData(I2C_Handle_t* pI2CHandle, uint8_t* pRxBuffer, uint32_t len, uint32_t slaveAddr)
{
	// Enable the I2C peripheral 
	I2C_PeripheralControl(pI2CHandle->pI2Cx, ENABLE);
	
	// 1. Generate the START condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);
	
	// 2. Confirm that start generation is completed bt checking the SB flag in the SR1
	// 	 	NOTE: Until SB is cleared, SCL will be stretched (pulled to LOW)
	while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB) == FLAG_RESET);
	
	// 3. Send the address format to slave
	I2C_ExecuteAddressPhaseRead(pI2CHandle, slaveAddr);
	
	// 4. Confirm that address phase is completed by checking ADDR flag in SR1
	while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR) == FLAG_RESET);
	
	// 5.Implement data reception corresponding to different numbers of received bytes.
	if (len == 1)
	{
		// Clear bit ACK = 0 to generate NACK signal after receiving first byte
		pI2CHandle->pI2Cx->CR1 &= ~(1 << 10);
		
		// Clear the ADDR flag according to its software sequence
		I2C_ClearAddrFlag(pI2CHandle->pI2Cx);
		
		// Wait until RXNE = 1
		while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE) == FLAG_RESET); 
		
		// Generate STOP condition to close the communication after receiving the current byte
		I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
		
		*pRxBuffer = pI2CHandle->pI2Cx->DR;
	}
	else if (len == 2)
	{
		// Clear bit ACK = 0 and set bit POS = 1
		pI2CHandle->pI2Cx->CR1 &= ~(1 << 10);
		pI2CHandle->pI2Cx->CR1 |= (1 << 11);
		
		// Clear the ADDR flag according to its software sequence
		I2C_ClearAddrFlag(pI2CHandle->pI2Cx);
		
		// Wait until flag BTF = 1. At this moment, data 1 is in DR and data 2 is in Shift Register
		// NOTE: SCL stretched low until a data 1 is read
		while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF) == FLAG_RESET); 
		
		// Generate STOP condition to close the communication
		I2C_GenerateStopCondition(pI2CHandle->pI2Cx); 
		
		// Read data 1 and 2 from Data Register
		*pRxBuffer = pI2CHandle->pI2Cx->DR;
		pRxBuffer++;
		*pRxBuffer = pI2CHandle->pI2Cx->DR;
		
		// Clear bit POS
		pI2CHandle->pI2Cx->CR1 &= ~(1 << 11);
	}
	else if (len > 2)
	{	
		// Enable ACK bit after enabling I2C peripheral
		I2C_ManageAck(pI2CHandle->pI2Cx, I2C_ACK_ENABLE);
		
		// Clear the ADDR flag according to its software sequence
		I2C_ClearAddrFlag(pI2CHandle->pI2Cx);
		
		while (len > 0)
		{
			// Wait until RXNE = 1
			while (I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE) == FLAG_RESET); 
			
			// After we have received the byte N - 1, clear bit ACK and generate STOP condition
			if (len == 2)
			{
				// Clear bit ACK = 0 to generate NACK signal after receiving first byte
				pI2CHandle->pI2Cx->CR1 &= ~(1 << 10);
			
				// Generate STOP condition to close the communication
				I2C_GenerateStopCondition(pI2CHandle->pI2Cx); 
			}
			
			// Read the data from DR
			*pRxBuffer = pI2CHandle->pI2Cx->DR;
			pRxBuffer++;
			
			len--;
		}
	}
	
	// 6. Re-enable the ACK bit to operate in the next transfer.
	if(pI2CHandle->I2C_Config.I2C_AckControl == I2C_ACK_ENABLE)
    {
		I2C_PeripheralControl(pI2CHandle->pI2Cx, ENABLE);
        pI2CHandle->pI2Cx->CR1 |= (1 << 10);
    }
}
