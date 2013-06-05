#!/usr/bin/perl 
#use strict;
#use warnings;
use 5.010;


open (DATA, 'settings.csv') or die("Cannot open settings.csv");
open (LUK_F, '>settings_lukoil.h') or die("Cannot open settings_lukoil.h");
open (TNK_F, '>settings_tnkbp.h') or die("Cannot open settings_tnkbp.h");

print LUK_F "\#ifndef _SETTINGS_LUKOIL_H_\n";
print LUK_F "\#define _SETTINGS_LUKOIL_H_\n";

print TNK_F "\#ifndef _SETTINGS_TNKBP_H_\n";
print TNK_F "\#define _SETTINGS_TNKBP_H_\n";


print LUK_F "static setting_t lukoil_Settings[] = \{\n";
print TNK_F "static setting_t tnkbp_Settings[] = \{\n";

my $offset = 0;

while(<DATA>) {
    chomp;
# name, suffix, id, set_size, get_size, is_visible, modbus, lukoil modbus, tnk modbus
    if (/^([\"]+.*[\"]+),.*,[0-9]+[0-9]*,[0-9]+[0-9]*,[0-9]+[0-9]*,[0-1]+,(0|0x[0-9A-Fa-f]+[0-9A-Fa-f]*),(0|0x[0-9A-Fa-f]+[0-9A-Fa-f]*),(0|0x[0-9A-Fa-f]+[0-9A-Fa-f]*)$/) {
#		print "$2 $3 $4\n";
		if($3 ne "0") {
			print LUK_F "\t\{$1,0.0,$3,1\},\n";
		}
		if($4 ne "0") {
			print TNK_F "\t\{$1,0.0,$4,1\},\n";
		}
    } else {
		if(/^(.*),.*,[0-9]+[0-9]*,[0-9]+[0-9]*,[0-9]+[0-9]*,[0-1]+,(0|0x[0-9A-Fa-f]+[0-9A-Fa-f]*),(0|0x[0-9A-Fa-f]+[0-9A-Fa-f]*),(0|0x[0-9A-Fa-f]+[0-9A-Fa-f]*)$/) {
#			print "$2 $3 $4\n";
			if($3 ne "0") {
				print LUK_F "\t\{\"$1\",0.0, $3,1\},\n";
			}
			if($4 ne "0") {
				print TNK_F "\t\{\"$1\",0.0, $4,1\},\n";
			}
		} else {
			print "$_\n";
		}
    }
}

#print "\};\n";

print LUK_F "\};\n";
print TNK_F "\};\n";

print LUK_F "\#define NUMBER_OF_SETTINGS_LUKOIL (sizeof(lukoil_Settings) / sizeof(setting_t))\n";
print TNK_F "\#define NUMBER_OF_SETTINGS_TNKBP (sizeof(tnkbp_Settings) / sizeof(setting_t))\n";

print LUK_F "\#endif\n";
print TNK_F "\#endif\n";


close(LUK_F);
close(TNK_F);

