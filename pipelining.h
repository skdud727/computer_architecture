#ifndef __PIPELINING_H
#define __PIPELINING_H
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <bitset>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <cstring>

using namespace std;

void printpc(struct NPCS * npcs);
int textfind(int instext);
void alu_ins();
void IF();
void ID();
void EX();
void MEM();
void WB();
void print_reg();
void print_mem();
string check_hazard_mwb();
string check_hazard_exm();

vector<int32_t> ins(100);
map<int32_t, int> textmem;
map<int32_t, int> datamem;

int pcstart = 0x400000;
int datastart = 0x10000000;
int pc = 0x400000;

int dataa = 0;
int text = 0;
int datacnt = 0; 
int textcnt = 0;	 
int cnt = 0;
int datasize = 0;
int textsize = 0;

int argv_d =0;
int argv_n =0;
int argv_m =0;
int argv_p = 0;
bool branch_predictor;


int ins_num=0;
int add1 = 0;
int add2 =0;

int addfind(int add);
vector<int32_t> reg(32,0);

struct IF_ID{
 	int instr=0;
	int NPC=0;
};

struct ID_EX{
	int rs,rt,rd,shamt,imm,target=0;
	int NPC=0;
	int forward_res=0;
	string instr_s;
	string format;
	string mwb_hazard;
	string exm_hazard;
	bool load_sig = false;
	bool store_sig = false;
};

struct EX_MEM{
	string format;
	string instr_s;
	int rs,rt,rd,imm=0;
	int br_target=0;
	int alu_out=0;
	bool load_sig = false;
    bool store_sig = false;
};

struct MEM_WB{
	string format;
	string instr_s;
	int rs,rt,rd=0;
	int alu_out=0;
	int mem_out=0;
	bool load_sig = false;
    bool store_sig = false;
};


struct STAGE{
	int if_noop=1;
	int id_noop=0;
	int ex_noop=0;
	int mem_noop=0;
	int wb_noop=0;
	int if_load_stall =0;
	int if_branch_stall =0;
	int id_branch_stall =0;
	int ex_branch_stall =0;
	int if_jump_stall = 0;
	bool mem_prediction_good = false;
};


struct NPCS{
  	int if_pc;
    int id_pc;
    int ex_pc;
    int mem_pc;
    int wb_pc;
};




#endif









