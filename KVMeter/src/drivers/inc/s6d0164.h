
#ifndef __S6D0164_H__
#define __S6D0164_H__

#include "stm32f10x.h"

//TFT resolution 176*220
#define TFT_MIN_X	0
#define TFT_MIN_Y	0
#define TFT_MAX_X	175
#define TFT_MAX_Y	219


typedef enum
{ 
	PORTRAIT     = 0,
	LANDSCAPE,
	UPSIDE_DOWN_PORTRAIT,
	UPSIDE_DOWN_LANDSCAPE,
} ROTATION_MODE;


class S6D0164 {
public:
	S6D0164();
	~S6D0164();
	void setupHw(GPIO_TypeDef* dataPort, GPIO_TypeDef* controlPort, uint16_t rdPin, uint16_t wrPin, uint16_t rsPin, uint16_t csPin, uint16_t resetPin);
	void init(void);
	void enable(short on);
	uint32_t readID();
	void clear(uint16_t color);
	void setWindow(uint16_t startX, uint16_t startY, uint16_t stopX, uint16_t stopY);
	void pixelDraw(uint16_t xpos, uint16_t ypos, uint16_t color);
	void lineDraw(uint16_t ypos, uint16_t* line, uint32_t size);
	void fillScreen(uint16_t xstart, uint16_t ystart, uint16_t xstop, uint16_t ystop, uint16_t color);
	void putChar(uint16_t x, uint16_t y, uint8_t chr, uint16_t charColor, uint16_t bkgColor);
	void printf(uint16_t x, uint16_t y, const char *format, ...);
	void printf(uint16_t x, uint16_t y, uint16_t charColor, uint16_t bkgColor, const char *format, ...);
	void bufferDraw(uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize, uint16_t* buf);
	void drawBorder(uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, uint16_t borderWidth, uint16_t color);
	uint8_t IsDataSending();
	void resetIsDataSending();
	void setColor(uint16_t color, uint16_t bgColor);
	void setFont(const unsigned char font[]);
	void setRotation(uint8_t rotationMode);
	uint16_t RGB888ToRGB565(uint8_t r, uint8_t g, uint8_t b);
	bool isReady(void);
	
protected:
	GPIO_TypeDef* dataPort;
	GPIO_TypeDef* controlPort;
	uint16_t rdPin;
	uint16_t wrPin;
	uint16_t rsPin;
	uint16_t csPin;
	uint16_t resetPin;

	uint8_t isDataSending;
	uint8_t rotationMode;
	uint16_t color;
	uint16_t bgColor;
	const unsigned char *font;
	bool isOk;

private:
	void setCol(uint16_t StartCol, uint16_t EndCol);
	void setPage(uint16_t StartPage, uint16_t EndPage);
	void setBus8bit(uint8_t val);
	void setBus(uint16_t val);
	void sendCmd(uint8_t index);
	void sendWord(uint16_t data);
	void sendCmdAndData(uint8_t cmd, uint16_t data);
	void putString(const char str[], uint16_t x, uint16_t y, uint16_t charColor, uint16_t bkgColor);
	void switchRd(short BitVal);
	void switchWr(short BitVal);
	void switchRs(short BitVal);
	void switchCs(short BitVal);
	void switchReset(short BitVal);
	void switchToReadMode();
	void switchToWriteMode();
	
	uint16_t readWord();
};

#define RED			0xf800
#define PURPLE		0xf310
#define GREEN		0x07e0
#define BLUE		0x001f
#define BLACK		0x0000
#define YELLOW		0xffe0
#define ORANGE		0xfc00
#define WHITE		0xffff
#define CYAN		0x07ff
#define BRIGHT_RED	0xf810
#define GRAY1		0x8410
#define GRAY2		0x4208

#define S6D0164_SYSTEM                  0x00
#define S6D0164_OUTPUT                  0x01
#define S6D0164_WAVEFORM                0x02
#define S6D0164_ENTRY_MODE              0x03
#define S6D0164_DISPLAY                 0x07
#define S6D0164_BLANK_PERIOD            0x08
#define S6D0164_FRAME_CYCLE             0x0B
#define S6D0164_INTERFACE               0x0C
#define S6D0164_OSCILLATOR              0x0F
#define S6D0164_POWER_1                 0x10
#define S6D0164_POWER_2                 0x11
#define S6D0164_POWER_3                 0x12
#define S6D0164_POWER_4                 0x13
#define S6D0164_POWER_5                 0x14
#define S6D0164_VCI                     0x15
#define S6D0164_GRAM_ADDRESS_SET_X      0x20
#define S6D0164_GRAM_ADDRESS_SET_Y      0x21
#define S6D0164_GRAM_RW                 0x22
#define S6D0164_SOFT_RESET              0x28
#define S6D0164_VERTICAL_SCROLL_1A      0x31
#define S6D0164_VERTICAL_SCROLL_1B      0x32
#define S6D0164_VERTICAL_SCROLL_2       0x33
#define S6D0164_PARTIAL_1               0x34
#define S6D0164_PARTIAL_2               0x35
#define S6D0164_HORIZ_END               0x36
#define S6D0164_HORIZ_START             0x37
#define S6D0164_VERT_END                0x38
#define S6D0164_VERT_START              0x39
#define S6D0164_GAMMA_0                 0x50
#define S6D0164_GAMMA_1                 0x51
#define S6D0164_GAMMA_2                 0x52
#define S6D0164_GAMMA_3                 0x53
#define S6D0164_GAMMA_4                 0x54
#define S6D0164_GAMMA_5                 0x55
#define S6D0164_GAMMA_6                 0x56
#define S6D0164_GAMMA_7                 0x57
#define S6D0164_GAMMA_8                 0x58
#define S6D0164_GAMMA_9                 0x59
#define S6D0164_TEST_KEY                0x80
#define S6D0164_MTP_CTL                 0x81
#define S6D0164_MTP_DATA_WR             0x82
#define S6D0164_PANEL_ID                0x93


#endif //__S6D0164_H__
