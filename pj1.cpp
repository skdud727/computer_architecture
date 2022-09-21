
#include <stdio.h>
#include <bitset>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <ctype.h>
#include <map>
#include <sstream>
#include <cctype>

using namespace std;


void textlabeldic(char * sample);
void datasection(string line);
void textsection(string line);
void instobinary(vector<string> t1, int inssize);
int labelfind(string label);
void bintohexa(int32_t num,string ins);

string labelname;
int datasize = 0;
int textsize = 0;
int inssize =0;


map<string,int32_t> labeldic;
vector<long> value;
vector<int32_t> hexa;

int main(int argc, const char * argv[]) {

	
    char sample[50];
    strcpy(sample,argv[1]);

    ifstream readf(sample);
    
    string line;
    textlabeldic(sample);


    if ( readf.is_open()){
    	while (!readf.eof()){
        getline(readf,line);
        char str[100];
        strcpy(str,line.c_str());
    
	if(strstr(str, ".data")!=NULL){
		continue;
	}

	else if(strstr(str, ".word")!=NULL){
		datasection(line);
		continue;
	}
	else if (strstr(str,".text") != NULL){
		continue;
	}
	else { 
		textsection(line);
	}
    	}        
   }
    	readf.close();
	
    char * filename = strtok(sample, ".");
    char exten[5] = ".o";
    strcat(filename,exten);

    ofstream writef;
    writef.open(filename);

    if (writef.is_open()){
	writef << "0x" <<hex <<textsize*4<<"\n";
	writef << "0x" << hex << datasize*4 << "\n";

	for (int j=0; j<hexa.size(); j++){
		writef <<"0x" <<hex << hexa[j] <<"\n";
	}

	for( int i=0; i<value.size(); i++){
		writef << "0x" <<hex<<value[i] << "\n";
	}
   }
   writef.close();
	
}


// count textsize and assign address
void textlabeldic(char * sample){
	ifstream readf(sample);
   	string line;

	while(!readf.eof()){
		getline(readf,line);
		char str[100];
		strcpy(str,line.c_str());
		if (strlen(str)!=0){
		  if (strstr(str,".data") == NULL){
		       if (strstr(str,".word")==NULL){
			      if (strstr(str,".text") == NULL){
				if (strstr(str, ":")!=NULL){
					int strl = strlen(str);
                			char *label=strtok(str,":");
					int labell = strlen(label);
					labelname = label;
                			labeldic[labelname]=0x00400000 + textsize*4;
					if (strl > labell+2){
						textsize++;
					}
				} else {
					textsize++;
					
				}

			      }
		       }	       
		  }
		}
	}
}


// make datasection vector
void datasection(string line){
	istringstream ss(line);
	string source;
	char data[20];

	while(getline(ss, source, ' ')){
		strcpy(data,source.c_str());
		if(strlen(data)!=0){
			if (strstr(data, ":")!=NULL){
				char *label = strtok(data,":");
				labelname = label;
				labeldic[labelname] = 0x10000000+datasize*4;
			} else {
			  	if (atoi(data)!=0){
					value.push_back(atoi(data));
					datasize++;
				}else if ( strstr(data,"0x")!=NULL){
					long hexa=strtol(data, NULL, 16);
					value.push_back(hexa);
					datasize++;
				}
			}
		}
	}

}


// make textsection vector
void textsection(string line){
	istringstream ss(line);
	string source;
	char data[20];
	string inst;
	vector<string> t1;

	while(getline(ss,source, ' ')){
		strcpy(data,source.c_str());
		if(strlen(data)!=0){
			if (strstr(data,":")!=NULL){
				continue;
			} else{
				if (strstr(data,"(") != NULL){
					char * off = strtok(data,"(");
					char * off2 = strtok(NULL, "$)");
					t1.push_back(off);
					t1.push_back(off2);
				}else if (strstr(data,"$")!=NULL){
					char * datai = strtok(data, "$,");
					t1.push_back(datai);
				} else {
					t1.push_back(data);
				}
           		}
		}
	}
	instobinary(t1, inssize);
	if(t1.size()!=0){
		inssize++;
	}
}


// make binary instruction information
void  instobinary(vector<string> t1, int inssize){
	int32_t num = 0;
	int32_t num1 = 0;
	string ins0;
	string ins1;
	string ins2;
	string ins3;
	
	int insize = 0;

	if (t1.size()!=0){
		ins0 = t1[0];
		ins1 = t1[1];
		if (t1.size()>2){
			ins2 = t1[2];
		}
		if (t1.size()>3){
			ins3 =t1[3];
		}


	if ( ins0 == "la"){
		int add = labelfind(ins2);
        //need to compare with only lower 16bit
		if ( (add & 65535)!=0){
			num1=(0xf<<26)|(0<<21)|(stoi(ins1)<<16)|(add>>16);
			num= (0xd<<26)| (stoi(ins1)<<21) | (stoi(ins1)<<16) | (add & 65535);
			textsize++;
			bintohexa(num1,ins0);
		} else {
		num= (0xf<<26)| (0<<21) | (stoi(ins1)<<16) | (add>>16);
		}
	}	
	else if ( ins0 == "addiu"){
		char ch[20];
		strcpy(ch, ins3.c_str());
		if (strstr(ch,"0x")!=NULL){
			long hexa=strtol(ch, NULL, 16);
			num =(9<<26) | (stoi(ins2)<<21) | (stoi(ins1)<<16) |hexa;
		}else if (strstr(ch,"-")!=NULL){
			int imm = stoi(ins3);
			imm = 65535 & imm;
			num =(9<<26) | (stoi(ins2)<<21) | (stoi(ins1)<<16) | imm;
		} else { num= (9<<26)| (stoi(ins2)<<21) | (stoi(ins1)<<16) | stoi(ins3); }
	} else if (ins0 == "addu"){
		num = (0<<26) | (stoi(ins2)<<21) | (stoi(ins3)<<16) | (stoi(ins1)<<11)| (0<<6) |0x21;
	}
	else if ( ins0 == "and"){	
		num = (0<<26) | (stoi(ins2)<<21) | (stoi(ins3)<<16) | (stoi(ins1)<<11)| (0<<6) |0x24;
	}
	else if ( ins0 == "andi"){
		char ch[20];
		strcpy(ch, ins3.c_str());
		if ( strstr(ch,"0x")!=NULL){
			long hexa=strtol(ch, NULL, 16);
			num= (0xc<<26)| (stoi(ins2)<<21) | (stoi(ins1)<<16) | hexa;
		} else if (strstr(ch,"-")!=NULL){
			int imm = stoi(ins3);
			imm = 65535 & imm;
		  	num= (0xc<<26)| (stoi(ins2)<<21) | (stoi(ins1)<<16) | imm; 
		} else { num= (0xc<<26)| (stoi(ins2)<<21) | (stoi(ins1)<<16) | stoi(ins3); } 
	}
	else if ( ins0 == "beq"){
		int add = labelfind(ins3);
		int offset = add - (4 + (0x00400000+ (inssize*4)));
		offset = offset/4;
		offset = 65535 & offset;
		num= (4<<26)| (stoi(ins1)<<21) | (stoi(ins2)<<16) | offset;
	}
	else if ( ins0 == "bne"){
		int add = labelfind(ins3);
		int offset = add - (4 + (0x00400000 + (inssize*4)));
		offset= offset/4;
		offset = 65535 & offset;
		num= (5<<26)| (stoi(ins1)<<21) | (stoi(ins2)<<16) | offset;
	}
	else if ( ins0 == "j"){
		int add =labelfind(ins1);
		add= (add>>2);
		num= (2<<26)| add;
	}
	else if ( ins0 == "jal"){
		int add = labelfind(ins1);
		add = (add>>2);
		num= (3<<26)| add; 
	}
	else if ( ins0 == "jr"){
		num = (0<<26)|(stoi(ins1)<<21)|(0<<6)| 8;
	}
	else if ( ins0 == "lui"){
		char ch[20];
		strcpy(ch, ins2.c_str());
		if ( strstr(ch,"0x")!=NULL){
			long hexa=strtol(ch,  NULL, 16);
			num= (0xf<<26)| (0<<21) | (stoi(ins1)<<16) | hexa;
		} else if (strstr(ch,"-")!=NULL){
			int imm = stoi(ins3);
			imm = 65535 & imm;
			num = (0xf<<26) | (0<<21) |(stoi(ins1)<<16) |imm;
		} else {num= (0xf<<26)| (0<<21) | (stoi(ins1)<<16) | stoi(ins2);}
	}
	
	else if ( ins0 == "lw"){
		num= (0x23<<26)| (stoi(ins3)<<21) | (stoi(ins1)<<16) | stoi(ins2);
	}
	else if ( ins0 == "lb"){
		num= (0x20<<26)| (stoi(ins3)<<21) | (stoi(ins1)<<16) | stoi(ins2);
	}
	else if ( ins0 == "nor"){
		num = (0<<26) | (stoi(ins2)<<21) | (stoi(ins3)<<16) | (stoi(ins1)<<11)| (0<<6) |0x27;
	}
	else if ( ins0 == "or"){
		num = (0<<26) | (stoi(ins2)<<21) | (stoi(ins3)<<16) | (stoi(ins1)<<11)| (0<<6) |0x25;
	}
	else if ( ins0 == "ori"){
		char ch[20];
		strcpy(ch,ins3.c_str());
		if ( strstr(ch,"0x")!=NULL){
			long hexa=strtol(ch, NULL, 16);
			num= (0xd<<26)| (stoi(ins2)<<21) | (stoi(ins1)<<16) | hexa;
		} else if (strstr(ch,"-")!=NULL){
			int imm = stoi(ins3);
			imm = 65535 & imm;
			num = (0xd<<26) |(stoi(ins2)<<21) | (stoi(ins1)<<16) | imm;
		} else { num= (0xd<<26)| (stoi(ins2)<<21) | (stoi(ins1)<<16) | stoi(ins3);} 
	} 
	else if ( ins0 == "sltiu"){
		char ch[20];
		strcpy(ch,ins3.c_str());
		if ( strstr(ch,"0x")!=NULL){
			long hexa=strtol(ch, NULL, 16);
			num= (0xb<<26)| (stoi(ins2)<<21) | (stoi(ins1)<<16) | hexa;
		} else if (strstr(ch,"-")!=NULL){
			int imm = stoi(ins3);
			imm = 65535 & imm;
			num= (0xb<<26)| (stoi(ins2)<<21) | (stoi(ins1)<<16) | imm ;
		} else { num= (0xb<<26)| (stoi(ins2)<<21) | (stoi(ins1)<<16) | stoi(ins3);}
	}
	else if ( ins0 == "sltu"){
		num = (0<<26) | (stoi(ins2)<<21) | (stoi(ins3)<<16) | (stoi(ins1)<<11)| (0<<6) |0x2b;
	}
	else if ( ins0 == "sll"){
		num = (0<<26) | (0<<21) | (stoi(ins2)<<16) | (stoi(ins1)<<11)| (stoi(ins3)<<6) |0;
	}
	else if ( ins0 == "srl"){
		num = (0<<26) | (0<<21) | (stoi(ins2)<<16) | (stoi(ins1)<<11)| (stoi(ins3)<<6) |2;
	}
	else if ( ins0 == "sw"){
		num= (0x2b<<26)| (stoi(ins3)<<21) | (stoi(ins1)<<16) | stoi(ins2);
	}
	else if ( ins0 == "sb"){
		num= (0x28<<26)| (stoi(ins3)<<21) | (stoi(ins1)<<16) | stoi(ins2);
	}
	else if ( ins0 == "subu"){
		num = (0<<26) | (stoi(ins2)<<21) | (stoi(ins3)<<16) | (stoi(ins1)<<11)| (0<<6) |0x23;
	}
	bintohexa(num,ins0);
	}
}


// find label to use for  jump instruction
int labelfind(string label){
      for (auto iter=labeldic.begin() ; iter!=labeldic.end(); iter ++){
	      if (label == iter->first){
		     return iter->second;
	      }
      }      
      return -1;
}



// convert binary32bit instruction to hexademical
void bintohexa(int32_t  num, string ins){
	char binarytotal[40];
	int hexnum = 0;
	string str= bitset<32>(num).to_string();
	strcpy(binarytotal,str.c_str());
	for(int i = 0; i <strlen(binarytotal); i++){
		hexnum= hexnum*2 +binarytotal[i] - '0';
	};
	hexa.push_back(hexnum);

}

