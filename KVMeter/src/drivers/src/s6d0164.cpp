
#include <stdio.h>
#include <stdarg.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_dma.h"
#include "delay.h"
#include "s6d0164.h"
#include <fonts.h>

S6D0164::S6D0164() {
	isDataSending = 0;
	manualCsControl = 0;
	disableDMA = 0;

	color = WHITE;
	bgColor = BLACK;
	
	font = Consolas8x14;
	isLandscape = true;
	isOk = false;
}

S6D0164::~S6D0164() {
}

void S6D0164::setupHw(SPI_TypeDef* spi, uint16_t spiPrescaler, GPIO_TypeDef* controlPort, uint16_t rsPin, uint16_t resetPin, uint16_t csPin) {
	this->spi = spi;
	this->spiPrescaler = spiPrescaler;
	this->controlPort = controlPort;
	this->rsPin = rsPin;
	this->resetPin = resetPin;
	this->csPin = csPin;

	/* RS, RESET & CS pins of S6D0164 */
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = rsPin | resetPin | csPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(controlPort, &GPIO_InitStructure);
}

void S6D0164::setSPISpeed(uint16_t prescaler) {
	SPI_Cmd(spi, DISABLE);
	spi->CR1 &= ~SPI_CR1_BR; // Clear SPI baud rate bits
	spi->CR1 |= prescaler; // Set SPI baud rate bits
	SPI_Cmd(spi, ENABLE);
}

void S6D0164::init(void) {

	setSPISpeed(SPI_BaudRatePrescaler_16);
	switchCs(0);
	switchReset(0);
	DelayManager::DelayMs(5);
	switchReset(1);

	sendCmd(S6D0164_SOFT_RESET_REG); //software reset
	DelayManager::DelayMs(5);
	sendCmd(S6D0164_DISPLAYOFF_REG); // display off

	sendCmd(S6D0164_POWERCTL1_REG);    	//Power control
	sendData(0x23);   	//VRH[5:0]

	sendCmd(S6D0164_POWERCTL2_REG);    	//Power control
	sendData(0x10);   	//SAP[2:0];BT[3:0]

	sendCmd(S6D0164_VCOMCTL1_REG);    	//VCM control
	sendData(0x3E); //Contrast
	sendData(0x28);

	sendCmd(S6D0164_VCOMCTL2_REG);    	//VCM control2
	sendData(0x86);

	sendCmd(S6D0164_MEMACCESS_REG);    	// Memory Access Control
	sendData(0x48);  //my,mx,mv,ml,BGR,mh,0.0

	sendCmd(S6D0164_PIXFORMATSET_REG);
	sendData(0x55);

	sendCmd(S6D0164_FRAMECTL_NOR_REG);
	sendData(0x00);
	sendData(0x18);

	sendCmd(S6D0164_FUNCTONCTL_REG);    	// Display Function Control
	sendData(0x08);
	sendData(0x82);
	sendData(0x27);

	sendCmd(S6D0164_ENABLE_3G_REG);    	// 3Gamma Function Disable
	sendData(0x00);

	sendCmd(S6D0164_GAMMASET_REG);    	//Gamma curve selected
	sendData(0x01);

	sendCmd(S6D0164_POSGAMMACORRECTION_REG);    	//Set Gamma
	sendData(0x0F);
	sendData(0x31);
	sendData(0x2B);
	sendData(0x0C);
	sendData(0x0E);
	sendData(0x08);
	sendData(0x4E);
	sendData(0xF1);
	sendData(0x37);
	sendData(0x07);
	sendData(0x10);
	sendData(0x03);
	sendData(0x0E);
	sendData(0x09);
	sendData(0x00);


	sendCmd(S6D0164_NEGGAMMACORRECTION_REG);    	//Set Gamma
	sendData(0x00);
	sendData(0x0E);
	sendData(0x14);
	sendData(0x03);
	sendData(0x11);
	sendData(0x07);
	sendData(0x31);
	sendData(0xC1);
	sendData(0x48);
	sendData(0x08);
	sendData(0x0F);
	sendData(0x0C);
	sendData(0x31);
	sendData(0x36);
	sendData(0x0F);

	sendCmd(S6D0164_SLEEP_OUT_REG);    	//Exit Sleep
	DelayManager::DelayMs(100);
	sendCmd(S6D0164_DISPLAYON_REG);   	//Display on
	DelayManager::DelayMs(100);

	//sendCmd(S6D0164_WRITEBRIGHT_REG);   	//Change brightness
	//sendData(0x50);
	
	switchCs(1);
	setSPISpeed(this->spiPrescaler);
	isOk = true;
}


void S6D0164::enable(short on) {
	switchCs(0);
	if (on == 0) {
		sendCmd(S6D0164_DISPLAYOFF_REG);
		isOk = false;
	} else {
		sendCmd(S6D0164_DISPLAYON_REG);
		isOk = true;
	}
	switchCs(1);
	DelayManager::DelayMs(100);
}


void S6D0164::sleep(short on){
	switchCs(0);
	if (on == 0) {
		sendCmd(S6D0164_SLEEP_OUT_REG);
	} else {
		sendCmd(S6D0164_SLEEP_ENTER_REG);
	}
	switchCs(1);
	DelayManager::DelayMs(100);
}

void S6D0164::clear(uint16_t color)
{
	if (isLandscape)
	{
		fillScreen(TFT_MIN_X, TFT_MIN_Y, TFT_MAX_X, TFT_MAX_Y, color);
	}
	else
	{
		fillScreen(TFT_MIN_Y, TFT_MIN_X, TFT_MAX_Y, TFT_MAX_X, color);
	}
}

void S6D0164::setCol(uint16_t StartCol, uint16_t EndCol)
{
	sendCmd(S6D0164_COLADDRSET_REG);    // Column Command address
	sendWords(StartCol, EndCol);
}

void S6D0164::setPage(uint16_t StartPage, uint16_t EndPage)
{
	sendCmd(S6D0164_PAGEADDRSET_REG);   // Page Command address
	sendWords(StartPage, EndPage);
}

void S6D0164::setWindow(uint16_t startX, uint16_t startY, uint16_t stopX, uint16_t stopY)
{
	if (isLandscape)
	{
		setPage(startX, stopX);
		setCol(startY, stopY);	
	}
	else
	{
		setPage(TFT_MAX_X - stopY, TFT_MAX_X - startY);
		setCol(startX, stopX);	
	}
}	

void S6D0164::setFont(const unsigned char font[])
{
	this->font = font;
}

void S6D0164::setLandscape()
{
	isLandscape = true;
}

void S6D0164::setPortrait()
{
	isLandscape = false;
}

void S6D0164::fillScreen(uint16_t xstart, uint16_t ystart, uint16_t xstop, uint16_t ystop, uint16_t color)
{
	uint32_t pixels = (xstop - xstart + 1) * (ystop - ystart + 1);
	while (this->isDataSending); //wait until all data wasn't sended
	switchCs(0);   // CS=0;
	setWindow(xstart, ystart, xstop, ystop);
	sendCmd(S6D0164_MEMORYWRITE_REG);

	switchRs(1);

	if (!disableDMA) {
		manualCsControl = 1;
		if (pixels > 65535) {
			initDMAforSendSPI(&color, 65535, 1);
			DMATXStart();
			pixels -= 65535;
			while (this->isDataSending); //wait until all data wasn't sended
		}
		manualCsControl = 0;
		initDMAforSendSPI(&color, pixels + 2, 1);
		DMATXStart();
		while (this->isDataSending); //wait until all data wasn't sended
	}
	else 
	{
		SPI_DataSizeConfig(spi, SPI_DataSize_16b);
		while (pixels) {
			sendWord16bitMode(color);
			pixels--;
		}
		SPI_DataSizeConfig(spi, SPI_DataSize_8b);
		switchCs(1);   // CS=1;
	}
}

void S6D0164::pixelDraw(uint16_t xpos, uint16_t ypos, uint16_t color)
{
	while (this->isDataSending); //wait until all data wasn't sended
	switchCs(0);   // CS=0;
	setWindow(xpos, ypos, xpos, ypos);
	sendCmd(S6D0164_MEMORYWRITE_REG);
	sendWord(color);
	switchCs(1);   // CS=1;
}

void S6D0164::bufferDraw(uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize, uint16_t* buf)
{
	switchCs(0);   // CS=0
	setWindow(x, y, x + xsize - 1, y + ysize - 1);
	sendCmd(S6D0164_MEMORYWRITE_REG);
	switchRs(1);
	
	if (!disableDMA) {
		initDMAforSendSPI(buf, xsize * ysize, 0);
		DMATXStart();
	}
	else 
	{
		uint32_t l;
		SPI_DataSizeConfig(spi, SPI_DataSize_16b);
		for (l = 0; l < xsize * ysize; l++) {
			sendWord16bitMode(buf[l]);
		}
		SPI_DataSizeConfig(spi, SPI_DataSize_8b);
		switchCs(1);   // CS=1;
	}
	while (this->isDataSending); //wait until all data wasn't sended
}

void S6D0164::lineDraw(uint16_t ypos, uint16_t* line,  uint32_t size)
{
	bufferDraw(0, ypos, size, 1, line);
}

void S6D0164::drawBorder(uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, uint16_t bw, uint16_t color)
{
	fillScreen(xpos, ypos, xpos + bw, ypos + height, color);
	fillScreen(xpos + bw, ypos + height - bw, xpos + width, ypos + height, color);
	fillScreen(xpos + width - bw, ypos, xpos + width, ypos + height - bw, color);
	fillScreen(xpos + bw, ypos, xpos + width - bw, ypos + bw, color);
}

void S6D0164::drawBattery(uint16_t x, uint16_t y, uint8_t level, uint16_t color, uint16_t bkgColor) {
	//battery 25x10
	uint8_t width = 26;
	uint8_t height = 10;
	uint16_t buf[width * height];
	uint16_t t = 0;
	uint8_t i, j;
	uint16_t innerColor = getColorByLevel(level);
	
	uint16_t lcol;
	uint8_t div = (width - 6) / 5;
		
	if (isLandscape)
	{
		for (i = 0; i < 2; i++) //battery cup
		{
			buf[t++] = bkgColor; 
			buf[t++] = bkgColor; 
			buf[t++] = bkgColor; 
			buf[t++] = color; 
			buf[t++] = color; 
			buf[t++] = color;
			buf[t++] = color; 
			buf[t++] = bkgColor; 
			buf[t++] = bkgColor; 
			buf[t++] = bkgColor;
		}
		for (i = 0; i < height; i++)
		{
			buf[t++] = color;	
		}
		buf[t++] = color;
		for (j = 0; j < height - 2; j++)
		{
			buf[t++] = bkgColor;	
		}
		buf[t++] = color;
	
		for (i = 0; i < width - 6; i++)
		{
			buf[t++] = color;
			buf[t++] = bkgColor;
		
			if (((width - 6) - i) <= level*div)
			{
				lcol = innerColor;
			}
			else
			{
				lcol = bkgColor;
			}
			for (j = 0; j < height - 4; j++)
			{
				buf[t++] = lcol;	
			}
		
			buf[t++] = bkgColor;
			buf[t++] = color;
		}
		buf[t++] = color;
	
		for (j = 0; j < height - 2; j++)
		{
			buf[t++] = bkgColor;	
		}
		buf[t++] = color;
		for (i = 0; i < height; i++) //floor
		{
			buf[t++] = color;	
		}	
	}
	else //for portrait orientation
	{
		div = width / 5;
		
		buf[t++] = bkgColor;
		buf[t++] = bkgColor;
		for (i = 0; i < width - 2; i++)
		{
			buf[t++] = color;	
		}
		for (j = 0; j < 2; j++)
		{
			buf[t++] = bkgColor;
			buf[t++] = bkgColor;
			buf[t++] = color;
			for (i = 0; i < width - 4; i++)
			{
				if (((width - 4) - i) <= level*div)
				{
					buf[t++] = innerColor;
				}
				else
				{
					buf[t++] = bkgColor;
				}
			}
			buf[t++] = color;
		}
		for (j = 0; j < 4; j++)
		{
			buf[t++] = color;
			buf[t++] = color;
			buf[t++] = color;
			for (i = 0; i < width - 4; i++)
			{
				if (((width - 4) - i) <= level*div)
				{
					buf[t++] = innerColor;
				}
				else
				{
					buf[t++] = bkgColor;
				}	
			}
			buf[t++] = color;
		}
		for (j = 0; j < 2; j++)
		{
			buf[t++] = bkgColor;
			buf[t++] = bkgColor;
			buf[t++] = color;
			for (i = 0; i < width - 4; i++)
			{
				if (((width - 4) - i) <= level*div)
				{
					buf[t++] = innerColor;
				}
				else
				{
					buf[t++] = bkgColor;
				}	
			}
			buf[t++] = color;
		}
		buf[t++] = bkgColor;
		buf[t++] = bkgColor;
		for (i = 0; i < width - 2; i++)
		{
			buf[t++] = color;	
		}
	}
	
	bufferDraw(x, y, width, height, buf);
}

void S6D0164::putChar(uint16_t x, uint16_t y, uint8_t chr, uint16_t charColor, uint16_t bkgColor) {
	uint8_t i, j;
	
	uint8_t f_width = font[0];	
	uint8_t f_height = font[1];
	uint16_t f_bytes = (f_width * f_height / 8);
	
	uint16_t t = 0;
	uint16_t charbuf[(f_width + 1) * f_height];
	
	//fill charbuf
	if (isLandscape)
	{
		for (i = 0; i < f_width; i++)
		{
			for (j = 0; j < f_height; j++) {
				uint16_t bitNumberGlobal = f_width * (f_height - j) + (f_width - i);
				uint16_t byteNumberLocal = (bitNumberGlobal / 8);
				uint8_t bitNumberInByte = bitNumberGlobal - byteNumberLocal * 8;
				uint8_t glyphByte = font[(chr - 0x20) * f_bytes + byteNumberLocal + 2];
				uint8_t mask = 1 << bitNumberInByte;
				if (glyphByte & mask) {
					charbuf[t++] = charColor;
				}
				else 
				{
					charbuf[t++] = bkgColor;
				}
			}
		}
		for (j = 0; j < f_height; j++) { //vertical empty line right from symbol
			charbuf[t++] = bkgColor;
		}
	}
	else //portrait
	{
		for (i = 0; i < f_height; i++)
		{
			for (j = 0; j < f_width; j++) {
				uint16_t bitNumberGlobal = f_width * i + (f_width - j);
				uint16_t byteNumberLocal = (bitNumberGlobal / 8);
				uint8_t bitNumberInByte = bitNumberGlobal - byteNumberLocal * 8;
				uint8_t glyphByte = font[(chr - 0x20) * f_bytes + byteNumberLocal + 2];
				uint8_t mask = 1 << bitNumberInByte;
				if (glyphByte & mask) {
					charbuf[t++] = charColor;
				}
				else 
				{
					charbuf[t++] = bkgColor;
				}
			}
			charbuf[t++] = bkgColor; //vertical empty line right from symbol
		}
		y -= 3;
	}
	
	bufferDraw(x, y, f_width + 1, f_height, charbuf);
}

void S6D0164::putString(const char str[], uint16_t x, uint16_t y, uint16_t charColor, uint16_t bkgColor) {
	while (this->isDataSending); //wait until all data wasn't sended
	while (*str != 0) {
		putChar(x, y, *str, charColor, bkgColor);
		x += font[0]; //increment to font width
		str++;
	}
}

void S6D0164::printf(uint16_t x, uint16_t y, uint16_t charColor, uint16_t bkgColor, const char *format, ...) {
	char buf[40];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	putString(buf, x, y, charColor, bkgColor);
}

void S6D0164::printf(uint16_t x, uint16_t y, const char *format, ...)
{
	char buf[40];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	putString(buf, x, y, color, bgColor);
}

uint32_t S6D0164::readID() {
	uint32_t data;
	switchCs(0);   // CS=0
	sendCmd(S6D0164_READID1_REG);
	switchRs(1);
	spi->DR = 0x00; //ignore first byte
	data = readByte();
	switchCs(1);   // CS=1
	data <<= 8;

	switchCs(0);   // CS=0
	sendCmd(S6D0164_READID2_REG);
	switchRs(1);
	spi->DR = 0x00; //ignore first byte
	data |= readByte();
	switchCs(1);   // CS=1
	data <<= 8;

	switchCs(0);   // CS=0
	sendCmd(S6D0164_READID3_REG);
	switchRs(1);
	spi->DR = 0x00; //ignore first byte
	data |= readByte();
	switchCs(1);   // CS=1
	data <<= 8;

	switchCs(0);   // CS=0
	sendCmd(S6D0164_READID4_REG);
	switchRs(1);
	spi->DR = 0x00; //ignore first byte
	data |= readByte();
	switchCs(1);   // CS=1
	
	return data;
}


uint8_t S6D0164::readByte()
{
	uint8_t data;
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET);
	data = spi->DR;
	return data;
}

void S6D0164::sendByteInt(uint8_t byte)
{
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET);
	spi->DR = byte;
}

void S6D0164::sendWordInt(uint16_t data)
{
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET);
	spi->DR = data;
}

void S6D0164::sendCmd(uint8_t index)
{
	switchRs(0);
	sendByteInt(index);
    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_BSY) != RESET);
}

void S6D0164::sendData(uint8_t data)
{
	switchRs(1);
	sendByteInt(data);
    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_BSY) != RESET);
}

void S6D0164::sendWord(uint16_t data)
{
	switchRs(1);
	sendByteInt(data >> 8);
	sendByteInt(data & 0xFF);
    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_BSY) != RESET);
}

void S6D0164::sendWord16bitMode(uint16_t data)
{
	switchRs(1);
	sendWordInt(data);
    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_BSY) != RESET);
}

void S6D0164::sendWords(uint16_t data1, uint16_t data2)
{
	switchRs(1);
	sendByteInt(data1 >> 8);
	sendByteInt(data1 & 0xFF);
	sendByteInt(data2 >> 8);
	sendByteInt(data2 & 0xFF);
    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_BSY) != RESET);
}

void S6D0164::initDMAforSendSPI(uint16_t* buffer, uint32_t size, uint8_t singleColor)
{
	DMA_InitTypeDef  DMA_InitStructure;
	//channel3 for SPI1_TX, channel5 for SPI2_TX	
	DMA_DeInit(((spi == SPI1) ? DMA1_Channel3 : DMA1_Channel5));
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(spi->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = size;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = singleColor ? DMA_MemoryInc_Disable : DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(((spi == SPI1) ? DMA1_Channel3 : DMA1_Channel5), &DMA_InitStructure);
}

void S6D0164::DMATXStart()
{
	if (!isOk) return;
	SPI_DataSizeConfig(spi, SPI_DataSize_16b);
	//channel3 for SPI1_TX, channel5 for SPI2_TX
	DMA_Cmd(((spi == SPI1) ? DMA1_Channel3 : DMA1_Channel5), ENABLE);
	// Enable the SPI TX DMA request
	SPI_I2S_DMACmd(spi, SPI_I2S_DMAReq_Tx, ENABLE);
	DMA_ITConfig(((spi == SPI1) ? DMA1_Channel3 : DMA1_Channel5), DMA_IT_TC, ENABLE);
	this->isDataSending = 1;
}

void S6D0164::DMATXInterrupt()
{
	if (!isOk) return;
	DMA_ClearITPendingBit((spi == SPI1) ? DMA1_IT_TC3 : DMA1_IT_TC5);
	DMA_Cmd(((spi == SPI1) ? DMA1_Channel3 : DMA1_Channel5), DISABLE);
	SPI_DataSizeConfig(spi, SPI_DataSize_8b);
	if (!manualCsControl) {
		switchCs(1);   // CS=1;
	}
	this->isDataSending = 0;
}


uint8_t S6D0164::IsDataSending()
{
	return this->isDataSending;
}


void S6D0164::resetIsDataSending()
{
	this->isDataSending = 0;
}

bool S6D0164::isReady()
{
	return this->isOk;
}

void S6D0164::setDisableDMA(uint8_t isDisable)
{
	this->disableDMA = isDisable; 
}


void S6D0164::setColor(uint16_t color, uint16_t bgColor)
{
	this->color = color;
	this->bgColor = bgColor;
}

uint16_t S6D0164::RGB888ToRGB565(uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t r5 = (uint16_t)((r * 249 + 1014) >> 11);
	uint16_t g6 = (uint16_t)((g * 253 + 505) >> 10);
	uint16_t b5 = (uint16_t)((b * 249 + 1014) >> 11);
	return (uint16_t)(r5 << 11 | g6 << 5 | b5);
}

void S6D0164::switchCs(short BitVal)
{
	if (BitVal != Bit_RESET) {
		controlPort->BSRR = csPin;
	} else {
		controlPort->BRR = csPin;
	}
}

void S6D0164::switchRs(short BitVal)
{
	if (BitVal != Bit_RESET) {
		controlPort->BSRR = rsPin;
	} else {
		controlPort->BRR = rsPin;
	}
}

void S6D0164::switchReset(short BitVal)
{
	if (BitVal != Bit_RESET) {
		controlPort->BSRR = resetPin;
	} else {
		controlPort->BRR = resetPin;
	}
}

uint16_t S6D0164::getColorByLevel(uint8_t level)
{
	if (level == 0)
	{
		return BLACK;
	} 
	if (level == 1)
	{
		return RED;
	} 
	if (level == 2)
	{
		return YELLOW;
	}
	if (level == 3)
	{
		return WHITE;
	}
	return GREEN;
}