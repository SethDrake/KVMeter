
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
	
	font = Consolas8x15;
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
	sendCmdAndData(S6D0164_ENTRY_MODE, 0x1010);
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
	if (this->rotationMode == PORTRAIT)
	{
		fillScreen(TFT_MIN_X, TFT_MIN_Y, TFT_MAX_X, TFT_MAX_Y, color);
	}
	else if (this->rotationMode == LANDSCAPE)
	{
		fillScreen(TFT_MIN_Y, TFT_MIN_X, TFT_MAX_Y, TFT_MAX_X, color);
	}
}

void S6D0164::setHorizontal(const uint16_t startX, const uint16_t stopX)
{
	sendCmdAndData(S6D0164_HORIZ_START, startX);
	sendCmdAndData(S6D0164_HORIZ_END, stopX);
	sendCmdAndData(S6D0164_GRAM_ADDRESS_SET_X, startX);
}

void S6D0164::setVertical(const uint16_t startY, const uint16_t stopY)
{
	
	sendCmdAndData(S6D0164_VERT_START, startY);
	sendCmdAndData(S6D0164_VERT_END, stopY);
	sendCmdAndData(S6D0164_GRAM_ADDRESS_SET_Y, (this->rotationMode == PORTRAIT) ? startY : stopY);
}

void S6D0164::setFont(const unsigned char font[])
{
	this->font = font;
}

void S6D0164::setRotation(const uint8_t rotationMode)
{
	//0 0 0 1 0 0 0 0 0 0 ID1 ID0 AM 0 0 0
	//default - 0x1030 - 0001000000110000
	this->rotationMode = rotationMode;
	switchCs(0);
	if (this->rotationMode == PORTRAIT) // 0x1010 - 0001000000010000
	{
		sendCmdAndData(S6D0164_ENTRY_MODE, 0x1030);	
	}
	else if (this->rotationMode == LANDSCAPE) // 0x1018 - 0001000000011000
	{
		sendCmdAndData(S6D0164_ENTRY_MODE, 0x1018);	
	}
	switchCs(1);
}

void S6D0164::fillScreen(const uint16_t xstart, const uint16_t ystart, const uint16_t xstop, const uint16_t ystop, const uint16_t color)
{
	uint32_t pixels = (xstop - xstart + 1) * (ystop - ystart + 1);
	switchCs(0);
	if (this->rotationMode == PORTRAIT)
	{
		setHorizontal(xstart, xstop);
		setVertical(TFT_MAX_Y - ystop, TFT_MAX_Y - ystart);
	}
	else if (this->rotationMode == LANDSCAPE)
	{
		setHorizontal(TFT_MAX_X - ystop, TFT_MAX_X - ystart);
		setVertical(TFT_MAX_Y - xstop, TFT_MAX_Y - xstart);	
	}
	sendCmd(S6D0164_GRAM_RW);

	if (color == BLACK || color == WHITE)
	{
		sendMulti(color, pixels);
	}
	else
	{
		while (pixels) {
			sendWord(color);
			pixels--;
		}	
	}
	switchCs(1);
}

void S6D0164::pixelDraw(const uint16_t xpos, const uint16_t ypos, const uint16_t color)
{
	switchCs(0);
	if (this->rotationMode == PORTRAIT)
	{
		setHorizontal(xpos, xpos);
		setVertical(TFT_MAX_Y - ypos, TFT_MAX_Y - ypos);
	}
	else if (this->rotationMode == LANDSCAPE)
	{
		setHorizontal(TFT_MAX_X - ypos, TFT_MAX_X - ypos);
		setVertical(TFT_MAX_Y - xpos, TFT_MAX_Y - xpos);	
	}
	sendCmd(S6D0164_GRAM_RW);
	sendWord(color);
	switchCs(1);
}

void S6D0164::bufferDraw(const uint16_t x, const uint16_t y, const uint16_t xsize, const uint16_t ysize, uint16_t* buf)
{
	switchCs(0);
	if (this->rotationMode == PORTRAIT)
	{
		setHorizontal(x, x + xsize - 1);
		setVertical(TFT_MAX_Y - y - (ysize - 1), TFT_MAX_Y - y);
	}
	else if (this->rotationMode == LANDSCAPE)
	{
		setHorizontal((TFT_MAX_X - y) - (ysize - 1), TFT_MAX_X - y);
		setVertical((TFT_MAX_Y - x) - (xsize - 1), TFT_MAX_Y - x);
	}
	sendCmd(S6D0164_GRAM_RW);

	for (uint32_t l = 0; l < xsize * ysize; l++) {
		sendWord(buf[l]);
	}
	switchCs(1);
}

void S6D0164::lineDraw(const uint16_t ypos, uint16_t* line, const uint32_t size)
{
	bufferDraw(0, ypos, size, 1, line);
}

void S6D0164::drawBorder(const uint16_t xpos, const uint16_t ypos, const uint16_t width, const uint16_t height, const uint16_t borderWidth, const uint16_t color)
{
	const uint16_t bw = borderWidth - 1;
	fillScreen(xpos, ypos, xpos + bw, ypos + height, color);
	fillScreen(xpos + bw, ypos + height - bw, xpos + width, ypos + height, color);
	fillScreen(xpos + width - bw, ypos, xpos + width, ypos + height - bw, color);
	fillScreen(xpos + bw, ypos, xpos + width - bw, ypos + bw, color);
}

void S6D0164::putChar(const uint16_t x, const uint16_t y, const uint8_t chr, const uint16_t charColor, const uint16_t bkgColor) {
	uint16_t y1 = y;
	const uint8_t f_width = font[0];	
	const uint8_t f_height = font[1];
	const uint8_t firstCharCode = font[2];

	uint16_t totalPixels = f_width * f_height;
	uint16_t bytesPerSymbol = totalPixels / 8;
	if (totalPixels % 8 != 0)
	{
		bytesPerSymbol++;
	}
	
	uint16_t t = 0;
	uint16_t charbuf[totalPixels];
	
	//fill charbuf
	for (uint8_t i = 0; i < f_height; i++)
	{
		for (uint8_t j = 0; j < f_width; j++) {
			const uint16_t bitNumberInChar = (i * f_width) + j;
			const uint16_t byteNumberInChar = (bitNumberInChar / 8);
			const uint8_t bitNumberInByte = 7 - (bitNumberInChar - (byteNumberInChar * 8));
			const uint8_t glyphByte = font[3 + (chr - firstCharCode) * bytesPerSymbol + byteNumberInChar];
			const uint8_t mask = 1 << bitNumberInByte;
			if (glyphByte & mask) {
				charbuf[t++] = charColor;
			}
			else 
			{
				charbuf[t++] = bkgColor;
			}
		}
	}
	y1 -= 3;
	
	bufferDraw(x, y1, f_width, f_height, charbuf);
}

void S6D0164::putString(const char str[], const uint16_t x, const uint16_t y, const uint16_t charColor, const uint16_t bkgColor) {
	uint16_t x1 = x; 
	while (*str != 0) {
		putChar(x1, y, *str, charColor, bkgColor);
		x1 += font[0]; //increment to font width
		str++;
	}
}

void S6D0164::printf(const uint16_t x, const uint16_t y, uint16_t const charColor, uint16_t const bkgColor, const char *format, ...) {
	char buf[40];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	putString(buf, x, y, charColor, bkgColor);
}

void S6D0164::printf(const uint16_t x, const uint16_t y, const char *format, ...)
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
	tickWr();
	dataPort->ODR &= 0xFF00;
	dataPort->ODR |= val & 0x00FF;
	tickWr();
}

void S6D0164::sendCmd(const uint8_t cmd)
{
	switchRs(0);
	setBus(cmd);
}

void S6D0164::sendWord(const uint16_t data)
{
	switchRs(1);
	setBus(data);
}

void S6D0164::sendMulti(const uint16_t data, uint32_t count)
{
	switchRs(1);
	setBus(data);
	count--;
	while (count)
	{
		tickWr();
		tickWr();
		count--;
	}
}

void S6D0164::sendCmdAndData(const uint8_t cmd, const uint16_t data)
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


void S6D0164::setColor(const uint16_t color, const uint16_t bgColor)
{
	this->color = color;
	this->bgColor = bgColor;
}

uint16_t S6D0164::RGB888ToRGB565(const uint8_t r, const uint8_t g, const uint8_t b)
{
	const uint16_t r5 = (uint16_t)((r * 249 + 1014) >> 11);
	const uint16_t g6 = (uint16_t)((g * 253 + 505) >> 10);
	const uint16_t b5 = (uint16_t)((b * 249 + 1014) >> 11);
	return (uint16_t)(r5 << 11 | g6 << 5 | b5);
}


void S6D0164::tickWr()
{
	switchWr(0);
	switchWr(1);
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
