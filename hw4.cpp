#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <iterator>
#include <string>  
#include <random>
#include <iostream>
#include <list>
#include <algorithm>
#include <cstring>
#include <memory.h>

using namespace std;

struct check {
	string action;
	unsigned long long add;
	int tag;
	int set;
};

struct aboutcache {
	int tag;
	int cnt=0;
	int set;
	int drtbit;
};


bool argv_lru = false;
bool argv_rnd= false; 
int level1_cpy = 0;
int level1_asc = 0;
int level2_cpy = 0;
int level2_asc = 0;
int block_size = 0;

void action_level1(check newadd);
void action_level2(check newadd);
check make_address(string action, string realadd);


bool level1_hit = false;
bool level2_hit = false;
aboutcache** level1;
aboutcache** level2;

int total_access = 0; 
int read_access = 0;
int write_access = 0;
int cale1_rmiss = 0; 
int cale1_wmiss = 0;
int cale2_rmiss = 0;
int cale2_wmiss = 0;
int L1_clean_eviction = 0;
int L1_dirty_eviction = 0;
int L2_clean_eviction = 0;
int L2_dirty_eviction = 0;
int cale2_read = 0;
int cale2_write = 0;


int cnt =0;
int cnt2=0;
int blknum1 = 0;
int blknum2 = 0;
int set1_num = 0;
int set2_num = 0;
int pt =0;

int main(int argc, char* argv[]) {


	ifstream readf;
	readf.open(argv[argc - 1]);

	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0) {
			char str[50];
			strcpy(str,argv[i+1]);
			level2_cpy = stoul(str,NULL,10);
			if (level2_cpy >=4) level1_cpy = level2_cpy/4;
			else if (level2_cpy <=2) level1_cpy = level2_cpy;
		} else if (strcmp(argv[i], "-a") == 0) {
			char str[50];
            strcpy(str,argv[i+1]);
            level2_asc = stoul(str,NULL,10);
            if (level2_asc >=4) level1_asc = level2_asc/4;
            else if (level2_asc <=2) level1_asc = level2_asc;
		} else if (strcmp(argv[i], "-b") == 0) {
			char str[50];
			strcpy(str, argv[i + 1]);
			block_size = stoul(str, NULL, 10);
		} else if (strcmp(argv[i], "-lru") == 0) {
			argv_lru =true;
		} else if (strcmp(argv[i], "-random") == 0) {
			argv_rnd = true;
		}
	}


	set1_num = level1_cpy * 1024 / (block_size *level1_asc);
	set2_num = level2_cpy * 1024 /(block_size *level2_asc);
	blknum1 = level1_cpy*1024 / block_size;
	blknum2 = level2_cpy*1024 / block_size;
		
		
	level1 = new aboutcache*[set1_num];

	for (int i = 0; i < set1_num; i++) {
		level1[i] = new aboutcache[level1_asc];
		memset(level1[i], 0, sizeof(aboutcache)*level1_asc);
	}

	level2 = new aboutcache*[set2_num];

	for (int i = 0; i < set2_num; i++) {
		level2[i] = new aboutcache[level2_asc];
		memset(level2[i], 0, sizeof(aboutcache)*level2_asc);
	}

	string line;

	if (readf.is_open()) {		
		while (readf.eof() != true) {
			getline(readf, line);	
			if(line.size() < 2) break; // 이거 2보다 작으면 안되는게 있는듯함,, 자꾸 terminate 뜸
			string action = line.substr(0,1);
			string realadd = line.substr(2,line.size()-1);
			action_level1(make_address(action,realadd));
		 }
	}

	char trfile[30];
   	strcpy(trfile,argv[argc-1]);

	char * filename = strtok(trfile, ".");
    char exten[2] = "_";
    strcat(filename,exten);
	string exten1 = to_string(level2_cpy);
	char const *exten11 = exten1.c_str();
	strcat(filename,exten11);
	char exten2[2] = "_";
    strcat(filename,exten2);
	string exten3 = to_string(level2_asc);
    char const * exten33 = exten3.c_str();
    strcat(filename,exten33);
	char exten4[2] = "_";
    strcat(filename,exten4);
	string exten5 = to_string(block_size);
    char const * exten55 = exten5.c_str();
    strcat(filename,exten55);
	char exten6[5] = ".out";
    strcat(filename,exten6);

	ofstream output(filename);
	
	output<< "-- General Stats --" << endl;
	output<< "L1 Capacity:  " << level1_cpy << endl;
	output<< "L1 way:  " << level1_asc << endl;
	output<< "L2 Capacity:  " << level2_cpy << endl;
	output<< "L2 way:  " << level2_asc << endl;
	output<< "Block Size:      " << block_size << endl;
	output<< "Total accesses:  " << total_access << endl;
	output<< "Read accesses:  " << read_access << endl;
	output<< "Write accesses:  " << write_access << endl;
	output<< "L1 Read misses:  " << cale1_rmiss << endl;
	output<< "L2 Read misses:  " << cale2_rmiss << endl;
	output<< "L1 Write misses:  " << cale1_wmiss << endl;
	output<< "L2 Write misses:  " << cale2_wmiss << endl;
	output<< "L1 Read miss rate:  " << ((double)cale1_rmiss / (double)read_access)*100<<"%"<<endl;
	output<< "L2 Read miss rate:  " << ((double)cale2_rmiss / (double)cale2_read) *100 <<"%"<< endl;
	output<< "L1 Write miss rate:  " << ((double)cale1_wmiss / (double)write_access)*100<<"%"<< endl;
	output<< "L2 Write miss rate:  " << ((double)cale2_wmiss / (double)cale2_write) *100 <<"%"<< endl;
	output<< "L1 Clean evictionion:  " << L1_clean_eviction << endl;
	output<< "L2 Clean evictionion:  " << L2_clean_eviction << endl;
	output<< "L1 Dirty evictionion:  " << L1_dirty_eviction << endl;
	output<< "L2 Dirty evictionion:  " << L2_dirty_eviction << endl;
	
	for (int i = 0; i < set1_num; i++) {
		delete[] level1[i];
	}
	delete[] level1;
	

	for (int i = 0; i < set2_num;i++) {
		delete[] level2[i];
	}
	delete[] level2;

	output.close();
	readf.close();
    return 0;
}


check make_address(string action, string realadd) {
        check newadd ;
    	newadd.action = action;
    	newadd.add = stoull(realadd,NULL,16);

    	int set_bits = log(set1_num)/log(2);
    	int blc_bits = log(block_size)/log(2);
    	int sum_blcset = set_bits + blc_bits;
    	newadd.tag = newadd.add >> sum_blcset;

    	int tag_bits = 64 - sum_blcset;
    	newadd.set = (newadd.add >> blc_bits) << (tag_bits+blc_bits) >> (tag_bits + blc_bits);
        return newadd;

}

void action_level1(check newadd) {
	
	int rndindex = 0;
	int cngvalue=0;
	int cngcnt=0;
	int useblk =0;

	aboutcache* nlevel1 = new aboutcache;
	nlevel1->set = newadd.set;
    nlevel1->tag = newadd.tag;

	 if (newadd.action == "R") {
                nlevel1->drtbit = 0;
                read_access++;
                total_access++;
        }

	 else if (newadd.action == "W") {
                nlevel1->drtbit = 1;
                write_access++;
                total_access++;
        }


	for (int i = 0; i < level1_asc; i++) {
		if (level1[nlevel1->set][i].cnt != 0) useblk++;
	}

	
	for (int i = 0; i < level1_asc; i++) {
		if (level1[nlevel1->set][i].tag == nlevel1->tag) {
			cnt ++;
			//nlevel1->cnt = cnt;
			level1_hit = true;
			level1[nlevel1->set][i].cnt = cnt;
			break;
		} else level1_hit = false;
	}

	if (level1_hit == false) {
		if (newadd.action == "R") cale1_rmiss++;
		else if (newadd.action == "W") cale1_wmiss++;

		action_level2(newadd);

		if (useblk == level1_asc){
			if(argv_lru){
				cngvalue = level1[nlevel1->set][level1_asc-1].cnt;
				cngcnt = level1_asc-1;
			  	for (int i = level1_asc-1; i>=0; i--){
                                   if (level1[nlevel1->set][i].cnt < cngvalue) {
                                        cngvalue = level1[nlevel1->set][i].cnt;
                                        cngcnt =i;
                              	  }
                     	   }

                    cnt ++;
                    nlevel1->cnt = cnt;
                    level1[nlevel1->set][cngcnt] = *nlevel1;

                    if (level1[nlevel1->set][cngcnt].drtbit == 1)L1_dirty_eviction++;
                    else if (level1[nlevel1->set][cngcnt].drtbit == 0)L1_clean_eviction++;

            }
			else if (argv_rnd){
                if(level1_asc==1){
                        cnt ++;
                        nlevel1->cnt = cnt;
                        level1[nlevel1->set][0] = *nlevel1;

                        if (level1[nlevel1->set][0].drtbit == 1)L1_dirty_eviction++;
                        else if (level1[nlevel1->set][0].drtbit == 0)L1_clean_eviction++;
			   } else {
				   srand((unsigned int)time(NULL));
				   rndindex = rand() % (level1_asc-1);

			   	   cnt ++;
			   	   nlevel1->cnt = cnt;
			   	   level1[nlevel1->set][rndindex] = *nlevel1;

                   if (level1[nlevel1->set][rndindex].drtbit == 1)L1_dirty_eviction++;
                   else if (level1[nlevel1->set][rndindex].drtbit == 0)L1_clean_eviction++;
			   }
			}
		} else{
                        cnt ++;
                        nlevel1->cnt = cnt;
                        level1[nlevel1->set][useblk] = *nlevel1;
                }

		}
}


void action_level2(check newadd) {

	int rndindex = 0;
	int cngcnt =0;
	int cngvalue=0;
	int useblk =0;

	aboutcache* nlevel2 = new  aboutcache;
	nlevel2->set = newadd.set;
	nlevel2->tag = newadd.tag;

	if (newadd.action == "R") {
                nlevel2->drtbit = 0;
                cale2_read++;
        }


	else if (newadd.action == "W") {
		nlevel2->drtbit = 1;
		cale2_write++;
	}

	for (int i = 0; i < level2_asc; i++) {
		if (level2[nlevel2->set][i].cnt != 0) useblk++;
	}


	for (int i = 0; i < level2_asc; i++) {
		if (level2[nlevel2->set][i].tag == nlevel2->tag) {
			level2_hit = true;
			cnt2 ++;
			level2[nlevel2->set][i].cnt = cnt2;
			//nlevel2->cnt = cnt;
			//cnt ++;
			break;
		} else level2_hit = false;
	}

	if (level2_hit == false) {
		if (newadd.action == "W")cale2_wmiss++;
        	else if (newadd.action == "R")cale2_rmiss++;


		if (useblk == level2_asc){
		   if (argv_lru){
			cngvalue = level2[nlevel2->set][level2_asc-1].cnt;
			cngcnt = level2_asc-1;
			for (int i = level2_asc-1; i>=0; i--){
                    if (level2[nlevel2->set][i].cnt < cngvalue) {
                            cngvalue = level2[nlevel2->set][i].cnt;
                            cngcnt =i;
                    }
            }

                cnt2 ++;
                nlevel2->cnt = cnt2;
                level2[nlevel2->set][cngcnt] = *nlevel2;
            
                if (level2[nlevel2->set][cngcnt].drtbit == 1)L2_dirty_eviction++;
                else if (level2[nlevel2->set][cngcnt].drtbit == 0)L2_clean_eviction++;

          }

		  else if (argv_rnd){
			if(level2_asc==1){
				cnt2 ++;
                nlevel2->cnt = cnt2;
                level2[nlevel2->set][0] = *nlevel2;
				if (level2[nlevel2->set][0].drtbit == 1)L2_dirty_eviction++;
                else if (level2[nlevel2->set][0].drtbit == 0)L2_clean_eviction++;
			} else {
				srand((unsigned int)time(NULL));
				rndindex = rand() % (level2_asc-1);
				cnt2 ++;
                nlevel2->cnt = cnt2;
				level2[nlevel2->set][rndindex] = *nlevel2;
				if (level2[nlevel2->set][rndindex].drtbit == 1)L2_dirty_eviction++;
                else if (level2[nlevel2->set][rndindex].drtbit == 0)L2_clean_eviction++;
			}

		}
	} else {
		cnt2 ++;
		nlevel2->cnt = cnt2;
		level2[nlevel2->set][useblk] = *nlevel2;
	}


	}
}




