/**
  ******************************************************************************
  * @file    main.c
  * @author  ��������� �.�.
  * @version V3.2
  * @date    05.10.20
  * @brief   ��������� ���������� ��� ������ ��������
   ******************************************************************************
**/

#include "stm8s.h"

uint8_t keyNo;
uint8_t firstByte;
uint8_t secondByte;
uint8_t thirdByte;
uint8_t fourthByte;
uint8_t fifthByte;
uint8_t sixthByte;
uint8_t row[8] = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f};
uint8_t Otkl = 0;

void SomeDelay(uint16_t DelValue) // ������� ��������
{
	for (DelValue; DelValue > 0; DelValue--);
}

void initializePorts(void) // ������������� ������ ��
{
        // ROWS
	GPIO_Init (GPIOD, GPIO_PIN_ALL, GPIO_MODE_OUT_PP_HIGH_FAST); // Output push-pull, high level, 10MHz
	// COLUMNS
	GPIO_Init (GPIOB, GPIO_PIN_ALL, GPIO_MODE_IN_PU_NO_IT); // Input pull-up, no external interrupt
	// SPI, ������� ���� ������� ����
	SPI_Init (SPI_FIRSTBIT_MSB, SPI_BAUDRATEPRESCALER_2, SPI_MODE_MASTER, SPI_CLOCKPOLARITY_LOW, 
	          SPI_CLOCKPHASE_1EDGE, SPI_DATADIRECTION_1LINE_TX, SPI_NSS_HARD, (uint8_t)0x07);
	SPI_Cmd(ENABLE);
	// RESET
	GPIO_Init (GPIOC, GPIO_PIN_7, GPIO_MODE_IN_PU_IT); // Input pull-up, external interrupt
	EXTI_SetExtIntSensitivity (EXTI_PORT_GPIOC, EXTI_SENSITIVITY_RISE_ONLY); // ���������� ��� ��������� 1 �� PC7
	enableInterrupts(); // ��������� ����������
	// OE ��������� ������ (��������� ������)
	GPIO_Init (GPIOC, GPIO_PIN_1, GPIO_MODE_OUT_PP_HIGH_FAST); // Output push-pull, high level, 10MHz
	GPIO_WriteLow (GPIOC, GPIO_PIN_1); // ��������� ������� ��������� ������
	// SRCLR �������� ������ (��������� ������)
        GPIO_Init (GPIOC, GPIO_PIN_2, GPIO_MODE_OUT_PP_LOW_FAST); // Output push-pull, low level, 10MHz - ��� ������ ��������� ������
        SomeDelay(8000);
	GPIO_Init (GPIOC, GPIO_PIN_2, GPIO_MODE_OUT_PP_HIGH_FAST); // Output push-pull, high level, 10MHz
	// NSS1 ��� ��������� ������ (��������� ������)
	GPIO_Init (GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST); // Output push-pull, high level, 10MHz
	// NSS2 ��� �������� �� ���� (��������� ������)
	GPIO_Init (GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST); // Output push-pull, high level, 10MHz
}

void sendBytes(uint8_t firstByte, uint8_t secondByte, uint8_t thirdByte, 
               uint8_t fourthByte, uint8_t fifthByte, uint8_t sixthByte) // ������ ������ via SPI (6 ��������� ������ - 6 ����)
{
		GPIO_WriteLow (GPIOC, GPIO_PIN_3); // NSS1
		
		SPI_SendData (firstByte); // D10
		while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) { // ���� ���� ����� �������� ������ ����
		}
		SPI_SendData (secondByte); // D5
		while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) {
		}
		SPI_SendData (thirdByte); // D4
		while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) {
		}
		SPI_SendData (fourthByte); // D3
		while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) {
		}
		SPI_SendData (fifthByte); // D2
		while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) {
		}
		SPI_SendData (sixthByte); // D1
		while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) {
		}
		while (SPI_GetFlagStatus(SPI_FLAG_BSY) == SET) { // ���� ���� ���������� ������
		}
		
		GPIO_WriteHigh (GPIOC, GPIO_PIN_3);
}

uint8_t getKeyNo(uint8_t row) // ����������� 0 �� ������ � ���������� ��������
{
	GPIO_Write (GPIOD, row); // 1-8 ������	
	Otkl = 0;
	for (Otkl; Otkl < 40; Otkl++) { // ���� ������� ������ 25, �� ��� ������ �����������. ��� ������, ��� ������ ������� ������������ ���������� ��� ������� ������
          SomeDelay(8000);
	}
	keyNo = GPIO_ReadInputData (GPIOB) & 0x3f;  // 0011 1111 ����� ��� ��������� ��������� PB6, PB7
	return(keyNo);
}

void sendByteFPGA(uint8_t ByteFPGA) // ������ ����� � ���� via SPI
{
		GPIO_WriteLow (GPIOC, GPIO_PIN_4); // NSS2
		
		SPI_SendData (ByteFPGA); // D6
		while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) { // ���� ���� ����� �������� ������ ����
		}
		while (SPI_GetFlagStatus(SPI_FLAG_BSY) == SET) { // ���� ���� ���������� ������
		}
		
		GPIO_WriteHigh (GPIOC, GPIO_PIN_4);
}

INTERRUPT_HANDLER(IRQ_Handler_EXTI_PORT_C, 5) // ���������� ����������
{
	sendByteFPGA(0);
	sendBytes(0,0,0,0,0,0);
        WWDG_SWReset(); // software reset, used Window watchdog (WWDG)
}

void writeByte(uint8_t Byte, uint8_t Mask) // ������������ + ������ ������ via SPI
{
	switch (Byte) {
		case 1 : 
			firstByte = firstByte ^ Mask; // �������� �� ������ 2
			break;
		
		case 2 : 
			secondByte = secondByte ^ Mask;
			break;
		
		case 3 : 
			thirdByte = thirdByte ^ Mask;
			break;
		
		case 4 : 
			fourthByte = fourthByte ^ Mask;
			break;
		
		case 5 : 
			fifthByte = fifthByte ^ Mask;
			break;
			
		case 6 : 
			sixthByte = sixthByte ^ Mask;
			break;
	}
	sendBytes(firstByte,secondByte,thirdByte,fourthByte,fifthByte,sixthByte);
}

void getMaskRow1(void) // ����� ������� ������ � ������ ������
{
	switch (keyNo) {
		case 0x01 :
			sendByteFPGA(0x02);
			writeByte (2, 0x01);
			break;
		
		case 0x02 :
			sendByteFPGA(0x03);
			writeByte (2, 0x02);
			break;
		
		case 0x04 :
			sendByteFPGA(0x04);
			writeByte (2, 0x04);
			break;
		
		case 0x08 :
			sendByteFPGA(0x05);
			writeByte (2, 0x08);
			break;
		
		case 0x10 :
			sendByteFPGA(0x19);
			writeByte (6, 0x40);
			break;
			
		case 0x20 :
			sendByteFPGA(0x1a);
			writeByte (6, 0x80);
			break;
	}
}

void getMaskRow2(void) // ������� SWIM/ROW2 JUMPER!
{
	switch (keyNo) {
		case 0x01 :
			sendByteFPGA(0x13);
			writeByte (6, 0x01);
			break;
		
		case 0x02 :
			sendByteFPGA(0x14);
			writeByte (6, 0x02);
			break;
		
		case 0x04 :
			sendByteFPGA(0x15);
			writeByte (6, 0x04);
			break;
		
		case 0x08 :
			sendByteFPGA(0x16);
			writeByte (6, 0x08);
			break;
		
		case 0x10 :
			sendByteFPGA(0x17);
			writeByte (6, 0x10);
			break;
			
		case 0x20 :
			sendByteFPGA(0x18);
			writeByte (6, 0x20);
			break;
	}
}

void getMaskRow3(void)
{
	switch (keyNo) {
		case 0x01 :
			sendByteFPGA(0x07);
			writeByte (5, 0x01);
			break;
		
		case 0x02 :
			sendByteFPGA(0x08);
			writeByte (5, 0x02);
			break;
		
		case 0x04 :
			sendByteFPGA(0x09);
			writeByte (5, 0x04);
			break;
		
		case 0x08 :
			sendByteFPGA(0x0a);
			writeByte (5, 0x08);
			break;
		
		case 0x10 :
			sendByteFPGA(0x0b);
			writeByte (5, 0x10);
			break;
			
		case 0x20 :
			sendByteFPGA(0x0c);
			writeByte (5, 0x20);
			break;
	}
}

void getMaskRow4(void)
{
	switch (keyNo) {
		case 0x01 :
			sendByteFPGA(0x24);
			writeByte (3, 0x01);
			break;
		
		case 0x02 :
			sendByteFPGA(0x25);
			writeByte (3, 0x02);
			break;
		
		case 0x04 :
			sendByteFPGA(0x26);
			writeByte (3, 0x04);
			break;
		
		case 0x08 :
			sendByteFPGA(0x27);
			writeByte (3, 0x08);
			break;
		
		case 0x10 :
			sendByteFPGA(0x28);
			writeByte (3, 0x10);
			break;
			
		case 0x20 :
			sendByteFPGA(0x29);
			writeByte (3, 0x20);
			break;
	}
}

void getMaskRow5(void)
{
	switch (keyNo) {
		case 0x01 :
			sendByteFPGA(0x0d);
			writeByte (4, 0x01);
			break;
		
		case 0x02 :
			sendByteFPGA(0x0e);
			writeByte (4, 0x02);
			break;
		
		case 0x04 :
			sendByteFPGA(0x0f);
			writeByte (4, 0x04);
			break;
		
		case 0x08 :
			sendByteFPGA(0x10);
			writeByte (4, 0x08);
			break;
		
		case 0x10 :
			sendByteFPGA(0x11);
			writeByte (4, 0x10);
			break;
			
		case 0x20 :
			sendByteFPGA(0x12);
			writeByte (4, 0x20);
			break;
	}
}

void getMaskRow6(void)
{
	switch (keyNo) {
		case 0x01 :
			sendByteFPGA(0x23);
			writeByte (3, 0x80);
			break;
		
		case 0x02 :
			sendByteFPGA(0x21);
			writeByte (1, 0x80);
			break;
		
		case 0x04 :
			sendByteFPGA(0x22);
			writeByte (3, 0x40);
			break;
		
		case 0x08 :
			sendByteFPGA(0x20);
			writeByte (5, 0x40);
			break;
		
		case 0x10 :
			sendByteFPGA(0x01);
			writeByte (5, 0x80);
			break;
			
		case 0x20 :
			sendByteFPGA(0x06);
			writeByte (4, 0x40);
			break;
	}
}

void getMaskRow7(void)
{
	switch (keyNo) {
		case 0x01 :
			sendByteFPGA(0x1e);
			writeByte (2, 0x10);
			break;
		
		case 0x02 :
			sendByteFPGA(0x1f);
			writeByte (2, 0x20);
			break;
		
		case 0x04 :
			sendByteFPGA(0x1b);
			writeByte (4, 0x80);
			break;
		
		case 0x08 :
			sendByteFPGA(0x1c);
			writeByte (2, 0x40);
			break;
		
		case 0x10 :
			sendByteFPGA(0x1d);
			writeByte (2, 0x80);
			break;
			
		case 0x20 :
			sendByteFPGA(0x2a);
			writeByte (1, 0x01);
			break;
	}
}

void getMaskRow8(void)
{
	switch (keyNo) {
		case 0x01 :
			sendByteFPGA(0x2b);
			writeByte (1, 0x02);
			break;
		
		case 0x02 :
			sendByteFPGA(0x2c);
			writeByte (1, 0x04);
			break;
		
		case 0x04 :
			sendByteFPGA(0x2d);
			writeByte (1, 0x08);
			break;
		
		case 0x08 :
			sendByteFPGA(0x2e);
			writeByte (1, 0x10);
			break;
		
		case 0x10 :
			sendByteFPGA(0x2f);
			writeByte (1, 0x20);
			break;
			
		case 0x20 :
			sendByteFPGA(0x30);
			writeByte (1, 0x40);
			break;
	}
}

uint8_t getMask(uint8_t row) // ����� ������ ��� ������, � ����������� �� �������� ������
{	
        if (keyNo == 0) { // ���� ������ �� ������
            return(0);
        }
        
	switch (row) {
		case 0xfe :
			getMaskRow1();
			break;
		
		case 0xfd : 
			getMaskRow2();
			break;
		
		case 0xfb : 
			getMaskRow3();
			break;
		
		case 0xf7 : 
			getMaskRow4();
			break;
		
		case 0xef : 
			getMaskRow5();
			break;
		
		case 0xdf : 
			getMaskRow6();
			break;
		
		case 0xbf : 
			getMaskRow7();
			break;
		
		case 0x7f : 
			getMaskRow8();
			break;
		}
}

void main(void)
{
        for (Otkl; Otkl < 50; Otkl++) { // �������� ����� ������� ���������
          SomeDelay(8000);
        }
	CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // f = 16 MHz
	initializePorts();
        sendByteFPGA(0); // ��������� ����� ������� ���������
	
        uint8_t i = 0;
	while (1) // ������������ ���� ������ ��������� ����������
	{
          for (i; i < 8; i++) {
            getKeyNo(row[i]);
            getMask(row[i]);
          }
          i = 0;
	}
}