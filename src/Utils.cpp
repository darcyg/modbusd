/*
 * Utils.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#include "Utils.h"

#include <stdio.h>


#include <execinfo.h>
#include <stdlib.h>

std::string itoa(int i) {
    std::ostringstream oss;
    oss << i;
    return oss.str();
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, elems);
        return elems;
}

std::string choppa(const std::string &t, const std::string &ws)
{
    std::string str = t;
    size_t found;
    found = str.find_last_not_of(ws);
    if (found != std::string::npos)
    	str.erase(found+1);
    else
    	str.clear();            // str is all whitespace

    str.erase(0, str.find_first_not_of(ws));

    return str;
}

/*
 * Returns time in format YYYY-MM-DD HH:MM:SS
 */
std::string TimeToString(time_t time)
{
	char buffer[32];
	struct tm* tm = localtime(&time);
	snprintf(buffer,32,"%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	return std::string(buffer);
}

unsigned short  Crc16(unsigned short crcInit, unsigned char byte) {
	  unsigned short lb = byte;
	  int i;
	  crcInit ^= lb << 8;
	  for (i=0; i < 8; i++)
	  {
		  if ( (crcInit & (1 << 15)) != 0)
			  crcInit = (crcInit << 1) ^ 0x8005;
		  else
			  crcInit  = crcInit << 1;
	  }
	  return  crcInit;
}


unsigned short  Crc16(unsigned short crcInit, unsigned char buffer[], int size) {
	for (int i = 0; i < size; i++)
		crcInit = Crc16(crcInit, buffer[i]);
	return crcInit;
}

#ifdef __DEBUG__
void _do_backtrace()
{
    int j, nptrs;
	#define SIZE 100
    void *buffer[100];
    char **strings;

    nptrs = backtrace(buffer, SIZE);
    printf("backtrace() returned %d addresses\n", nptrs);

    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
       would produce similar output to the following: */

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
    }

    for (j = 0; j < nptrs; j++)
        printf("%s\n", strings[j]);

    free(strings);
}
#endif
