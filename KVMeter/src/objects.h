#pragma once
#ifndef __OBJECTS_H_
#define __OBJECTS_H_

#include "s6d0164.h"

template<class ForwardIt>
	ForwardIt max_element(ForwardIt first, ForwardIt last)
	{
		if (first == last) {
			return last;
		}
		ForwardIt largest = first;
		++first;
		for (; first != last; ++first) {
			if (*largest < *first) {
				largest = first;
			}
		}
		return largest;
	}


typedef enum
{ 
	VOLTAGE = 0,
	TEMPERATURE
} ADC_MODE;


extern S6D0164 display;

extern float voltage;
extern float cpuTemp;

extern void readADCValue(void);

#endif //__OBJECTS_H_