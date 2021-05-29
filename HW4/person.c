#include <stdio.h>
#include "person.h"
#include<stdlib.h>
#include<string.h>

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

void readPage(FILE *fp, char *pagebuf, int pagenum)
{	
	fseek(fp, HEADER_RECORD + PAGE_SIZE * pagenum, SEEK_SET);
	fread((void *)pagebuf, PAGE_SIZE, 1, fp);	
}

void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, HEADER_RECORD + PAGE_SIZE * pagenum, SEEK_SET);
	fwrite((void *)pagebuf, PAGE_SIZE, 1, fp);
}

void pack(char *recordbuf, const Person *p)
{
	sprintf(recordbuf, "%s#%s#%s#%s#%s#%s#", p->id, p->name, p->age, p->addr, p->phone, p->email);
}

void unpack(const char *recordbuf, Person *p)
{
	sscanf(recordbuf, "%[^#]%[^#]%[^#]%[^#]%[^#]%[^#]", p->id, p->name, p->age, p->addr, p->phone, p->email);
}
//header record를 write해준다.
void writeHeader(FILE *fp){
	fseek(fp, 0, SEEK_SET);
	fwrite((void *)&header_record, HEADER_RECORD, 1, fp);
}
//새로운 페이지 할당
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
			//fit 체크	
			if(len >= strlen(recordbuf)){
				int next[2];
				memcpy(pagebuf+HEADER_AREA_SIZE+offset, recordbuf, len);
				writePage(fp, pagebuf, cur[0]);
				//list 수정
				memcpy(next, delete_record+1, sizeof(int)*2);

				if(header_record.d_page_num != cur[0] || header_record.d_record_num != cur[1]){

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
					header_record.d_page_num = next[0];
					header_record.d_record_num = next[1];
					writeHeader(fp);
				}
				free(pagebuf);
				free(header_area);
				free(delete_record);
				free(recordbuf);
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
		}else//새로운 페이지 alloc
			alloc_page(fp, recordbuf, page_num+1);
	}else{
		alloc_page(fp, recordbuf, 0);
	}
	free(pagebuf);
	free(header_area);
	free(recordbuf);
	return;
}

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
	
	if((fp = fopen(argv[2], "r+b")) == NULL){
		fp = fopen(argv[2], "w+b");
		header_record.page_cnt = 0;
		header_record.record_cnt = 0;
		header_record.d_page_num = -1;
		header_record.d_record_num = -1;
		writeHeader(fp);
		
	}else{
		fread(&header_record, HEADER_RECORD, 1, fp);
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
		}else{
			fprintf(stderr, "Invalid Option\n");
			exit(1);
		}
		
	}else{
		fprintf(stderr, "NEED MORE PARAMETER.\n");
		exit(1);
	}
	return 0;
}