
#include <stdio.h>
#include <stdarg.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "delay.h"
#include "s6d0164.h"
#include <fonts.h>

S6D0164::S6D0164() {
	isDataSending = 0;
	rotationMode = PORTRAIT;
	color = WHITE;
	bgColor = BLACK;
	
	font = Consolas8x14;
	isOk = false;
}

S6D0164::~S6D0164() {
}

void S6D0164::setupHw(GPIO_TypeDef* dataPort, GPIO_TypeDef* controlPort, const uint16_t rdPin, const uint16_t wrPin, const uint16_t rsPin, const uint16_t csPin, const uint16_t resetPin) {
	this->dataPort = dataPort;
	this->controlPort = controlPort;
	this->rdPin = rdPin;
	this->wrPin = wrPin;
	this->rsPin = rsPin;
	this->csPin = csPin;
	this->resetPin = resetPin;

	switchRd(1);
	switchWr(1);
	switchRs(1);
	switchCs(1);
	switchReset(1);
}


void S6D0164::switchToReadMode()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = 0x00FF; //lower 8 bit
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(dataPort, &GPIO_InitStructure);
}

void S6D0164::switchToWriteMode()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = 0x00FF; //lower 8 bit
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(dataPort, &GPIO_InitStructure);
}

void S6D0164::init() {

	switchReset(1);
	DelayManager::DelayMs(5);
	switchReset(0);
	DelayManager::DelayMs(15);
	switchReset(1);
	DelayManager::DelayMs(15);

	switchCs(0);

	sendCmdAndData(S6D0164_POWER_2, 0x001A);
	sendCmdAndData(S6D0164_POWER_3, 0x3121);
	sendCmdAndData(S6D0164_POWER_4, 0x006C);
	sendCmdAndData(S6D0164_POWER_5, 0x4249);

	sendCmdAndData(S6D0164_POWER_1, 0x0800);
	DelayManager::DelayMs(10);
	sendCmdAndData(S6D0164_POWER_2, 0x011A);
	DelayManager::DelayMs(10);	
	sendCmdAndData(S6D0164_POWER_2, 0x031A);
	DelayManager::DelayMs(10);
	sendCmdAndData(S6D0164_POWER_2, 0x071A);
	DelayManager::DelayMs(10);
	sendCmdAndData(S6D0164_POWER_2, 0x0F1A);
	DelayManager::DelayMs(10);	
	sendCmdAndData(S6D0164_POWER_2, 0x0F3A);
	DelayManager::DelayMs(30);

	sendCmdAndData(S6D0164_OUTPUT, 0x011C);
	sendCmdAndData(S6D0164_WAVEFORM, 0x0100);
	sendCmdAndData(S6D0164_ENTRY_MODE, 0x1030);
	sendCmdAndData(S6D0164_DISPLAY, 0x0000);
	sendCmdAndData(S6D0164_BLANK_PERIOD, 0x0808);
	sendCmdAndData(S6D0164_FRAME_CYCLE, 0x1100);
	sendCmdAndData(S6D0164_INTERFACE, 0x0000);

	sendCmdAndData(S6D0164_OSCILLATOR, 0x1401);
	sendCmdAndData(S6D0164_VCI, 0x0000);
	sendCmdAndData(S6D0164_GRAM_ADDRESS_SET_X, 0x0000);
	sendCmdAndData(S6D0164_GRAM_ADDRESS_SET_Y, 0x0000);

	sendCmdAndData(S6D0164_GAMMA_0, 0x0001);//Gamma
	sendCmdAndData(S6D0164_GAMMA_1, 0x020B);
	sendCmdAndData(S6D0164_GAMMA_2, 0x0805);
	sendCmdAndData(S6D0164_GAMMA_3, 0x0404);
	sendCmdAndData(S6D0164_GAMMA_4, 0x0C0C);
	sendCmdAndData(S6D0164_GAMMA_5, 0x000C);
	sendCmdAndData(S6D0164_GAMMA_6, 0x0101);
	sendCmdAndData(S6D0164_GAMMA_7, 0x0400);
	sendCmdAndData(S6D0164_GAMMA_8, 0x1108);
	sendCmdAndData(S6D0164_GAMMA_9, 0x050C);

	sendCmdAndData(S6D0164_HORIZ_END, TFT_MAX_X); 
	sendCmdAndData(S6D0164_HORIZ_START, 0x0000); 	
	sendCmdAndData(S6D0164_VERT_END, TFT_MAX_Y);
	sendCmdAndData(S6D0164_VERT_START, 0x0000); 
	
	sendCmdAndData(S6D0164_OSCILLATOR, 0x0B01);
	sendCmdAndData(S6D0164_DISPLAY, 0x0016);	
	sendCmdAndData(S6D0164_DISPLAY, 0x0017);

	switchCs(1);

	isOk = true;
}


void S6D0164::enable(const short on) {
	switchCs(0);
	if (on == 0) {
		sendCmdAndData(S6D0164_DISPLAY, 0x0013);
		isOk = false;
	} else {
		sendCmdAndData(S6D0164_DISPLAY, 0x0000);
		isOk = true;
	}
	switchCs(1);
	DelayManager::Delay(100);
}

void S6D0164::clear(const uint16_t color)
{
	fillScreen(TFT_MIN_X, TFT_MIN_Y, TFT_MAX_X, TFT_MAX_Y, color);
}


void S6D0164::setCol(const uint16_t StartCol, const uint16_t EndCol)
{
	
	sendCmdAndData(S6D0164_VERT_START, StartCol);
	sendCmdAndData(S6D0164_VERT_END, EndCol);
	sendCmdAndData(S6D0164_GRAM_ADDRESS_SET_Y, StartCol);
}

void S6D0164::setPage(const uint16_t StartPage, const uint16_t EndPage)
{
	sendCmdAndData(S6D0164_HORIZ_START, StartPage);
	sendCmdAndData(S6D0164_HORIZ_END, EndPage);
	sendCmdAndData(S6D0164_GRAM_ADDRESS_SET_X, StartPage);
}

void S6D0164::setFont(const unsigned char font[])
{
	this->font = font;
}


void S6D0164::setRotation(const uint8_t rotationMode)
{
	this->rotationMode = rotationMode;
}

void S6D0164::fillScreen(const uint16_t xstart, const uint16_t ystart, const uint16_t xstop, const uint16_t ystop, const uint16_t color)
{
	uint32_t pixels = (xstop - xstart + 1) * (ystop - ystart + 1);
	switchCs(0);
	setPage(xstart, xstop);
	setCol(ystart, ystop);
	sendCmd(S6D0164_GRAM_RW);

	while (pixels) {
		sendWord(color);
		pixels--;
	}
	switchCs(1);
}

void S6D0164::pixelDraw(uint16_t xpos, uint16_t ypos, uint16_t color)
{
	switchCs(0);
	if (rotationMode == PORTRAIT)
	{
		setPage(xpos, xpos);
		setCol(TFT_MAX_Y - ypos, TFT_MAX_Y - ypos);	
	}
	else if (rotationMode == LANDSCAPE)
	{
		setPage(TFT_MAX_X - ypos, TFT_MAX_X - ypos);
		setCol(TFT_MAX_Y - xpos, TFT_MAX_Y - xpos);		
	}
	sendCmd(S6D0164_GRAM_RW);
	sendWord(color);
	switchCs(1);
}

void S6D0164::bufferDraw(uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize, uint16_t* buf)
{
	switchCs(0);
	if (rotationMode == PORTRAIT)
	{
		setPage(x, x + xsize - 1);
		setCol(TFT_MAX_Y - y, TFT_MAX_Y - y + ysize - 1);	
	}
	else if (rotationMode == LANDSCAPE)
	{
		setPage(TFT_MAX_X - y - ysize - 1, TFT_MAX_X - y);
		setCol(TFT_MAX_Y - x - xsize - 1, TFT_MAX_Y - x);		
	}
	sendCmd(S6D0164_GRAM_RW);

	for (uint32_t l = 0; l < xsize * ysize; l++) {
		sendWord(buf[l]);
	}
	switchCs(1);
}

void S6D0164::lineDraw(uint16_t ypos, uint16_t* line,  uint32_t size)
{
	bufferDraw(0, ypos, size, 1, line);
}

void S6D0164::drawBorder(uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, uint16_t borderWidth, uint16_t color)
{
	const uint16_t bw = borderWidth - 1;
	if (rotationMode == PORTRAIT)
	{
		fillScreen(xpos, TFT_MAX_Y - ypos - height, xpos + bw, TFT_MAX_Y - ypos, color);
		fillScreen(xpos + bw, TFT_MAX_Y - ypos - bw, xpos + width, TFT_MAX_Y - ypos, color);
		fillScreen(xpos + width - bw, TFT_MAX_Y - ypos - height, xpos + width, TFT_MAX_Y - ypos - bw, color);
		fillScreen(xpos + bw, TFT_MAX_Y - ypos - height, xpos + width - bw, TFT_MAX_Y - ypos - height + bw, color);
	}
	else
	{
		fillScreen(TFT_MAX_X - ypos - height, TFT_MAX_Y - xpos - bw, TFT_MAX_X - ypos, TFT_MAX_Y - xpos, color);
		fillScreen(TFT_MAX_X - ypos - bw, TFT_MAX_Y - xpos - width, TFT_MAX_X - ypos, TFT_MAX_Y - xpos, color);
		fillScreen(TFT_MAX_X - ypos - height, TFT_MAX_Y - xpos - bw - width, TFT_MAX_X - ypos, TFT_MAX_Y - xpos - width, color);
		fillScreen(TFT_MAX_X - ypos - height - bw, TFT_MAX_Y - xpos - bw - width, TFT_MAX_X - ypos - height, TFT_MAX_Y - xpos, color);		
	}
}

void S6D0164::putChar(uint16_t x, uint16_t y, uint8_t chr, uint16_t charColor, uint16_t bkgColor) {
	uint8_t i, j;
	
	uint8_t f_width = font[0];	
	uint8_t f_height = font[1];
	uint16_t f_bytes = (f_width * f_height / 8);
	
	uint16_t t = 0;
	uint16_t charbuf[(f_width + 1) * f_height];
	
	//fill charbuf
	if (rotationMode == LANDSCAPE)
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
	//while (this->isDataSending); //wait until all data wasn't sended
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
	switchToReadMode();
	sendCmd(S6D0164_SYSTEM);
	const uint16_t data = readWord();
	switchToWriteMode();
	return data;
}


uint16_t S6D0164::readWord()
{
	switchCs(0);
	switchRd(0);
	switchRd(1);
	uint16_t data = dataPort->IDR << 8;
	switchRd(0);
	switchRd(1);
	data |= dataPort->IDR & 0xFF;
	switchCs(1);
	return data;
}

void S6D0164::setBus(const uint16_t val)
{
	dataPort->ODR &= 0xFF00;
	dataPort->ODR |= val >> 8;
	switchWr(0);
	switchWr(1);
	dataPort->ODR &= 0xFF00;
	dataPort->ODR |= val & 0x00FF;
	switchWr(0);
	switchWr(1);
}

void S6D0164::sendCmd(uint8_t cmd)
{
	switchRs(0);
	setBus(cmd);
}

void S6D0164::sendWord(uint16_t data)
{
	switchRs(1);
	setBus(data);
}


void S6D0164::sendCmdAndData(uint8_t cmd, uint16_t data)
{
	sendCmd(cmd);
	sendWord(data);
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

void S6D0164::switchRd(const short BitVal)
{
	if (BitVal != Bit_RESET) {
		controlPort->BSRR = rdPin;
	}
	else {
		controlPort->BRR = rdPin;
	}
}

void S6D0164::switchWr(const short BitVal)
{
	if (BitVal != Bit_RESET) {
		controlPort->BSRR = wrPin;
	}
	else {
		controlPort->BRR = wrPin;
	}
}

void S6D0164::switchRs(const short BitVal)
{
	if (BitVal != Bit_RESET) {
		controlPort->BSRR = rsPin;
	}
	else {
		controlPort->BRR = rsPin;
	}
}

void S6D0164::switchCs(const short BitVal)
{
	if (BitVal != Bit_RESET) {
		controlPort->BSRR = csPin;
	} else {
		controlPort->BRR = csPin;
	}
}

void S6D0164::switchReset(const short BitVal)
{
	if (BitVal != Bit_RESET) {
		controlPort->BSRR = resetPin;
	} else {
		controlPort->BRR = resetPin;
	}
}
