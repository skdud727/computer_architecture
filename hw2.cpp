#include <stdio.h>
#include <bitset>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <cstring>

using namespace std;

void instruction(int binary);
int addfind(int add);
void printd1();
void printd2();
vector<int32_t> reg(32,0);
vector<int32_t> ins(100);
map<int32_t, int> textmem;
map<int32_t, int> datamem;
int pcstart = 0x400000;
int datastart = 0x10000000;
int datacnt = 0;
int textcnt = 0;
int cnt = 0;
int text = 0;
int data =0;
int datasize = 0;
int textsize = 0;
int pc =0x400000;
int jump_add=0;
int argv_d =0;
int argv_n =0;
int argv_m =0;
int ins_num=0;
int add1 = 0;
int add2 =0;


int main(int argc, const char*argv[]){


  for (int i =0 ; i < argc ; i ++){
           if ( strcmp(argv[i], "-m") ==0 ){
                   argv_m =1;
                   char str[50];
                   strcpy(str, argv[i+1]);
                   char *  addr1 = strtok(str,":");
                   char *  addr2 = strtok(NULL, ":");
                   add1 = stoul(addr1, NULL,16);
                   add2 = stoul(addr2, NULL,16);
           } else if ( strcmp (argv[i], "-d") ==0){
                   argv_d = 1;
           } else if ( strcmp (argv[i] , "-n") == 0) {
                   argv_n = 1;
                   ins_num = stoul( argv[i+1], NULL,10);
           }
   }


   char sample [30];
   strcpy(sample,argv[argc-1]);	

   ifstream readf(sample);

   string line;
   if (readf.is_open()){
	   while(!readf.eof()){
		   getline(readf,line);
		   char str[50];
		   strcpy(str, line.c_str());
		   if ( cnt == 0){
		   	textcnt = stoul(str,NULL,16)/4;
		   	cnt ++;
		   } else if ( cnt ==1) {
                        datacnt = stoul(str,NULL,16)/4;
                        cnt++;
                   }else if (cnt >=2 && cnt <textcnt +2) {
                           text = stoul(str, NULL , 16);
                           textmem[pcstart+textsize*4]=text;
                           ins[textsize]=text;
                           textsize++;
                           cnt ++;
                   } else if (cnt >=textcnt+2 && strlen(str)!=0){
                           data = stoul(str,NULL,16);
                           datamem[datastart+datasize*4] = data;
                           datasize++;
                    }
	       }
      }


   cout << "Current register calues:" << endl;
   cout << "--------------------------------------------" <<endl;

   if ( argv_n ==1) {
	   if ( ins_num == 0) {
   			cout << "PC: 0x" <<hex<<pc<<endl;
   			printd1();
			if ( argv_m ==1) printd2();
	   } else { //insnum !=0일 때
		if ( argv_d ==1){
		     int i=0;	
   		     cout << "PC: 0x" <<hex<<pc<<endl;
   		     printd1();
		   for( int j=0; j< ins_num; j++){
			instruction(ins[i]);
			cout << "PC: 0x" << hex<< pc << endl;
        		printd1();
			if ( jump_add != 0){
				if (textmem.count(jump_add)){
					i = (jump_add-0x400000)/4;
					jump_add=0;
				}
			} else i++;
		        if (argv_m ==1) printd2();
	  	    }
		} else{
			int i=0;
 			for( int j=0; j< ins_num; j++){
                        	instruction(ins[i]);
				if (jump_add !=0){
					if (textmem.count(jump_add)){
					 i =(jump_add-0x400000)/4;
					 jump_add=0;
					}
				} else i++;	
			}
			 cout << "PC: 0x" << hex<< pc << endl;
		 	 printd1();

			if ( argv_m ==1) printd2();
	   	}
   	}
   } else { // argv_n ==0
		if ( argv_d ==1){
			int i=0;
		   while ( textmem.count(pc)!=0){ // 프로그램이 끝날 때 까지
			instruction(ins[i]); 
			cout << "PC: 0x"<< hex<<pc<<endl;
			printd1();
			if (jump_add !=0){
				if (textmem.count(jump_add)){ //jump발생
					i = (jump_add - 0x400000)/4;
					jump_add=0;
				}	
			} else i++;

			if (argv_m ==1) printd2();
		   } 
		} else {
			int i=0;
		    while ( textmem.count(pc)!=0){
			instruction(ins[i]);
			if (jump_add != 0 ){
				if (textmem.count(jump_add)){ //jump 발생
					i = (jump_add - 0x400000)/4;
					jump_add=0;
				}
			} else i++;			
		   } 
		    cout << "PC: 0x" << hex << pc << endl;
		    printd1();

		    if (argv_m ==1) printd2();
		}
	}

   readf.close();
   return 0;
}







void instruction(int binary){
	

	pc +=4;

	unsigned int mips[6];

	mips[0] = (unsigned int)((binary>>26)&0x3f);
	if (mips[0]==0){
	// Rtype
	mips[1] = (unsigned int)((binary>>21)&0x1f); // rs
	int rs = mips[1];
	mips[2] = (unsigned int)((binary>>16)&0x1f); // rt
	int rt = mips[2];
	mips[3] = (unsigned int)((binary>>11)&0x1f);// rd
	int rd = mips[3];
	mips[4] = (unsigned int)((binary>>6)&0x1f); // shamt
	int shamt = mips[4];
	mips[5] = (unsigned int)((binary>>0)&0x3f); // funct
	int funct = mips[5];

	
	 if (funct == 0x21){ // addu
		reg[rd] = reg[rs]+reg[rt];
	 } else if (funct == 0x24){ //and
	 	reg[rd] = reg[rs] & reg[rt];
	 } else if (funct ==8){ //branch (jr)
		jump_add=reg[rs];
		if (textmem.count(jump_add)){
			pc = jump_add;
		}
	 }else if (funct == 0x27){ //nor
	 	reg[rd]= ~(reg[rs] | reg[rt]);
	 } else if (funct == 0x25) { // or
	 	reg[rd]= reg[rs] | reg[rt];
	 } else if (funct == 0x2b){ //sltu
		 if ( reg[rs] < reg[rt]){
			 reg[rd] = 1;
		 } else reg[rd] =0;
	 } else if (funct == 0 ){ //sll
		 reg[rd] = (reg[rt]<<shamt);  
	 } else if (funct == 2) { //srl
		 reg[rd] = (reg[rt]>>shamt);  
	 } else if (funct == 0x23){ //subu
		 reg[rd]= reg[rs] - reg[rt];
	 }
	} 
	// Jtype
	 else if  (mips[0]==2 || mips[0]==3){
		int op = mips[0];
		mips[1] = (unsigned int)((binary>>0)&0x3ffffff); //target
		int target = mips[1];
		jump_add = target *4;
		if (op ==2){ // j
			if (textmem.count(jump_add)){
				pc = jump_add;
			}
			else if( jump_add>=(pcstart+4*textsize)) pc = jump_add; 
		} else { //jal
			reg[31] = pc; // reg[31]은 ra이고 다음 실행할 명령어는 pc에 들어감!
			if (textmem.count(jump_add)){
				pc = jump_add;
			} else if (jump_add>=(pcstart+4*textsize)) pc = jump_add;
		}
	} else { 
	// Itype
	int op = mips[0];
	mips[1] = (unsigned int)((binary>>21)&0x1f); //rs
	int rs = mips[1];
	mips[2] = (unsigned int)((binary>>16)&0x1f);//rt
	int rt = mips[2];
	mips[3] = (unsigned int)((binary>>0)&0xffff); //imm or add
	int imm = mips[3];

	int twos=0;
	int plus = 0;
	int plus_off = 0;
	int add_s = 0;
	int ret1 = 0;
	int ret2 = 0;

		if ( op == 9) { //addiu
			 if ((imm>>15)&0x01 == 1) { //signed 이면!
				 imm = imm | 0xffff0000;
				 imm = ~imm;
				 imm += 1;
				 twos = -imm;
				 reg[rt] = reg[rs] + twos;
			 } else {
				 imm = imm | 0x0000000;
				 reg[rt] = reg[rs] + imm;
			 }
		} else if ( op == 0xc){ //andi
			imm = imm | 0x00000000;
			reg[rt] = reg[rs] & imm;
		} else if ( op == 4){ //beq
			if ( reg[rs]== reg[rt] ) {
				jump_add = pc + imm*4;
				if (textmem.count(jump_add)){
					pc = jump_add;
				} else if (jump_add >= (pcstart+4*textsize)) pc = jump_add;
			}
		} else if ( op == 5) { //bne
			if ( reg[rs] != reg[rt] ) {
				jump_add = pc + imm*4;
				if (textmem.count(jump_add)){
					pc = jump_add;
				} else if (jump_add >= (pcstart+4*textsize)) pc = jump_add;
			}
		} else if ( op == 0xf) {
			reg[rt] = (imm<<16) | 0; // lui
		} else if ( op == 0x23){ //lw offset 4의 배수만
			plus = imm/4;
			add_s =reg[rs]+4*plus;
			ret1 = addfind(add_s);
			reg[rt] = ret1;


		} else if ( op ==0x20){ //lb
			plus = imm/4;
			plus_off = imm %4;
			add_s= reg[rs] + 4*plus;
			ret1 = addfind(add_s);

			if ( plus_off ==0) reg[rt] = ((int)(ret1&0xff000000 >> 24));
                        else if (plus_off ==1) reg[rt] = ret1 &0x00ff0000 >>16;
                        else if (plus_off ==2) reg[rt] = ret1 & 0x0000ff00 >>8;
                        else if (plus_off ==3) reg[rt] = ret1 & 0x00000000ff;
		} else if ( op == 0xd) { //ori
			imm = imm | 0x00000000;
			reg[rt] = reg[rs] | imm;
		} else if ( op == 0xb) { //sltiu
			if ((imm>>15)&0x01 ==1){
				imm = imm | 0xffff0000;
				imm = ~imm;
				imm +=1;
				twos = -imm;
				if (reg[rs] < twos){
					reg[rt] =1;
				} else reg[rt]=0;
			} else {
				imm = imm |0x00000000;
				if (reg[rs]<imm){
					reg[rt] = 1;
				} else reg[rt] = 0;
			}
		} else if ( op ==0x2b) { //sw, offset 4의 배수만
			plus = imm/4;
			add_s = reg[rs] + 4*plus;
			datamem [add_s] = reg[rt];	
		} else if ( op ==0x28){ // sb;
			plus = imm/4;
			plus_off = imm%4;
			add_s = reg[rs] + 4*plus;
			ret1 = addfind(add_s);

			if ( plus ==0) ret2 = ((ret1&0x00ffffff) | reg[rt] << 24);
                        else if (plus ==1) ret2 = ((ret1 & (int)0xff00ffff) | reg[rt] << 16);
                        else if (plus ==2) ret2 = ((ret1 & (int)0xffff00ff) | reg[rt] << 8);
                        else if (plus ==3) ret2 = ret1 & (int)0xffffff00 | reg[rt];
			
			datamem[add_s] = ret2;
		}
	}
	
}

int addfind(int add){
        for (auto iter=datamem.begin() ; iter!=datamem.end(); iter ++){
		if (iter->first == add){
                    	return iter->second;
		  }	
      	}	
      return 0;
}

void printd1(){
   cout << "Registers:" <<endl;
   for(int i = 0 ; i <32 ; i++){
           cout << "R"<<dec<<i<<": 0x"<<hex<< reg[i]<<endl;
   }
   cout << " "  << endl;
}

void printd2(){     
    cout << " " << endl;
    cout <<"Memory content [" <<"0x"<<hex<<add1 << ".." << "0x"<<hex<<add2 <<"]:"<<endl;
    cout <<"--------------------------------------------" << endl;

    // -m 주소가 text or data 그리고 범위 벗어나는지 확인해라(add1부터 벗어나는지 add2만 이상한지)
	if (textmem.count(add1)){
		map<int32_t, int>::iterator iter = textmem.find(add1);
		map<int32_t, int>::iterator iter2 = textmem.find(add2);
		int check_add1 = pcstart+textcnt*4;

		for ( auto it=iter; it != textmem.end() ; it++){
			cout << "0x"<<hex<<it->first<<": 0x"<<hex<<it->second<<endl;
			if ( it == iter2) break;
		}

		if( add2>=check_add1) { 
		    int num = (add2 - check_add1)/4;
		    for ( int i=0; i<num+1 ;i++){
			cout << "0x"<<hex<<check_add1+4*i<< ": 0x0"<<endl;
		    }
		}
	} else if (datamem.count(add1)){
		map<int32_t,int>::iterator iter =  datamem.find(add1);
		map<int32_t,int>::iterator iter2 =  datamem.find(add2);
		int check_add2 = datastart+datacnt*4;

		for (auto it=iter; it != datamem.end(); it++){
			cout <<"0x"<<hex<<it->first<<": 0x"<<hex<<it->second<<endl;
			if (it == iter2) break;
		}

		if (add2>=check_add2){
			int num = (add2 - check_add2)/4;
			for ( int i=0; i<num+1 ;i++){
				cout << "0x"<<hex<<check_add2+4*i<<": 0x0"<<endl;
			}
		}
	} else{
		int num = (add2 - add1)/4;
		for ( int i=0; i<num+1; i++){
			cout <<"0x"<<hex<<add1+4*i<<": 0x0"<<endl;
		}
      }
	cout << " "  << endl;
}
	 
