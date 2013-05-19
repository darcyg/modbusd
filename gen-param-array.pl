#!/usr/bin/perl 
#use strict;
#use warnings;
#use 5.010;

my $i=1;


open (DATA, 'parameters.csv') or die("Cannot open parameters.csv");

print "static data_parameter_t parameters[] = \{\n";


while(<DATA>) {
    chomp;
    if (/^([0-9]*),([\"]+.*[\"]+),(.*),(.*),(.*),(.*),(.+.*)/) {
		print "\t\{$1,0\.0,$6,1\},\n"
    } else{
	if (/^([0-9]*),(.*),(.+.*),(.*),(.*),(.*)/) {
		print "\t\{$1,0\.0,$6,1\},\n"
	}
	}
	$i = $i + 1;
}

print "};\n"
