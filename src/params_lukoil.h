#ifndef _PARAMS_LUKOIL_H_
#define _PARAMS_LUKOIL_H_
static data_parameter_t lukoil_params[] = {
	{0,1050100090,0.0,0xFF,1},
	{0,1050101000,0.0,0x0FD,1},
	{0,1050111070,0.0,0x10F,1},
	{0,1050100060,0.0,0x112,1},
	{0,1050100080,0.0,0x111,1},
	{0,1050110010,0.0,0x10E,1},
	{0,1050100050,0.0,0x109,1},
	{0,1050112010,0.0,0x117,1},
	{0,1050112020,0.0,0x116,1},
	{0,1050117000,0.0,0x101,1},
	{0,1050201000,0.0,0x903,1},
	{0,1050102010,0.0,0x900,1},
	{0,1080105030,0.0,0x11E,1},
	{0,1080104010,0.0,0x11B,1},
	{0,1080104020,0.0,0x11A,1},
	{0,1080100009,0.0,0x11C,1},
	{0,1080100010,0.0,0x11D,1},
	//FIXME: manually added. param is missing in spreadsheet
	{0,1050100100,0.0,0xFE,1},
};
#define NUMBER_OF_PARAMS_LUKOIL (sizeof(lukoil_params) / sizeof(data_parameter_t))
#endif
