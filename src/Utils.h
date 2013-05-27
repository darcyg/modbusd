/*
 * Utils.h
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <time.h>
#include <string>
#include <sstream>
#include <vector>

//need to swap byte if running on LE target (e.g. x86)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static inline unsigned short swap16(unsigned short x)
{
    return ( (x << 8) | (x >> 8) );
}
#define MSB(x) (((x) >> 8) & 0xFF)
#define LSB(x) ((x) & 0xFF)
#else
#define swap16(x) (x)
#define LSB(x) (((x) >> 8) & 0xFF)
#define MSB(x) ((x) & 0xFF)
#endif

#define _STRUCT_OFFSET(struct_type, member)	\
    ((long) ((unsigned char*) &((struct_type*) 0)->member))

std::string itoa(int i);
std::vector<std::string> split(const std::string &s, char delim);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::string choppa(const std::string &t, const std::string &ws);
std::string TimeToString(time_t time);
unsigned short Crc16(unsigned short crcInit, unsigned char byte);
unsigned short Crc16(unsigned short crcInit, unsigned char buffer[], int size);

//#ifdef __DEBUG__
void _do_backtrace();
//#else
//extern void _do_backtrace() {}
//#endif

#endif /* UTILS_H_ */
