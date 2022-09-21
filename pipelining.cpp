#include "pipelining.h"

using namespace std;

int cycle = 1;
int end_pc = 0;
int printm = 0;
string hazard = "no";


struct STATE {
	IF_ID id;
	ID_EX ix;
	EX_MEM exm;
	MEM_WB mwb;
	NPCS npcs;
	STAGE stage;
};

STATE one, newone;

int main(int argc, const char *argv[])
{

	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-atp") == 0) {
			branch_predictor = true;
		} else if (strcmp(argv[i], "-antp") == 0) {
			branch_predictor = false;
		} else if (strcmp(argv[i], "-m") == 0) {
			argv_m = 1;
			char str[50];
			strcpy(str, argv[i + 1]);
			char *addr1 = strtok(str, ":");
			char *addr2 = strtok(NULL, ":");
			add1 = stoul(addr1, NULL, 16);
			add2 = stoul(addr2, NULL, 16);
		} else if (strcmp(argv[i], "-d") == 0) {
			argv_d = 1;
		} else if (strcmp(argv[i], "-n") == 0) {
			argv_n = 1;
			ins_num = stoul(argv[i + 1], NULL, 10);
		} else if (strcmp(argv[i], "-p") == 0) {
			argv_p = 1;
		}
	}

	char sample[30];
	strcpy(sample, argv[argc - 1]);

	ifstream readf(sample);

	string line;
	if (readf.is_open()) {
		while (!readf.eof()) {
			getline(readf, line);
			char str[50];
			strcpy(str, line.c_str());
			if (cnt == 0) {
				textcnt = stoul(str, NULL, 16) / 4;
				cnt++;
			} else if (cnt == 1) {
				datacnt = stoul(str, NULL, 16) / 4;
				cnt++;
			} else if (cnt >= 2 && cnt < textcnt + 2) {
				text = stoul(str, NULL, 16);
				textmem[pcstart + textsize * 4] = text;
				ins[textsize] = text;
				textsize++;
				cnt++;
			} else if (cnt >= textcnt + 2 && strlen(str) != 0) {
				dataa = stoul(str, NULL, 16);
				datamem[datastart + datasize * 4] = dataa;
				datasize++;
			}
		}
	}
	//STATE one, newone;
	one.npcs.if_pc = pc;
	newone.npcs.if_pc = pc;
	one.stage.if_noop = 1;
	newone.stage.if_noop = 1;

	while (1) {
		
		if (one.stage.wb_noop != 0) {
			WB();

		}
		if (one.stage.mem_noop != 0) {
			MEM();
		}
		if (one.stage.ex_noop != 0) {
			EX();
		}
		if (one.stage.id_noop != 0) {
			ID();
		}
		if (one.stage.if_noop != 0) {
			IF();
		}
	
		if (argv_p) printpc(&one.npcs);	
		if (argv_d) {
			cout << "Current register values:" << endl;
			cout << "PC: 0x" << hex << newone.npcs.if_pc << endl;
			print_reg();
		}
		if (argv_p && argv_m || argv_d && argv_m || argv_d && argv_p
		    && argv_m) {
			print_mem();
			printm++;
		}

		if(argv_n) if (one.npcs.wb_pc == 0x400000 + (ins_num-1)*4) break;
		if (one.npcs.if_pc == 0 && one.npcs.id_pc == 0 && one.npcs.ex_pc == 0 && one.npcs.mem_pc == 0 && one.npcs.wb_pc == 0) break;

		cycle++;
		one = newone;
	}

	if (argv_p != 1) printpc(&one.npcs);
	if (argv_d != 1) {
		cout << "Current register values:" << endl;
		cout << "PC: 0x" << hex << end_pc << endl;
		print_reg();
	}
	if (printm == 0 && argv_m)
		print_mem();
	return 0;

}

void print_reg()
{
	cout << "Registers:" << endl;
	for (int i = 0; i < 32; i++) {
		cout << "R" << dec << i << ": 0x" << hex << reg[i] << endl;
	}
	cout << " " << endl;
}

void printpc(struct NPCS *npcs)
{

	if (npcs->if_pc == 0 && npcs->id_pc == 0 && npcs->ex_pc == 0 && npcs->mem_pc == 0 && npcs->wb_pc == 0) cycle--; 
	cout << "===== Completion cycle: " << dec << cycle << " =====" << endl
	    << endl;
	cout << "Current pipeline PC state:" << endl;

	stringstream s1, s2, s3, s4, s5;
	s1 << hex << npcs->if_pc;
	s2 << hex << npcs->id_pc;
	s3 << hex << npcs->ex_pc;
	s4 << hex << npcs->mem_pc;
	s5 << hex << npcs->wb_pc;

	string str1 = (npcs->if_pc == 0) ? "" : "0x" + s1.str();
	string str2 = (npcs->id_pc == 0) ? "" : "0x" + s2.str();
	string str3 = (npcs->ex_pc == 0) ? "" : "0x" + s3.str();
	string str4 = (npcs->mem_pc == 0) ? "" : "0x" + s4.str();
	string str5 = (npcs->wb_pc == 0) ? "" : "0x" + s5.str();

	cout << "{" << hex << str1 << "|" << str2 << "|" << str3 << "|" << str4 << "|" << str5 << "}" << endl;
	cout << " " << endl;

}

int textfind(int instext)
{
	for (auto iter = textmem.begin(); iter != textmem.end(); iter++) {
		if (iter->first == instext) {
			return iter->second;
		}
	}
	return 0;
}


string check_hazard_mwb(){
         if (newone.mwb.format == "I") {
                  if (newone.mwb.instr_s == "lw" || newone.mwb.instr_s == "lb") {
                             if (newone.mwb.rt == one.ix.rs && newone.mwb.rt != one.ix.rt){
                                        one.ix.forward_res = newone.mwb.mem_out;
                                        hazard = "rs";
                             }
                             else if (newone.mwb.rt == one.ix.rt && newone.mwb.rt != one.ix.rs){
                                        one.ix.forward_res = newone.mwb.mem_out;
                                        hazard= "rt";
                             }
                             else if(newone.mwb.rt == one.ix.rt && newone.mwb.rs == one.ix.rs){
                                        one.ix.forward_res = newone.mwb.mem_out;
                                        hazard = "rsrt";
                             } else hazard = "no";
                } else {
                            if (newone.mwb.rt == one.ix.rs && newone.mwb.rt != one.ix.rt){
                                        one.ix.forward_res = newone.mwb.alu_out;
                                        hazard = "rs";
                            } else if (newone.mwb.rt == one.ix.rt && newone.mwb.rt != one.ix.rs){
                                        one.ix.forward_res = newone.mwb.alu_out;
                                        hazard= "rt";
                            } else if(newone.mwb.rt == one.ix.rt && newone.mwb.rs == one.ix.rs){
                                     one.ix.forward_res = newone.mwb.alu_out;
                                     hazard = "rsrt";
                            } else hazard = "no";
                }
    } else  if (newone.mwb.format == "R" && newone.mwb.rd != 0 ){
            if (newone.mwb.rd == one.ix.rs && newone.mwb.rd != one.ix.rt){
                                one.ix.forward_res = newone.mwb.alu_out;
                                hazard = "rs";
            }
            else if (newone.mwb.rd == one.ix.rt && newone.mwb.rd != one.ix.rs){
                                one.ix.forward_res = newone.mwb.alu_out;
                                hazard = "rt";
            }
            else if (newone.mwb.rd == one.ix.rt && newone.mwb.rd == one.ix.rs){
                                one.ix.forward_res = newone.mwb.alu_out;
                                hazard = "rsrt";
            } else  hazard ="no";
    }
	 return  hazard;

}

string check_hazard_exm(){
		 if (newone.exm.format == "R" && newone.exm.rd != 0) {
            if (newone.exm.rd == one.ix.rs && newone.exm.rd != one.ix.rt){
                                one.ix.forward_res = newone.exm.alu_out;
                                hazard = "rs";
			}
			else if (newone.exm.rd == one.ix.rt && newone.exm.rd != one.ix.rt){
                                one.ix.forward_res = newone.exm.alu_out;
                                hazard = "rt";
			}
			else if (newone.exm.rd == one.ix.rt && newone.exm.rd == one.ix.rs){
                                one.ix.forward_res = newone.exm.alu_out;
                                hazard = "rsrt";
			} else  hazard ="no";
    } else if (newone.exm.format == "I") {
            if (newone.exm.rt == one.ix.rs && newone.exm.rt != one.ix.rt){
                                one.ix.forward_res = newone.exm.alu_out;
                                hazard = "rs";
			}
			else if (newone.exm.rt == one.ix.rt && newone.exm.rt != one.ix.rs){
                                one.ix.forward_res = newone.exm.alu_out;
                                hazard = "rt";
			} else if ( newone.exm.rt == one.ix.rt && newone.exm.rt && one.ix.rs){
                                one.ix.forward_res = newone.exm.alu_out;
                                hazard = "rsrt";
			} else hazard = "no";
    }
	return hazard;
}


void IF(){

	newone.stage.id_noop = one.stage.if_noop;

	if (one.npcs.if_pc >= pcstart + textsize * 4) {
		end_pc = one.npcs.if_pc;
		one.npcs.if_pc = 0;
		newone.id.NPC = 0;
		newone.npcs.id_pc = newone.id.NPC;
		return;
	}
	if (one.npcs.if_pc == 0) {
		newone.id.NPC = 0;
		newone.npcs.id_pc = newone.id.NPC;
		return;

	}

	if (one.stage.if_load_stall) {
		newone.stage.if_load_stall = 0;
		newone.npcs.if_pc = one.npcs.if_pc;
		newone.id.instr = textfind(one.npcs.id_pc);
		newone.id.NPC = one.npcs.id_pc;
		newone.npcs.id_pc = one.id.NPC;
	} else if ( one.stage.if_jump_stall){
		newone.stage.if_jump_stall  = 0;
		newone.id.NPC =0;
		newone.id.instr =0;
		newone.npcs.id_pc = newone.id.NPC;
	}else if (one.stage.if_branch_stall) {
		newone.stage.if_branch_stall  = 0;
        newone.id.NPC =0;
        newone.id.instr =0;
        newone.npcs.id_pc = newone.id.NPC;
	} else {
		newone.id.instr = textfind(one.npcs.if_pc);
		newone.id.NPC = one.npcs.if_pc;
		newone.npcs.if_pc = one.npcs.if_pc + 4;
		newone.npcs.id_pc = one.id.NPC;
	}

	return;
}

void ID()
{				// format 지정, ins 변수 만들어주기, op, rs 등등 각각 할당

	if (one.id.NPC == 0) {
		newone.ix.NPC = 0;
		return;
	}

	unsigned int mips[6];
	int binary = one.id.instr;

	if ( binary ==0 ) return;

	mips[0] = (unsigned int)((binary >> 26) & 0x3f);
	if (mips[0] == 0) {
		// Rtype
		one.ix.format = "R";
		mips[1] = (unsigned int)((binary >> 21) & 0x1f);	// rs
		one.ix.rs = mips[1];
		mips[2] = (unsigned int)((binary >> 16) & 0x1f);	// rt
		one.ix.rt = mips[2];
		mips[3] = (unsigned int)((binary >> 11) & 0x1f);	// rd
		one.ix.rd = mips[3];
		mips[4] = (unsigned int)((binary >> 6) & 0x1f);	// shamt
		one.ix.shamt = mips[4];
		mips[5] = (unsigned int)((binary >> 0) & 0x3f);	// funct
		int funct = mips[5];

		if (funct == 0x21) {	// addu
			one.ix.instr_s = "addu";
		} else if (funct == 0x24) {	//and
			one.ix.instr_s = "and";
		} else if (funct == 8) {	//branch (jr)
			one.ix.instr_s = "jr";
		} else if (funct == 0x27) {	//nor
			one.ix.instr_s = "nor";
		} else if (funct == 0x25) {	// or
			one.ix.instr_s = "or";
		} else if (funct == 0x2b) {	//sltu                         
			one.ix.instr_s = "sltu";
		} else if (funct == 0) {	//sll
			one.ix.instr_s = "sll";
		} else if (funct == 2) {	//srl
			one.ix.instr_s = "srl";
		} else if (funct == 0x23) {	//subu
			one.ix.instr_s = "subu";
		}
	}
	// Jtype
	else if (mips[0] == 2 || mips[0] == 3) {
		one.ix.format = "J";
		int op = mips[0];
		mips[1] = (unsigned int)((binary >> 0) & 0x3ffffff);	//target
		one.ix.target = mips[1];
		if (op == 2) {	// j
			one.ix.instr_s = "j";
		} else {	//jal
			one.ix.instr_s = "jal";
		}
	} else {
		// Itype
		one.ix.format = "I";

		int op = mips[0];
		mips[1] = (unsigned int)((binary >> 21) & 0x1f);	//rs
		one.ix.rs = mips[1];
		mips[2] = (unsigned int)((binary >> 16) & 0x1f);	//rt
		one.ix.rt = mips[2];
		mips[3] = (unsigned int)((binary >> 0) & 0xffff);	//imm or add
		one.ix.imm = mips[3];

		if (op == 9) {	//addiu
			one.ix.instr_s = "addiu";
		} else if (op == 0xc) {	//andi
			one.ix.instr_s = "andi";
		} else if (op == 4) {	//beq
			one.ix.instr_s = "beq";
		} else if (op == 5) {	//bne
			one.ix.instr_s = "bne";
		} else if (op == 0xf) {
			one.ix.instr_s = "lui";
		} else if (op == 0x23) {	//lw offset 4의 배수만
			one.ix.instr_s = "lw";
		} else if (op == 0x20) {	//lb
			one.ix.instr_s = "lb";
		} else if (op == 0xd) {	//ori
			one.ix.instr_s = "ori";
		} else if (op == 0xb) {	//sltiu
			one.ix.instr_s = "sltiu";
		} else if (op == 0x2b) {	//sw, offset 4의 배수만
			one.ix.instr_s = "sw";
		} else if (op == 0x28) {	// sb;
			one.ix.instr_s = "sb";
		}
	}

	if (one.stage.id_branch_stall) {
                newone.stage.id_branch_stall  = 0;
                newone.ix.NPC =0;
                newone.ix.instr_s ="";
                newone.npcs.ex_pc = newone.ix.NPC;
                one.npcs.id_pc = one.id.NPC;
                newone.ix.rs = 0;
                newone.ix.rt = 0;
                newone.ix.rd = 0;
                newone.ix.forward_res =0;
                return;
        } 

	if (newone.exm.load_sig==1){ 
                if ( newone.exm.rt == one.ix.rs || newone.exm.rt == one.ix.rt){
                         one.stage.if_load_stall = 1;
                         newone.npcs.ex_pc = 0;
                         one.npcs.id_pc = one.id.NPC;
                         newone.ix.rs = 0;
                         newone.ix.rt = 0;
                         newone.ix.rd = 0;
                         newone.ix.forward_res =0;
                         newone.ix.mwb_hazard="no";
                         newone.ix.exm_hazard= "no";
                         newone.ix.load_sig = 0;
			 return;
        	}
	}

	newone.ix.mwb_hazard = check_hazard_mwb();
	newone.ix.exm_hazard = check_hazard_exm();

	if ( one.ix.instr_s == "jr") {
	   if (newone.ix.mwb_hazard == "no" && newone.ix.exm_hazard == "no") newone.npcs.if_pc  = reg[one.ix.rs];
	   if (newone.ix.mwb_hazard == "rs" && newone.ix.exm_hazard == "rs") newone.npcs.if_pc = one.ix.forward_res;

	   	one.stage.if_jump_stall = 1;
		one.npcs.id_pc = one.id.NPC;
        newone.npcs.ex_pc = one.npcs.id_pc;
        newone.stage.ex_noop = one.stage.id_noop;
        newone.ix.instr_s = one.ix.instr_s;
		newone.ix.NPC = one.id.NPC + 4;
     		return;		

	}
	if (one.ix.instr_s == "j") {
		one.stage.if_jump_stall = 1;
        newone.npcs.if_pc = one.ix.target *4;
        one.npcs.id_pc = one.id.NPC;
        newone.npcs.ex_pc = one.npcs.id_pc;
        newone.stage.ex_noop = one.stage.id_noop;
        newone.ix.instr_s = one.ix.instr_s;
		newone.ix.NPC = one.id.NPC + 4;
		return;
	}
	if (one.ix.instr_s == "jal") {
		one.stage.if_jump_stall = 1;
        newone.npcs.if_pc = one.ix.target *4;
		one.npcs.id_pc = one.id.NPC;
		newone.npcs.ex_pc = one.npcs.id_pc;
		newone.stage.ex_noop = one.stage.id_noop;
		newone.ix.instr_s = one.ix.instr_s;
        newone.ix.format = one.ix.format;
		newone.ix.NPC = one.id.NPC + 4;
		return;
	}

	newone.ix.load_sig = (one.ix.instr_s == "lw"|| one.ix.instr_s == "lb") ? 1 : 0;
	newone.ix.store_sig = (one.ix.instr_s == "sw"|| one.ix.instr_s == "sb") ? 1 : 0;
	

	newone.ix.NPC = one.id.NPC + 4;
	newone.ix.instr_s = one.ix.instr_s;
	newone.ix.format = one.ix.format;
	newone.ix.rt = one.ix.rt;
	newone.ix.forward_res = one.ix.forward_res;
	newone.ix.rs = one.ix.rs;
	newone.ix.rd = one.ix.rd;
	newone.ix.imm = one.ix.imm;
	newone.ix.shamt = one.ix.shamt;
	newone.ix.target = one.ix.target;
	one.npcs.id_pc = one.id.NPC;
	newone.npcs.ex_pc = one.npcs.id_pc;
	newone.stage.ex_noop = one.stage.id_noop;


	if (one.ix.instr_s == "beq"|| one.ix.instr_s == "bne") {
		if (branch_predictor) { // TAKEN일 때  무조건 1 stall
			one.stage.if_branch_stall = 1;
                	newone.npcs.if_pc =  one.ix.NPC + 4 + (one.ix.imm) * 4;
                	one.npcs.id_pc = one.id.NPC;
		}
	}


	
	return;
}

void alu_ins(string ha)
{
	if (one.ix.format == "R") {
		newone.exm.format = one.ix.format;
		if (one.ix.instr_s == "addu") {	// addu
			cout << "hazard: " << ha << endl;
			if ( ha == "no") one.exm.alu_out = reg[one.ix.rs] + reg[one.ix.rt];
			else if ( ha == "rs") one.exm.alu_out = one.ix.forward_res + reg[one.ix.rt];
			else if ( ha == "rt") one.exm.alu_out = reg[one.ix.rs] + one.ix.forward_res;
			else if ( ha == "rsrt") one.exm.alu_out = one.ix.forward_res + one.ix.forward_res;
		} else if (one.ix.instr_s == "and") {	//and
			if ( ha == "no")  one.exm.alu_out = reg[one.ix.rs] & reg[one.ix.rt];
			else if ( ha == "rs") one.exm.alu_out = one.ix.forward_res & reg[one.ix.rt];
            else if ( ha == "rt") one.exm.alu_out = reg[one.ix.rs] & one.ix.forward_res;
            else if ( ha == "rsrt") one.exm.alu_out = one.ix.forward_res & one.ix.forward_res;
		} else if (one.ix.instr_s == "nor") {	//nor
			if (ha == "no") one.exm.alu_out = ~(reg[one.ix.rs] | reg[one.ix.rt]);
			else if ( ha == "rs") one.exm.alu_out = ~(one.ix.forward_res | reg[one.ix.rt]);
            else if ( ha == "rt") one.exm.alu_out = ~ (reg[one.ix.rs] | one.ix.forward_res);
            else if ( ha == "rsrt") one.exm.alu_out = ~(one.ix.forward_res | one.ix.forward_res);
		} else if (one.ix.instr_s == "or") {	// or
			if ( ha == "no") one.exm.alu_out = reg[one.ix.rs] | reg[one.ix.rt];
			else if ( ha == "rs") one.exm.alu_out = (one.ix.forward_res) | reg[one.ix.rt];
            else if ( ha == "rt") one.exm.alu_out = reg[one.ix.rs] | one.ix.forward_res;
            else if ( ha == "rsrt") one.exm.alu_out = one.ix.forward_res | one.ix.forward_res;
		} else if (one.ix.instr_s == "sltu") {	//sltu 
			if (ha == "no") (reg[one.ix.rs] < reg[one.ix.rt])? one.exm.alu_out = 1 : one.exm.alu_out = 0;
			else if (ha == "rs") (one.ix.forward_res < reg[one.ix.rt])? one.exm.alu_out = 1 : one.exm.alu_out = 0;             
			else if (ha == "rt") (reg[one.ix.rs] < one.ix.forward_res)? one.exm.alu_out = 1 : one.exm.alu_out = 0;
		} else if (one.ix.instr_s == "sll") {	//sll
			if (ha == "no") one.exm.alu_out = (reg[one.ix.rt] << one.ix.shamt);
			else if ( ha == "rt") one.exm.alu_out = (one.ix.forward_res << one.ix.shamt);
		} else if (one.ix.instr_s == "srl") {	//srl
			if (ha == "no") one.exm.alu_out = (reg[one.ix.rt] >> one.ix.shamt);
			else if ( ha == "rt") one.exm.alu_out = (one.ix.forward_res >> one.ix.shamt);
		} else if (one.ix.instr_s == "subu") {	//subu
			if ( ha == "no") one.exm.alu_out = reg[one.ix.rs] - reg[one.ix.rt];
            else if ( ha == "rs") one.exm.alu_out = one.ix.forward_res - reg[one.ix.rt];
            else if ( ha == "rt") one.exm.alu_out = reg[one.ix.rs] - one.ix.forward_res;
            else if ( ha == "rsrt") one.exm.alu_out = one.ix.forward_res - one.ix.forward_res;
		}
	} else if (one.ix.format =="J"){

		newone.exm.format = one.ix.format;
		if(one.ix.instr_s == "jal") {
			one.exm.alu_out = one.ix.NPC;
		}
	}else if (one.ix.format == "I") {
		newone.exm.format = one.ix.format;
		int twos = 0;
		int plus = 0;

		if (one.ix.instr_s == "addiu") {	//addiu
			if ((one.ix.imm >> 15) & 0x01 == 1) {	//signed이면
				one.ix.imm = one.ix.imm | 0xffff0000;
				one.ix.imm = ~(one.ix.imm);
				one.ix.imm += 1;
				twos = -(one.ix.imm);
				if (ha == "no") one.exm.alu_out = reg[one.ix.rs] + twos;
				if (ha == "rs") one.exm.alu_out = one.ix.forward_res + twos;
			} else {
				one.ix.imm = one.ix.imm | 0x0000000;
				if (ha == "no") one.exm.alu_out = reg[one.ix.rs] + one.ix.imm;
				if (ha == "rs") one.exm.alu_out = one.ix.forward_res + one.ix.imm;
			}
		} else if (one.ix.instr_s == "bne"){ // bne
			if (ha == "no")  if ( reg[one.ix.rs] != reg[one.ix.rt] ) {
                                one.exm.alu_out  = one.ix.NPC + one.ix.imm*4;
				newone.stage.mem_prediction_good = true;
			 } else newone.stage.mem_prediction_good = false;

			else if(ha == "rs") if (one.ix.forward_res != reg[one.ix.rt]) {
				one.exm.alu_out  = one.ix.NPC + one.ix.imm*4;
                                newone.stage.mem_prediction_good = true;
                         } else newone.stage.mem_prediction_good = false;

			else if(ha == "rt") if (one.ix.forward_res != reg[one.ix.rs]) {
                                one.exm.alu_out  = one.ix.NPC + one.ix.imm*4;
                                newone.stage.mem_prediction_good = true;
                         } else newone.stage.mem_prediction_good = false;

			else if(ha == "rsrt") if (one.ix.forward_res != one.ix.forward_res) {
                                one.exm.alu_out  = one.ix.NPC + one.ix.imm*4;
                                newone.stage.mem_prediction_good = true;
                         } else newone.stage.mem_prediction_good = false;
		} else if (one.ix.instr_s == "beq"){ //beq
			if (ha == "no")  if ( reg[one.ix.rs] == reg[one.ix.rt] ) {
                                one.exm.alu_out  = one.ix.NPC + one.ix.imm*4;
                                newone.stage.mem_prediction_good = true;
                         } else newone.stage.mem_prediction_good = false;

                        else if(ha == "rs") if (one.ix.forward_res == reg[one.ix.rt]) {
                                one.exm.alu_out  = one.ix.NPC + one.ix.imm*4;
                                newone.stage.mem_prediction_good = true;
                         } else newone.stage.mem_prediction_good = false;

                        else if(ha == "rt") if (one.ix.forward_res == reg[one.ix.rs]) {
                                one.exm.alu_out  = one.ix.NPC + one.ix.imm*4;
                                newone.stage.mem_prediction_good = true;
                         } else newone.stage.mem_prediction_good = false;

                        else if(ha == "rsrt") if (one.ix.forward_res == one.ix.forward_res) {
                                one.exm.alu_out  = one.ix.NPC + one.ix.imm*4;
                                newone.stage.mem_prediction_good = true;
                         } else newone.stage.mem_prediction_good = false;			     
		} else if (one.ix.instr_s == "andi") {	//andi
			one.ix.imm = one.ix.imm | 0x00000000;
			if (ha == "no") one.exm.alu_out = reg[one.ix.rs] & one.ix.imm;
			if (ha == "rs") one.exm.alu_out = one.ix.forward_res & one.ix.imm;
		} else if (one.ix.instr_s == "lui") {
			one.exm.alu_out = (one.ix.imm << 16) | 0;	// lui
		} else if ( one.ix.instr_s == "lw" || one.ix.instr_s =="lb"){ //lw or lb
			plus = one.ix.imm/4;
			if (ha =="no"||ha == "rt") one.exm.alu_out =reg[one.ix.rs]+4*plus;
			if (ha == "rs") one.exm.alu_out = one.ix.forward_res +4*plus;
		} else if (one.ix.instr_s == "sw" || one.ix.instr_s == "sb"){ //sw or sb
			plus = one.ix.imm/4;
			if ( ha == "no" || ha =="rt")one.exm.alu_out = reg[one.ix.rs] + 4*plus;
			if (ha == "rs") one.exm.alu_out = one.ix.forward_res +4*plus;
		} else if (one.ix.instr_s == "ori"){
			one.ix.imm = one.ix.imm | 0x00000000;
			if (ha == "no") one.exm.alu_out= reg[one.ix.rs] | one.ix.imm;
			if (ha == "rs") one.exm.alu_out = one.ix.forward_res | one.ix.imm;
			if (ha == "rsrt") one.exm.alu_out = one.ix.forward_res | one.ix.imm;
		}else if (one.ix.instr_s == "sltiu") {	//sltiu
			if ((one.ix.imm >> 15) & 0x01 == 1) {
				one.ix.imm = one.ix.imm | 0xffff0000;
				one.ix.imm = ~one.ix.imm;
				one.ix.imm += 1;
				twos = -(one.ix.imm);

				 if (ha == "no") (reg[one.ix.rs] < twos)? one.exm.alu_out = 1 : one.exm.alu_out = 0;
                       		 else if (ha == "rs") (one.ix.forward_res < twos )? one.exm.alu_out = 1 : one.exm.alu_out = 0;
			} else {
				one.ix.imm = one.ix.imm | 0x00000000;
				if (ha == "no") (reg[one.ix.rs] < one.ix.imm)? one.exm.alu_out = 1 : one.exm.alu_out = 0;
                                else if (ha == "rs") (one.ix.forward_res < one.ix.imm )? one.exm.alu_out = 1 : one.exm.alu_out = 0;
			}
		}
	}
	return;
}

void EX()
{
	if (one.ix.NPC == 0) {
		one.npcs.ex_pc = 0;
		newone.npcs.mem_pc = 0;
		return;
	}
	
	 if (one.stage.ex_branch_stall ==1) {
                newone.stage.ex_branch_stall  = 0;
                newone.exm.instr_s ="";
                newone.npcs.mem_pc = 0;
                newone.exm.rs = 0;
                newone.exm.rt = 0;
                newone.exm.rd = 0;
                return;
        }

	if ( one.ix.instr_s == "j" || one.ix.instr_s == "jr"){
		newone.stage.mem_noop = one.stage.ex_noop;
		newone.npcs.mem_pc = one.npcs.ex_pc;
		newone.exm.instr_s = one.ix.instr_s;
		return;
	}


	if (one.exm.load_sig ==1 ) { // mem에 lw,lb있을 떄
		if ( one.exm.rt == one.ix.rt || one.exm.rt == one.ix.rs){// hazard 조건 추가해야함
			newone.exm.load_sig =0;
			newone.exm.store_sig =0;
            newone.npcs.mem_pc = 0;
			newone.exm.rs = 0;
			newone.exm.rt = 0;
			newone.exm.rd = 0;
			newone.exm.alu_out = 0;
			newone.exm.load_sig = 0;
		        return;	
		}
	}
	

	hazard = ( one.ix.mwb_hazard != "no" && one.ix.exm_hazard =="no")? one.ix.mwb_hazard : one.ix.exm_hazard;
       	if ( one.ix.mwb_hazard == "no" && one.ix.exm_hazard == "no") hazard ="no";
	if ( one.mwb.instr_s == "bne" ||one.mwb.instr_s == "beq") hazard = "no";

	alu_ins(hazard);

	hazard = "no";

	newone.stage.mem_noop = one.stage.ex_noop;
	newone.exm.rs = one.ix.rs;
	newone.exm.rt = one.ix.rt;
	newone.exm.rd = one.ix.rd;
	newone.exm.imm = one.ix.imm;
	newone.exm.instr_s = one.ix.instr_s;
	newone.exm.alu_out = one.exm.alu_out;
	newone.exm.load_sig = one.ix.load_sig;
	newone.exm.store_sig = one.ix.store_sig;
	newone.npcs.mem_pc = one.npcs.ex_pc;
	newone.stage.mem_noop = one.stage.ex_noop;



	return;
}

void MEM()
{
	if (one.npcs.mem_pc == 0) {
		newone.npcs.wb_pc = 0;
		return;
	}


	if (one.exm.instr_s == "beq" || one.exm.instr_s =="bne"){ // mem에서 branch일 때 
		if (branch_predictor && one.stage.mem_prediction_good){ // taken인데 taken
			one.stage.mem_prediction_good = false;
			newone.npcs.wb_pc = one.npcs.mem_pc;
			newone.stage.wb_noop = one.stage.mem_noop;
			newone.mwb.instr_s = one.exm.instr_s;
                        return;
		} else if ( branch_predictor && one.stage.mem_prediction_good == false){ // taken인데 not
			one.stage.if_branch_stall = 1;
			one.stage.id_branch_stall = 1;
			one.stage.ex_branch_stall = 1;
            newone.npcs.if_pc =  one.npcs.ex_pc + 4;
			newone.npcs.wb_pc = one.npcs.mem_pc;
			newone.stage.wb_noop = one.stage.mem_noop;
			newone.mwb.instr_s = one.exm.instr_s;
			return;
		} else if ( branch_predictor == false && one.stage.mem_prediction_good){ // NOT taken인데 taken
			one.stage.if_branch_stall = 1;
            one.stage.id_branch_stall = 1;
            one.stage.ex_branch_stall = 1;
            newone.npcs.if_pc =  one.exm.alu_out;
            newone.npcs.wb_pc = one.npcs.mem_pc;
            newone.stage.wb_noop = one.stage.mem_noop;
            newone.mwb.instr_s = one.exm.instr_s;
            return;
		} else if (branch_predictor == false && one.stage.mem_prediction_good == false){ // NOT Taken인데 not
            newone.npcs.wb_pc = one.npcs.mem_pc;
            newone.stage.wb_noop = one.stage.mem_noop;
            newone.mwb.instr_s = one.exm.instr_s;
			return;
		}


	}

	if ( one.exm.instr_s == "j" || one.exm.instr_s == "jr"){
                newone.stage.wb_noop = one.stage.mem_noop;
                newone.npcs.wb_pc = one.npcs.mem_pc;
                newone.mwb.instr_s = one.exm.instr_s;
                return;
        }


	if ( one.exm.load_sig) {
		if ( one.exm.instr_s == "lw"){
                        one.mwb.mem_out = addfind(one.exm.alu_out);
		}
		else if ( one.exm.instr_s == "lb"){
			int plus_off = one.exm.imm%4;
			one.mwb.mem_out = (addfind(one.exm.alu_out)<<(8*plus_off))>>24;
		}
	}

	if ( one.exm.store_sig ==1){
	    if ( one.mwb.load_sig ==0){
		string hazard_mem = "no";
		if(one.exm.rt == one.mwb.rt || one.exm.rt == one.mwb.rd) hazard_mem ="rt";

		if ( one.exm.instr_s == "sw"){

			if (hazard_mem == "no")  datamem [one.exm.alu_out] = reg[one.exm.rt];
			if (hazard_mem == "rt")  datamem [one.exm.alu_out] = one.mwb.alu_out;
		}
		else if (one.exm.instr_s =="sb"){
			int ret3=0;
			ret3 = (hazard_mem == "no")? reg[one.exm.rt]:one.mwb.alu_out;
			int plus_off = one.exm.imm%4;
			int ret1 = datamem[one.exm.alu_out];
			int ret2 = 0;

			if (plus_off ==0) {
				ret2 = ret1 & 0x00ffffff ;
				ret2 |=  ret3 <<24;
			} else if (plus_off ==1) {
                ret2 = ret1 & 0xff00ffff ;
                ret2 |=  (ret3 <<24)>>8;
            } else if (plus_off ==2) {
                ret2 = ret1 & 0xffff00ff ;
                ret2 |=  (ret3 <<24)>>16;
            } else if (plus_off ==3) {
                ret2 = ret1 & 0xffffff00 ;
                ret2 |=  (ret3 <<24)>>24;
            }
			
			datamem[one.exm.alu_out] = ret2;
		}
	    } else{ // lw sw연달아 올 때
		   if(one.exm.rt == one.mwb.rt){ // 겹침이 발생하면
			 if ( one.exm.instr_s == "sw"){
                        	datamem [one.exm.alu_out] = one.mwb.mem_out;
                	}
                	else if (one.exm.instr_s =="sb"){
                        	int plus_off = one.exm.imm%4;
                        	int ret1 = datamem[one.exm.alu_out];
                        	int ret2=0;

                        	if (plus_off ==0) {
                                	ret2 = ret1 & 0x00ffffff ;
                                	ret2 |=  one.mwb.mem_out <<24;
                       	 	} else if (plus_off ==1) {
                                	ret2 = ret1 & 0xff00ffff ;
                               		ret2 |=  (one.mwb.mem_out <<24)>>8;
                        	} else if (plus_off ==2) {
                                	ret2 = ret1 & 0xffff00ff ;
                                	ret2 |=  (one.mwb.mem_out <<24)>>16;
                        	} else if (plus_off ==3) {
                                	ret2 = ret1 & 0xffffff00 ;
                                	ret2 |=  (one.mwb.mem_out <<24)>>24;
                        	}

                       		 datamem[one.exm.alu_out] = ret2;
                	}

		   }
	    }
	}

	newone.mwb.rs = one.exm.rs;
	newone.mwb.rt = one.exm.rt;
	newone.mwb.rd = one.exm.rd;
	newone.mwb.instr_s = one.exm.instr_s;
	newone.mwb.format = one.exm.format;
	newone.mwb.alu_out = one.exm.alu_out;
	newone.mwb.mem_out = one.mwb.mem_out;
	newone.mwb.load_sig = one.exm.load_sig;
    newone.mwb.store_sig = one.exm.store_sig;
	newone.stage.wb_noop = one.stage.mem_noop;
	newone.npcs.wb_pc = one.npcs.mem_pc;

	return;
}

void WB()
{

	if (one.npcs.wb_pc == 0) {
		return;
	}

	
	if (one.npcs.wb_pc !=0) {
			if (one.mwb.format == "R") reg[one.mwb.rd] = one.mwb.alu_out;
			else if ( one.mwb.format == "J"){
				if( one.mwb.instr_s == "jal") reg[31] = one.mwb.alu_out;
			} else if (one.mwb.format == "I") {
				if ( one.mwb.load_sig ){
					reg[one.mwb.rt] =one.mwb.mem_out;
				} else if ( one.mwb.load_sig !=1 && one.mwb.store_sig !=1){
					reg[one.mwb.rt] =one.mwb.alu_out;
				}
			}
		}
			        
		newone.stage.wb_noop = one.stage.wb_noop;
		return;

}

int addfind(int add)
{
	for (auto iter = datamem.begin(); iter != datamem.end(); iter++) {
		if (iter->first == add) {
			return iter->second;
		}
	}
	return 0;
}

void print_mem()
{
	cout << " " << endl;
	cout << "Memory content [" << "0x" << hex << add1 <<
	    ".." << "0x" << hex << add2 << "]:" << endl;
	cout << "--------------------------------------------" << endl;

	// -m 주소가 text or data 그리고 범위 벗어나는지 확인해라(add1부터 벗어나는지 add2만 이상한지)
	if (textmem.count(add1)) {
		map < int32_t, int >::iterator iter = textmem.find(add1);
		map < int32_t, int >::iterator iter2 = textmem.find(add2);
		int check_add1 = pcstart + textcnt * 4;

		for (auto it = iter; it != textmem.end(); it++) {
			cout << "0x" << hex << it->first << ": 0x" <<
			    hex << it->second << endl;
			if (it == iter2)
				break;
		}

		if (add2 >= check_add1) {
			int num = (add2 - check_add1) / 4;
			for (int i = 0; i < num + 1; i++) {
				cout << "0x" << hex <<
				    check_add1 + 4 * i << ": 0x0" << endl;
			}
		}
	} else if (datamem.count(add1)) {
		map < int32_t, int >::iterator iter = datamem.find(add1);
		map < int32_t, int >::iterator iter2 = datamem.find(add2);
		int check_add2 = datastart + datacnt * 4;

		for (auto it = iter; it != datamem.end(); it++) {
			cout << "0x" << hex << it->first << ": 0x" <<
			    hex << it->second << endl;
			if (it == iter2)
				break;
		}

		if (add2 >= check_add2) {
			int num = (add2 - check_add2) / 4;
			for (int i = 0; i < num + 1; i++) {
				cout << "0x" << hex <<
				    check_add2 + 4 * i << ": 0x0" << endl;
			}
		}
	} else {
		int num = (add2 - add1) / 4;
		for (int i = 0; i < num + 1; i++) {
			cout << "0x" << hex << add1 + 4 * i << ": 0x0" << endl;
		}
	}
	cout << " " << endl;
}
