#ifndef INC_STM32F407XXX_I2C_DRIVER_H_
#define INC_STM32F407XXX_I2C_DRIVER_H_

#include "stm32f407xx.h"

/*
 * Configuration structure for I2Cx peripheral
 */
typedef struct 
{
	uint32_t I2C_SCLSpeed;			/* Specifies the clock frequency */
	uint32_t I2C_FMDutyCycle;		/* Specifies the I²C fast mode duty cycle. */
	uint8_t I2C_AckControl;			/* Specifies the Acknowledge enable	*/
	uint32_t I2C_DeviceAddr;		/* Specifies the device own address. */
	uint32_t I2C_AddrMode;			/* Specifies if 7-bit or 10-bit addressing mode is selected. */
	uint32_t I2C_NoStretchMode;		/* Specifies if nostretch mode is selected. */
} I2C_Config_t;

/*
 * Handle structure for I2Cx peripheral
 */
typedef struct
{
	I2C_RegDef_t *pI2Cx;
	I2C_Config_t I2C_Config;
} I2C_Handle_t;

/*
 * I2C_SCLSpeed
 */
#define I2C_SCL_SPEED_SM			100000
#define I2C_SCL_SPEED_FM4K			400000
#define I2C_SCL_SPEED_FM2K			200000

/*
 * I2C_DeviceMode
 */
#define I2C_MODE_MASTER				1
#define I2C_MODE_SLAVE				0

/*
 * I2C_FMDUTYCYCLE
 */
#define I2C_FM_DUTY_2				0
#define I2C_FM_DUTY_16_9			1

/*
 * I2C_AckControl
 */
#define	I2C_ACK_ENABLE				1
#define I2C_ACK_DISABLE				0

/*
 * I2C_AddrMode
 */
#define I2C_ADDRESS_MODE_7_BITS			0
#define I2C_ADDRESS_MODE_10_BITS		1

/*
 * I2C_NoStretchMode
 */
#define I2C_NOSTRETCH_ENABLE				0
#define I2C_NOSTRETCH_DISABLE				1

/*
 * I2C related status flags definitions
 */
#define I2C_FLAG_SB					(1 << 0)
#define I2C_FLAG_ADDR				(1 << 1)
#define I2C_FLAG_BTF				(1 << 2)
#define I2C_FLAG_ADD10				(1 << 3)
#define I2C_FLAG_STOPF				(1 << 4)
#define I2C_FLAG_RXNE				(1 << 6)
#define I2C_FLAG_TXE				(1 << 7)
#define I2C_FLAG_BERR				(1 << 8)
#define I2C_FLAG_ARLO				(1 << 9)
#define I2C_FLAG_AF					(1 << 10)
#define I2C_FLAG_OVR				(1 << 11)
#define I2C_FLAG_PECERR				(1 << 12)
#define I2C_FLAG_TIMEOUT			(1 << 14)
#define I2C_FLAG_SMBALERT			(1 << 15)

/***************************************************************************************
 *						APIs support by this driver
 *		For more information about the APIs check the function definitions
 ***************************************************************************************/

/*	Peripheral clock	*/
void I2C_PeripheralClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi);

/*	Init and De-init	*/
void I2C_Init(I2C_Handle_t *pI2CHandle);
void I2C_DeInit(I2C_Handle_t *pI2CHandle);

void I2C_ManageAck(I2C_RegDef_t* pI2Cx, uint8_t EnorDi);

/*
 * Data send and receive
 */
void I2C_Master_SendData(I2C_Handle_t* pI2CHandle, uint8_t* pTxBuffer, uint32_t len, uint32_t slaveAddr);
void I2C_Master_ReceiveData(I2C_Handle_t* pI2CHandle, uint8_t* pRxBuffer, uint32_t len, uint32_t slaveAddr);

/*
 * IRQ Configurattion and ISR handling
 */
void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);

/*
 * Other peripheral Control APIs
 */
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi);
uint8_t I2C_GetFlagStatus(I2C_RegDef_t* pI2Cx, uint32_t FlagName);

/*
 * Application callback
 */
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv);

#endif