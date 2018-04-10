#pragma once

#include "Definitions.h"

class Alignment
{
public:
	Alignment() {};
	Alignment(char* chrom, u_int c_pos) {
		chromosome = chrom;
		pos = c_pos;
	};

	~Alignment() {};

	/* c-string ukonceny nulou */
	char* chromosome;
	/* pozice zacatku */
	u_int pos;

	/* Sekce: Martin Kubala*/
	char* cigar_r1;
	char* cigar_r2;
	/*
		Mapping quality
		CIGAR string
	*/
};
