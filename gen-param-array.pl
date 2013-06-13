#!/usr/bin/perl 
#use strict;
#use warnings;
#use 5.010;

my $i=1;


open (DATA, 'parameters.csv') or die("Cannot open parameters.csv");

open (LUK_F, '>params_lukoil.h') or die("Cannot open settings_lukoil.h");
open (TNK_F, '>params_tnkbp.h') or die("Cannot open settings_tnkbp.h");

print LUK_F "\#ifndef _PARAMS_LUKOIL_H_\n";
print LUK_F "\#define _PARAMS_LUKOIL_H_\n";

print TNK_F "\#ifndef _PARAMS_TNKBP_H_\n";
print TNK_F "\#define _PARAMS_TNKBP_H_\n";


print LUK_F "static data_parameter_t lukoil_params[] = \{\n";
print TNK_F "static data_parameter_t tnkbp_params[] = \{\n";


while(<DATA>) {
    chomp;
    if (/^([0-9]*),([\"]+.*[\"]+),(.*),(.*),(.*),(.*),(.+.*),(0|0x[0-9A-Fa-f]+[0-9A-Fa-f]*),(0|0x[0-9A-Fa-f]+[0-9A-Fa-f]*)$/) {
		if($7 ne "0") {
			print LUK_F "\t\{$1,0\.0,$7,1\},\n"
		}
		if($8 ne "0") {
			print TNK_F "\t\{$1,0\.0,$8,1\},\n"
		}
    } else{
	if (/^([0-9]*),(.*),(.+.*),(.*),(.*),(.*),(0|0x[0-9A-Fa-f]+[0-9A-Fa-f]*),(0|0x[0-9A-Fa-f]+[0-9A-Fa-f]*)$/) {
		if($7 ne "0") {
			print LUK_F "\t\{$1,0\.0,$7,1\},\n"
		}
		if($8 ne "0") {
			print TNK_F "\t\{$1,0\.0,$8,1\},\n"
		}
	}
	}
	$i = $i + 1;
}

print LUK_F "\};\n";
print TNK_F "\};\n";

print LUK_F "\#define NUMBER_OF_PARAMS_LUKOIL (sizeof(lukoil_params) / sizeof(data_parameter_t))\n";
print TNK_F "\#define NUMBER_OF_PARAMS_TNKBP (sizeof(tnkbp_params) / sizeof(data_parameter_t))\n";

print LUK_F "\#endif\n";
print TNK_F "\#endif\n";


close(LUK_F);
close(TNK_F);
