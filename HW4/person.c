#include <stdio.h>
#include "person.h"
#include<stdlib.h>
#include<string.h>

//./a.out a person.dat "8811032129018" "GD Hong" "23" "Seoul" "02-820-0924" "gdhong@ssu.ac.kr"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음
#define HEADER_RECORD 16

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
	
	/*if(sizeof(pagebuf) != PAGE_SIZE){
		fprintf(stderr, "page size error.\n");
		exit(1);
	}*/
	fseek(fp, PAGE_SIZE*pagenum+HEADER_RECORD, SEEK_SET);
	fread((void *)pagebuf, PAGE_SIZE, 1, fp);
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 레코드 파일의 위치에 저장한다. 
// 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//

void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	/*
	if(sizeof(pagebuf) != PAGE_SIZE){
		fprintf(stderr, "page size error.\n");
		exit(1);
	}*/
	fseek(fp, PAGE_SIZE*pagenum+HEADER_RECORD, SEEK_SET);
	fwrite((void *)pagebuf, PAGE_SIZE, 1, fp);
}

//
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 
// 
void pack(char *recordbuf, const Person *p)
{
	int pos = 0;
	//id
	memcpy(recordbuf, p->id, strlen(p->id));
	pos += strlen(p->id);
	memset(recordbuf + pos, '#', 1);
	pos+=1;
	//name
	memcpy(recordbuf + pos, p->name, strlen(p->name));
	pos += strlen(p->name);
	memset(recordbuf + pos, '#', 1);
	pos += 1;
	//age
	memcpy(recordbuf + pos, p->age, strlen(p->age));
	pos += strlen(p->age);
	memset(recordbuf + pos, '#', 1);
	pos += 1;
	//addr
	memcpy(recordbuf + pos, p->addr, strlen(p->addr));
	pos += strlen(p->addr);
	memset(recordbuf + pos, '#', 1);
	pos += 1;
	//phone
	memcpy(recordbuf + pos, p->phone, strlen(p->phone));
	pos += strlen(p->phone);
	memset(recordbuf + pos, '#', 1);
	pos += 1;
	//email
	memcpy(recordbuf + pos, p->email, strlen(p->email));
	pos += strlen(p->email);
	memset(recordbuf + pos, '#', 1);
	

}

// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다.
//
void unpack(const char *recordbuf, Person *p)
{
	printf("unpacking record\n");
	char *ptr = strtok(recordbuf, "#");
	strcpy(p->id, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->name, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->age, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->addr, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->phone, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->email, ptr);

}

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값들을 구조체에 저장한 후 아래 함수를 호출한다.
//
void add(FILE *fp, const Person *p)
{
	//1. 삭제 데이터가 존재하는 지 확인
	//삭제 데이터가 존재하면 right size를 확인하여 first fit 전략을 사용하여 record 추가(삭제된 record의 크기만 고려하면 된다.)
	//2. 삭제된 데이터가 없다면 마지막 데이터 페이지에 append하여 저장한다. 

	char *pagebuf = malloc(sizeof(char)*PAGE_SIZE);
	char *recordbuf = malloc(sizeof(char)*MAX_RECORD_SIZE);
	void header_area = malloc(HEADER_AREA_SIZE);
	int record_cnt;
	int page_num;
	int offset, len;

	pack(recordbuf, &p);
	//삭제 레코드가 존재하는 지 확인한다. 
	if(header_record.d_page_num != -1){
		int d_page_num = header_record.d_page_num;
		//삭제된 record number는 header area에서 offset 및 length를 확인하기 위해
		//header area에서의 record counut크기 4bytes를 더하고 
		//한 record의 offset, length 저장을 위한 각 4bytes를 고려하여 8byte를 곱한다
		int d_record_num = header_record.d_record_num*8+4;
		do{
			readPage(fp, pagebuf, d_page_num);
			memcpy(header_area, pagebuf, HEADER_AREA_SIZE);
			memcpy(&len, header_area+d_record_num+sizeof(offset), sizeof(len));
		
		}while(len < sizeof(recordbuf));//해당 위치에 recordbuf를 쓸 수 있을 때
		
	}else{//삭제 데이터가 없으면 마지막에 append
		int page_num = header_record.page_cnt < 1 ? 0 : header_record.page_cnt-1;
		readPage(fp, pagebuf, page_num);
		memcpy(header_area, pagebuf, HEADER_AREA_SIZE);
		memcpy(&record_cnt, header_area, sizeof(int));
		
		
		

	}

}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *id)
{
	printf("START DELETE\n");
	char *pagebuf = malloc(sizeof(char)*PAGE_SIZE);
	void *header_area = malloc(HEADER_AREA_SIZE);
	char *recordbuf;
	Person tmp;
	int record_cnt;
	int offset, len;

	for(int i=0; i<header_record.page_cnt; i++){
		
		readPage(fp, pagebuf, i);
		memcpy(header_area, pagebuf, HEADER_AREA_SIZE);
		memcpy(&record_cnt, header_area, sizeof(int));
		printf("record cnt : %d\n", record_cnt);
		for(int j=0; j<record_cnt; j++){
			printf("check page num : %d, record num : %d\n", i, j);
			int pos = j*8+4;
			memcpy(&offset, header_area+pos, sizeof(int));
			memcpy(&len, header_area+pos+4, sizeof(int));
			printf("offset : %d, len : %d\n" ,offset, len);
			recordbuf = (char *)malloc(sizeof(char)*len);
			memcpy(recordbuf, pagebuf+HEADER_AREA_SIZE+offset, len);

			if(recordbuf[0] == '*'){
				free(recordbuf);
				continue;
			}

			unpack(recordbuf, &tmp);

			if(strcmp(tmp.id, id) == 0){
				printf("find!!\n");
				//record에 삭제 기록
				recordbuf[0] = '*';
				memcpy(recordbuf+1, &header_record.d_page_num, sizeof(int));
				memcpy(recordbuf+1+sizeof(int), &header_record.d_record_num, sizeof(int));
				header_record.d_page_num = i;
				header_record.d_record_num = j;
				writeHeader(fp);
				//pagebuf에 삭제된 record copy
				memcpy(pagebuf+HEADER_AREA_SIZE+offset, recordbuf, len);
				writePage(fp, pagebuf, i);
				free(recordbuf);
				free(pagebuf);
				return;
			}
			free(recordbuf);
			
		}
	}
	free(pagebuf);
	
}

void writeHeader(FILE *fp){
	fseek(fp, 0, SEEK_SET);
	fwrite((void *)header_record, HEADER_RECORD, 1, fp);
}
int main(int argc, char *argv[])
{
	FILE *fp; // 레코드 파일의 파일 포인터
	Person input;
	char *recordbuf = malloc(sizeof(char)*MAX_RECORD_SIZE);


	if((fp = fopen(argv[2], "r+b")) == NULL){
		fp = fopen(argv[2], "w+b");
		header_record.page_cnt = 1;
		header_record.record_cnt = 1;
		header_record.d_page_num = -1;
		header_record.d_record_num = -1;
		fwrite(&header_record, HEADER_RECORD, 1, fp);
		//tmp
		char *page = (char*)malloc(sizeof(char)*PAGE_SIZE);
		memset(page, '\0', PAGE_SIZE);
		int cnt = 1, offset = 0, len = 60;
		memcpy(page, &cnt, sizeof(int));
		memcpy(page+4, &offset, sizeof(int));
		memcpy(page+8, &len, sizeof(int));
		memcpy(page+HEADER_AREA_SIZE, "8811032129018#GD Hong#23#Seoul#02-820-0924#gdhong@ssu.ac.kr#", 60);
		writePage(fp, page, 0);

	}else{
		fread(&header_record, HEADER_RECORD, 1, fp);
		//printf("p_cnt : %d\nr_cnt : %d\nd_p_n : %d\nd_r_n : %d\n", header_record.page_cnt, header_record.record_cnt, header_record.d_page_num, header_record.d_record_num);
	}
	delete(fp, "8811032129018");
	if(argc > 3 ){
		for(int i=0; i< argc; i++){
			//printf("[%d] : %s\n", i, argv[i]);
		}
		if(strlen(argv[1]) != 1){
			fprintf(stderr, "Invalid Option\n");
			exit(1);
		}
		
		char option = argv[1][0];
		
		if(option == 'a'){ //record add
			strcpy(input.id, argv[3]);
			strcpy(input.name, argv[4]);
			strcpy(input.age, argv[5]);
			strcpy(input.addr, argv[6]);
			strcpy(input.phone, argv[7]);
			strcpy(input.email, argv[8]);
			pack(recordbuf, &input);
			printf("%s\n", recordbuf);
			printf("size : %ld\n",strlen(recordbuf));
			Person tmp;
			unpack(recordbuf, &tmp);
			printf("unpack : %s\n", tmp.id);
			printf("unpack : %s\n", tmp.age);
			printf("unpack : %s\n", tmp.email);

		}else if(option == 'd'){ //record delete
			
		}else{
			fprintf(stderr, "Invalid Option\n");
			exit(1);
		}
		
	
	}else{
		fprintf(stderr, "NEED MORE PARAMETER.\n");
		exit(1);

	}

	return 1;
}