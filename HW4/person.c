#include <stdio.h>
#include "person.h"
#include<stdlib.h>
#include<string.h>


void check_delete_list(FILE *);
void print_all(FILE *);

//./a.out d person.dat 0987654321123;./a.out d person.dat 1234567890123;./a.out d person.dat 444444444444;./a.out d person.dat 8811032129018
//./a.out a person.dat "8811032129018" "GD Hong" "23" "Seoul" "02-820-0924" "gdhong@ssu.ac.kr";./a.out a person.dat "1234567890123" "Choi SS" "25" "DongJak Seoul" "3632-2195" "frics@soongsil.ac.kr";./a.out a person.dat "0987654321123" "DG KIM" "24" "SangDoDong Seoul" "2333-4444-5555" "dong9706@ssu.ac.kr";./a.out a person.dat "444444444444" "Fuck Park" "56" "China BeiJing" "4444-4444" "deadchinese@ch.com"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음
#define HEADER_RECORD 16
#define RECORD_PER_PAGE (HEADER_AREA_SIZE-4)/8
typedef struct _Header{
	int page_cnt;
	int record_cnt;
	int d_record_num;
	int d_page_num;
}Header;

Header header_record;
// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 위의 readPage() 함수를 호출하여 pagebuf에 저장한 후, 여기에 필요에 따라서 새로운 레코드를 저장하거나
// 삭제 레코드 관리를 위한 메타데이터를 저장합니다. 그리고 난 후 writePage() 함수를 호출하여 수정된 pagebuf를
// 레코드 파일에 저장합니다. 반드시 페이지 단위로 읽거나 써야 합니다.
//
// 주의: 데이터 페이지로부터 레코드(삭제 레코드 포함)를 읽거나 쓸 때 페이지 단위로 I/O를 처리해야 하지만,
// 헤더 레코드의 메타데이터를 저장하거나 수정하는 경우 페이지 단위로 처리하지 않고 직접 레코드 파일을 접근해서 처리한다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{	
	fseek(fp, HEADER_RECORD + PAGE_SIZE * pagenum, SEEK_SET);
	fread((void *)pagebuf, PAGE_SIZE, 1, fp);	
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 레코드 파일의 위치에 저장한다. 
// 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//

void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, HEADER_RECORD + PAGE_SIZE * pagenum, SEEK_SET);
	fwrite((void *)pagebuf, PAGE_SIZE, 1, fp);
}

//
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 
// 
void pack(char *recordbuf, const Person *p)
{
	sprintf(recordbuf, "%s#%s#%s#%s#%s#%s#", p->id, p->name, p->age, p->addr, p->phone, p->email);
	printf("PACK : %s\n", recordbuf);
}

// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다.
//
void unpack(const char *recordbuf, Person *p)
{
	sscanf(recordbuf, "%[^#]%[^#]%[^#]%[^#]%[^#]%[^#]", p->id, p->name, p->age, p->addr, p->phone, p->email);
}

void writeHeader(FILE *fp){
	fseek(fp, 0, SEEK_SET);
	fwrite((void *)&header_record, HEADER_RECORD, 1, fp);
}
//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값들을 구조체에 저장한 후 아래 함수를 호출한다.
//
void alloc_page(FILE *fp, char *recordbuf, int page_num){
	char *pagebuf = malloc(sizeof(char)*PAGE_SIZE);
	char *header_area = malloc(sizeof(char)*HEADER_AREA_SIZE);
	int record_cnt = 1, offset = 0, len = strlen(recordbuf);
	
	//header area 정보 입력
	memcpy(header_area, &record_cnt, sizeof(int));
	memcpy(header_area+4, &offset, sizeof(int));
	memcpy(header_area+8, &len, sizeof(int));
	memcpy(pagebuf, header_area, HEADER_AREA_SIZE);
	//recordbuf 저장 및 page 쓰기
	memcpy(pagebuf+HEADER_AREA_SIZE, recordbuf, len);
	writePage(fp, pagebuf, page_num);

	//header_record update
	header_record.page_cnt++;
	header_record.record_cnt++;
	writeHeader(fp);
}

void add(FILE *fp, const Person *p)
{
	//1. 삭제 데이터가 존재하는 지 확인
	//삭제 데이터가 존재하면 right size를 확인하여 first fit 전략을 사용하여 record 추가(삭제된 record의 크기만 고려하면 된다.)
	//2. 삭제된 데이터가 없다면 마지막 데이터 페이지에 append하여 저장한다. 

	char *pagebuf = malloc(sizeof(char)*PAGE_SIZE);
	char *recordbuf = malloc(sizeof(char)*MAX_RECORD_SIZE);
	void *header_area = malloc(HEADER_AREA_SIZE);
	int record_cnt;
	int page_num = 0, record_num = 0;
	int offset, len;

	pack(recordbuf, p);
	//삭제 레코드가 존재하는 지 확인한다. 
	if(header_record.d_page_num != -1){
		int prev[2] = {0, 0};
		int cur[2];
		cur[0] = header_record.d_page_num;
		cur[1] = header_record.d_record_num;
		int d_page_num = header_record.d_page_num;
		//삭제된 record number는 header area에서 offset 및 length를 확인하기 위해
		//header area에서의 record counut크기 4bytes를 더하고 
		//한 record의 offset, length 저장을 위한 각 4bytes를 고려하여 8byte를 곱한다
		int d_record_num = header_record.d_record_num;
		int pos;
		char *delete_record;
		do{
			//삭제된 레코드 확인
			pos = cur[1]*8+4;

			readPage(fp, pagebuf, cur[0]);
			memcpy(header_area, pagebuf, HEADER_AREA_SIZE);
			memcpy(&offset, header_area+pos, sizeof(offset));
			memcpy(&len, header_area+pos+4, sizeof(len));

			delete_record = malloc(sizeof(char)*len);
			memcpy(delete_record, pagebuf+HEADER_AREA_SIZE+offset, len);
	
		
			if(len >= strlen(recordbuf)){
				int next[2];
				// printf("현재 위치!!!\n");
				// printf("PREV: %d, %d (직전 위치, 이거의 값을 current의 리스트 값으로 바꿔야한다, 그게 current값이긴 해) \n", prev[0], prev[1]);
				// printf("CURRENT : %d, %d (새로 삽입할 페이지, 레코드 번호)\n", cur[0], cur[1]);
				// printf("HEAD : %d, %d\n", header_record.d_page_num, header_record.d_record_num);
				// printf("write offset : %d\n", offset);
				memcpy(pagebuf+HEADER_AREA_SIZE+offset, recordbuf, len);
				writePage(fp, pagebuf, cur[0]);
				//list 수정
				memcpy(next, delete_record+1, sizeof(int)*2);
				// printf("NEXT : %d, %d (요걸 prev에 쓰면 될듯.)\n", cur[0], cur[1]);
				if(header_record.d_page_num != cur[0] || header_record.d_record_num != cur[1]){
					
					//printf("중간 업데이트\n");
					free(delete_record);
					//prev[1]은 record번호
					//해당하는 래코드의 offset, len을 받아 올 수 있다
					pos = prev[1]*8+4;
					readPage(fp, pagebuf, prev[0]);
					//prev의 정보 받아오기(next의 값을 쓴다.)
					memcpy(header_area, pagebuf, HEADER_AREA_SIZE);
					memcpy(&offset, header_area+pos, sizeof(offset));
					memcpy(&len, header_area+pos+4, sizeof(len));

					delete_record = malloc(sizeof(char)*len);
					memcpy(delete_record, pagebuf+HEADER_AREA_SIZE+offset, len);
					memcpy(delete_record+1, &next, sizeof(int)*2);

					memcpy(pagebuf+HEADER_AREA_SIZE+offset, delete_record, len);

					writePage(fp, pagebuf, prev[0]);

				}else{
					//printf("해드 업데이트\n");
					header_record.d_page_num = next[0];
					header_record.d_record_num = next[1];
					writeHeader(fp);
				}
				return;
			}
			//prev에 현재 페이지, 레코드 번호 저장
			memcpy(prev, cur, sizeof(int)*2);
			//다음 확인 리스트 확인
			memcpy(cur, delete_record+1, sizeof(int)*2);

			free(delete_record);
			
		}while(cur[0] != -1);//해당 위치에 recordbuf를 쓸 수 있을 때
		//삭제 데이터가 없으면 마지막에 append
	}
	
	//append할 page number 계산
	if(header_record.page_cnt > 0){
		page_num = header_record.page_cnt-1;
		readPage(fp, pagebuf, page_num);
	
		memcpy(header_area, pagebuf, HEADER_AREA_SIZE);
		memcpy(&record_cnt, header_area, sizeof(int));

		record_num = (record_cnt-1)*8+4;
		memcpy(&offset, header_area+record_num, sizeof(int));
		memcpy(&len, header_area+record_num+4, sizeof(int));
		// printf("offset : %d\n", offset);
		// printf("len : %d\n", len);

		int remain = DATA_AREA_SIZE-len;
		// printf("remain : %d\n", remain);
		//현재 페이지에 쓸 수 있을 때
		if(remain >= strlen(recordbuf) && record_cnt < RECORD_PER_PAGE){
			//printf("APPENDING EXIST PAGE\n");
			//offset, length 계산 및 저장
			record_cnt > 0 ? offset += len : 0;
			len = strlen(recordbuf);
			//printf("offset : %d , len : %d\n", offset, len);
			//header area update
			memcpy(header_area+4+(record_cnt*8), &offset, sizeof(int));
			memcpy(header_area+4+(record_cnt*8)+4, &len, sizeof(int));
			record_cnt++;
			memcpy(header_area, &record_cnt, sizeof(int));
			memcpy(pagebuf, header_area, HEADER_AREA_SIZE);
			//reocordbuf write
			memcpy(pagebuf+HEADER_AREA_SIZE+offset, recordbuf, len);
			writePage(fp, pagebuf, page_num);
			//header record update
			header_record.record_cnt++;
			writeHeader(fp);

			//printf("RECORD ADDED : %s\n", recordbuf);
			
		}else//새로운 페이지 alloc
			alloc_page(fp, recordbuf, page_num+1);

	}else{
		alloc_page(fp, recordbuf, 0);
	}
}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *id)
{
	//printf("START DELETE\n");
	char *pagebuf = malloc(sizeof(char)*PAGE_SIZE);
	void *header_area = malloc(HEADER_AREA_SIZE);
	char *recordbuf;
	Person tmp;
	int record_cnt;
	int offset, len;

	//전체 페이지 탐색
	for(int i=0; i<header_record.page_cnt; i++){
		//페이지 read
		readPage(fp, pagebuf, i);
		//현재 페이지의 header area 정보를 read한다.
		memcpy(header_area, pagebuf, HEADER_AREA_SIZE);
		//record 갯수 확인 
		memcpy(&record_cnt, header_area, sizeof(int));
		//printf("record cnt : %d\n", record_cnt);
		//현재 페이지에 record 개수만큼 반복문 수행
		for(int j=0; j<record_cnt; j++){
	//		printf("check page num : %d, record num : %d\n", i, j);
			//각 레코드의 offset, length의 position
			int pos = j*8+4;
			memcpy(&offset, header_area+pos, sizeof(int));
			memcpy(&len, header_area+pos+4, sizeof(int));
		//	printf("offset : %d, len : %d\n" ,offset, len);
			recordbuf = (char *)malloc(sizeof(char)*len);
			//recordbuf의 위치는 header area의 크기 + offset이다.
			memcpy(recordbuf, pagebuf+HEADER_AREA_SIZE+offset, len);
			//이미 삭제된 레코드의 경우 다음 레코드 탐색
			if(recordbuf[0] == '*'){
				free(recordbuf);
				continue;
			}
			//id값 확인을 위해 recordbuf의 내용을 unpack한다. 
			unpack(recordbuf, &tmp);
			//printf("%s\t%s\n", tmp.id, id);
			//해당하는 아이디를 찾았을 경우
			if(strcmp(tmp.id, id) == 0){
				//printf("FIND RECORD TO DELETE\n");
				//record에 삭제 기록
				recordbuf[0] = '*';

				memcpy(recordbuf+1, &header_record.d_page_num, sizeof(int));
				memcpy(recordbuf+1+sizeof(int), &header_record.d_record_num, sizeof(int));
				//header_record 수정 후 파일에 쓰기
				header_record.d_page_num = i;
				header_record.d_record_num = j;
				writeHeader(fp);
				//pagebuf에 삭제 기록을 하고 다시 파일에 write
				memcpy(pagebuf+HEADER_AREA_SIZE+offset, recordbuf, len);
				writePage(fp, pagebuf, i);
				free(header_area);
				free(recordbuf);
				free(pagebuf);
				
				return;
			}
			free(recordbuf);
			
		}
	}
	free(pagebuf);
	free(header_area);
}

int main(int argc, char *argv[])
{
	FILE *fp; // 레코드 파일의 파일 포인터
	Person input;
	char option;
	char *recordbuf = malloc(sizeof(char)*MAX_RECORD_SIZE);

	if((fp = fopen(argv[2], "r+b")) == NULL){
		fp = fopen(argv[2], "w+b");
		header_record.page_cnt = 0;
		header_record.record_cnt = 0;
		header_record.d_page_num = -1;
		header_record.d_record_num = -1;
		fwrite(&header_record, HEADER_RECORD, 1, fp);
		
	}else{
		fread(&header_record, HEADER_RECORD, 1, fp);
	//	print_header();
	//	printf("p_cnt : %d\nr_cnt : %d\nd_p_n : %d\nd_r_n : %d\n", header_record.page_cnt, header_record.record_cnt, header_record.d_page_num, header_record.d_record_num);
	}
	
	if(argc > 3 ){
		if(strlen(argv[1]) != 1){
			fprintf(stderr, "Invalid Option\n");
			exit(1);
		}
		option = argv[1][0];
		
		if(option == 'a'){ //record add
			strcpy(input.id, argv[3]);
			strcpy(input.name, argv[4]);
			strcpy(input.age, argv[5]);
			strcpy(input.addr, argv[6]);
			strcpy(input.phone, argv[7]);
			strcpy(input.email, argv[8]);
			add(fp, &input);
	
		}else if(option == 'd'){ //record delete
			delete(fp, argv[3]);
			check_delete_list(fp);
		}else{
			fprintf(stderr, "Invalid Option\n");
			exit(1);
		}
		
	}else{
		fprintf(stderr, "NEED MORE PARAMETER.\n");
		exit(1);

	}
	print_all(fp);

	return 0;
}




void check_delete_list(FILE *fp){

	printf("CHECK DELETE LIST\n");
	char *pagebuf = malloc(sizeof(char)*PAGE_SIZE);
	void *header_area = malloc(HEADER_AREA_SIZE);
	char *recordbuf;
	
	int record_cnt;
	int offset, len;
	//전체 페이지 탐색
	int p = header_record.d_page_num, d = header_record.d_record_num;
	while(p != -1){
		readPage(fp, pagebuf, p);
		memcpy(header_area, pagebuf, HEADER_AREA_SIZE);
		//record 갯수 확인 
		int pos = d*8+4;
		memcpy(&offset, header_area+pos, sizeof(int));
		memcpy(&len, header_area+pos+4, sizeof(int));
		recordbuf = (char *)malloc(sizeof(char)*len);
		memcpy(recordbuf, pagebuf+HEADER_AREA_SIZE+offset, len);
		printf("CHECK link page : %d, record : %d, length : %d\n", p, d, len);
		memcpy(&p, recordbuf+1, sizeof(int));
		memcpy(&d, recordbuf+5, sizeof(int));

	}
	printf("CHECK link page : %d, record : %d\n", p, d);


}

void print_header(){
	printf("\nHEAEDER_RECORD\n");
	printf("pages : %d\nrecord : %d\ncur_delete_page : %d\ncur_delete_record : %d\n\n\n", header_record.page_cnt, header_record.record_cnt, header_record.d_page_num, header_record.d_record_num);
}

void print_all(FILE *fp){
	

	char *pagebuf = malloc(sizeof(char)*PAGE_SIZE);
	void *header_area = malloc(HEADER_AREA_SIZE);
	char *recordbuf;
	Person tmp;
	int record_cnt;
	int offset, len;

	print_header();
	//전체 페이지 탐색
	for(int i=0; i<header_record.page_cnt; i++){
		//페이지 read
		
		readPage(fp, pagebuf, i);
		//현재 페이지의 header area 정보를 read한다.
		memcpy(header_area, pagebuf, HEADER_AREA_SIZE);
		//record 갯수 확인 
		memcpy(&record_cnt, header_area, sizeof(int));
		printf("PAGE[%d]\n", i);
		printf("RECORD COUNT : %d\n", record_cnt);
		
		for(int i=0; i<record_cnt; i++){
			int a[2];
			memcpy(&a, header_area+4+8*i, sizeof(int)*2);
			printf("record[%d] offset : %3d, length : %3d\n", i,  a[0], a[1]);
		}
		
		for(int j=0; j<record_cnt; j++){
			int pos = j*8+4;
			memcpy(&offset, header_area+pos, sizeof(int));
			memcpy(&len, header_area+pos+4, sizeof(int));
			recordbuf = (char *)malloc(sizeof(char)*len);
			//recordbuf의 위치는 header area의 크기 + offset이다.
			memcpy(recordbuf, pagebuf+HEADER_AREA_SIZE+offset, len);
			//이미 삭제된 레코드의 경우 다음 레코드 탐색
			if(recordbuf[0] == '*'){
				free(recordbuf);
				continue;
			}
			//id값 확인을 위해 recordbuf의 내용을 unpack한다. 
			printf("[%d] %s\n", j, recordbuf);
		}

	}
	free(pagebuf);
	free(header_area);
}
