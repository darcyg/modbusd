#!/usr/bin/perl 
#use strict;
#use warnings;
#use 5.010;


open (DATA, 'settings.csv') or die("Cannot open settings.csv");

print "static setting_t all_Settings[] = \{\n";

my $offset = 0;

while(<DATA>) {
    chomp;
# name, suffix, id, set_size, get_size, is_visible, modbus
    if (/^([\"]+.*[\"]+),(.*),([0-9]+[0-9]*),([0-9]+[0-9]*),([0-9]+[0-9]*),([0-1]+),(0x[0-9A-F]+[0-9A-F]*)/) {
	print "\t\{$1,0.0,$7,1\},\n";
    } else {
	if(/^(.*),(.*),([0-9]+[0-9]*),([0-9]+[0-9]*),([0-9]+[0-9]*),([0-1]+),(0x[0-9A-F]+[0-9A-F]*)/) {
		print "\t\{\"$1\",0.0, $7,1\},\n";
	}
    }
}

print "};\n"
