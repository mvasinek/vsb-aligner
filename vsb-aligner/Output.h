﻿#pragma once
#include <fstream>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>

#include "Alignment.h"
#include "Alignment_output.h"
#include "Definitions.h"
#include "Genome.h"
#include "GenomeRegion.h"
#include "List.h"
#include "Read.h"

using namespace std;

class Output {
private:
	ofstream ofs;
	char* sam_file = NULL;
	int score_MAX = 0;
	List<Alignment_output>* out_alignments = new List<Alignment_output>();
public:
	Output(char* output_file){
		this->sam_file = output_file;
		int file_check = remove(output_file); //If file exists, return code is 0. We have it here to be sure, that file will be deleted.
	}

	/*
	Print the head into the txt file
	*/
	void print_head(List<GenomeRegion>* gen){
		ListIterator<GenomeRegion> iterator(gen->First());
		this->ofs.open(this->sam_file, ios::out);
		while (iterator.Current() != NULL) {
			GenomeRegion* temp = iterator.Current()->Value();
			this->ofs << "@SQ SN:" << temp->chromosome_id << "\tLN:" << temp->bases_number << "\n";
			iterator.Next();
		}
		this->ofs.close();
	}

	/*
	Print program info into the txt file
	*/
	void print_program_info(char* ID, char* PN, char* VN, int T, char* r1, char* r2, char* genome) {
		this->ofs.open(this->sam_file, ios::out | ios::app);
		this->ofs << "@PG ID:" << ID << "\tPN:" << PN << "\tVN:" << VN << "\t-T " << T << "\t" << r1 << "\t" << r2 << "\t" << genome << "\n";
		this->ofs.close();
	}


	/*
	Preparing List of alignments output format for better handling 
	*/
	void output_prepare(List<Read>* reads, int T) {
		u_int id = 0;
		ListIterator<Read> iterator(reads->First());
		while (iterator.Current() != NULL) {
			Read* r = iterator.Current()->Value();
			ListIterator<Alignment> a_iterator(r->alignments->First());
			if (r->paired_read != NULL) {
				int temp = 0; // 
				Read* r2 = r->paired_read;
				ListIterator<Alignment> b_iterator(r2->alignments->First());
				while (a_iterator.Current() != NULL) {
					Alignment* a = a_iterator.Current()->Value();
					if ((a->score > T) && (a->available == true)) {
						char* RNEXT;
						u_int PNEXT = 0;
						int TLEN = 0;
						/*
						RNEXT calculating part
						*/
						string x1 = r->name;
						string x2 = r->paired_read->name;
						if (x1 == x2) {
							RNEXT = "=";
						}
						else {
							RNEXT = "*";
						}
						/*
						PNEXT, TLEN calculating part
						*/
						if (b_iterator.Current() != NULL) {
							Alignment* b = b_iterator.Current()->Value();
							PNEXT = b->pos;
							if (b->pos >= a->pos) {
								temp = b->pos - a->pos;
								temp += b->cigar_length;
							}
							else {
								temp = a->pos - b->pos;
								temp += a->cigar_length;
							}
							TLEN = temp + 1;
							b_iterator.Next();
						}
						else {
							PNEXT = 0;
							TLEN = 0;
						}
						temp = 0;
						/*
						Store alignment into output list
						*/
						this->out_alignments->Append(new Alignment_output(id, r->name, a->FLAG, a->chromosome, a->pos, a->MAPQ, a->cigar, RNEXT, PNEXT, TLEN, r->sequence, r->quality, a->score));
						id++;
					}
					a_iterator.Next();
				}

				a_iterator = r->alignments->First();
				b_iterator = r2->alignments->First();
				while (b_iterator.Current() != NULL) {
					Alignment* b = b_iterator.Current()->Value();
					if ((b->score > T) && (b->available == true)) {
						char* RNEXT;
						u_int PNEXT = 0;
						int TLEN = 0;
						/*
						RNEXT calculating part
						*/
						string x1 = r2->name;
						string x2 = r2->paired_read->name;
						if (x1 == x2) {
							RNEXT = "=";
						}
						else {
							RNEXT = "*";
						}
						/*
						PNEXT, TLEN calculating part
						*/
						if (a_iterator.Current() != NULL) {
							Alignment* a = a_iterator.Current()->Value();
							PNEXT = a->pos;
							if (a->pos >= b->pos) {
								temp = a->pos - b->pos;
								temp += a->cigar_length;

							}
							else {
								temp = b->pos - a->pos;
								temp += b->cigar_length;
							}
							TLEN = -temp-1;
							a_iterator.Next();
						}
						else {
							PNEXT = 0;
							TLEN = 0;
						}
						temp = 0;
						/*
						Store alignment into output list
						*/
						this->out_alignments->Append(new Alignment_output(id, r2->name, b->FLAG, b->chromosome, b->pos, b->MAPQ, b->cigar, RNEXT, PNEXT, TLEN, r2->sequence, r2->quality, b->score));
						id++;
					}
					b_iterator.Next();
				}

			}

			iterator.Next();
		}
	}

	/*
	Filter the alignments by the highes score of the alingments.
	*/
	void output_top_score_filtering() {
		//Zde bude protrizeni vyustupu kde bude mozne vypsat pouze alignmenty s nejvyssim score, pripadne alignmenty ktera budou mit spolecne nejvyssi score.
		//Vector bude pro alignmenty jenž jsou si rovny, bude se ukladat id prvku v listu a pokud bude prvek vyšší, tak se id použije k zneplatnění těchto alignmentu v samostatném cyklu dokud nebude vektor prázdný, takže while a následně bude porovnávaní nejvyššího alignmentu pokračovat.
		ListIterator<Alignment_output> iterator(out_alignments->First());
		while (iterator.Current() != NULL) {
			Alignment_output* out = iterator.Current()->Value();
			score_MAX = out->score;
			ListIterator<Alignment_output> iterator2(out_alignments->First());
			while (iterator2.Current() != NULL) {
				Alignment_output* out2 = iterator2.Current()->Value();
				if (out2->available == true) {
					string x1 = out->SEQ;
					string x2 = out2->SEQ;
					if (x1 == x2) {
						if (out2->score < this->score_MAX) { //If score is lesser then MAX?score, set available to false
							out2->available = false;
						}
						else if (out2->score == this->score_MAX) {
							//Nothing will happen
							//Add duplicity
						}
						else {
							this->score_MAX = out2->score;
							out->available = false;
							//Remove duplicity list, add id to other list for disavailable list
						}
					}
				}
				iterator2.Next();
			}
			iterator.Next();
		}
		this->ofs.close();
		
	}
	
	/*
	Print alignments data into the file
	*/
	void print_output_data() {
		this->ofs.open(this->sam_file, ios::out | ios::app);
		ListIterator<Alignment_output> iterator(out_alignments->First());
		while (iterator.Current() != NULL) {
			Alignment_output* out = iterator.Current()->Value();
			if (out->available == true) {
				this->ofs << out->QNAME << "\t";
				this->ofs << out->FLAG << "\t";
				this->ofs << out->RNAME << "\t";
				this->ofs << out->POS << "\t";
				this->ofs << out->MAPQ << "\t";
				this->ofs << out->CIGAR << "\t";
				this->ofs << out->RNEXT << "\t";
				this->ofs << out->PNEXT << "\t";
				this->ofs << out->TLEN << "\t";
				this->ofs << out->SEQ << "\t";
				this->ofs << out->QUAL << "\t";
				this->ofs << "\n";
			}
			iterator.Next();
		}
		this->ofs.close();
	}	
};