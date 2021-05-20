// 주의사항
// 1. blockmap.h에 정의되어 있는 상수 변수를 우선적으로 사용해야 함
// 2. blockmap.h에 정의되어 있지 않을 경우 본인이 이 파일에서 만들어서 사용하면 됨
// 3. 필요한 data structure가 필요하면 이 파일에서 정의해서 쓰기 바람(blockmap.h에 추가하면 안됨)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "blockmap.h"
// 필요한 경우 헤더 파일을 추가하시오.
//lbn, pbn이 저장되어 있다.
//인덱스 = lbn, value = pbn
int dd_read(int, char *);
int dd_write(int, char *);
int dd_erase(int );
void ftl_open();
void ftl_read(int , char *);
void ftl_write(int, char *);
void ftl_print();

int mapping_table[BLOCKS_PER_DEVICE];

FILE *flashfp;
typedef struct{
	int lbn;
	int lsn;
	long long is_free;
}spare;

int free_block;
//
// flash memory를 처음 사용할 때 필요한 초기화 작업, 예를 들면 address mapping table에 대한
// 초기화 등의 작업을 수행한다. 따라서, 첫 번째 ftl_write() 또는 ftl_read()가 호출되기 전에
// file system에 의해 반드시 먼저 호출이 되어야 한다.
//
void ftl_open()
{
	char *blockbuf;
	char pagebuf[PAGE_SIZE];
	spare sparebuf;


	if((flashfp = fopen("flashmemory", "r+b")) == NULL){
		flashfp = fopen("flashmemory", "w+b");
		blockbuf = (char *)malloc(BLOCK_SIZE);
		memset(blockbuf, 0xFF, BLOCK_SIZE);

		for(int i = 0; i < BLOCKS_PER_DEVICE; i++)
		{
			fwrite(blockbuf, BLOCK_SIZE, 1, flashfp);
		}
		free(blockbuf);
	}
	free_block = DATABLKS_PER_DEVICE;
	memset(mapping_table, 0XFF, sizeof(int)*BLOCKS_PER_DEVICE);

	for(int i=0; i<BLOCKS_PER_DEVICE; i++){
		dd_read(i*PAGES_PER_BLOCK, pagebuf);
		memcpy(&sparebuf, pagebuf+SECTOR_SIZE, SPARE_SIZE);
		if(sparebuf.lbn != -1)
			mapping_table[sparebuf.lbn] = i;
		if(sparebuf.is_free == TRUE)
			free_block = i;		
	}

	return;
}

//
// 이 함수를 호출하는 쪽(file system)에서 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 함
// (즉, 이 함수에서 메모리를 할당 받으면 안됨)
//
void ftl_read(int lsn, char *sectorbuf)
{
	int pbn, ppn;
	int lbn = lsn / PAGES_PER_BLOCK;
	int offset = lsn % PAGES_PER_BLOCK;
	char pagebuf[PAGE_SIZE];

	//ppn에 lbn과 매치하여 pb n을 담는다. 
	if((pbn = mapping_table[lbn]) < 0){
		fprintf(stderr, "ftl_read error any pbn match on lbn[%d]", lbn);
		exit(1);
	}

	ppn = pbn*PAGES_PER_BLOCK + offset;

	dd_read(ppn, pagebuf);
	memcpy(sectorbuf, pagebuf, SECTOR_SIZE);
	sectorbuf[SECTOR_SIZE] = '\0';

	return;
}

//
// 이 함수를 호출하는 쪽(file system)에서 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 함
// (즉, 이 함수에서 메모리를 할당 받으면 안됨)
//
void ftl_write(int lsn, char *sectorbuf)
{
	int ppn, pbn;
	int lbn = lsn/PAGES_PER_BLOCK;
	int offset = lsn%PAGES_PER_BLOCK;
	char pagebuf[PAGE_SIZE];
	spare sparebuf;

	if((pbn = mapping_table[lbn]) != -1){
		
		//physical page number를 저장
		ppn = pbn*PAGES_PER_BLOCK + offset;
		dd_read(ppn, pagebuf);
		memcpy(&sparebuf, pagebuf+SECTOR_SIZE, SPARE_SIZE);
		//해당 페이지에 이미 데이터가 존재하면 Overwrite이다.
		if(sparebuf.lsn >= 0){	
	
			int prev_ppn = pbn*PAGES_PER_BLOCK;
			int dest_ppn = free_block*PAGES_PER_BLOCK;

			for(int i=0; i<PAGES_PER_BLOCK; i++){
				if(i != offset){
					dd_read(prev_ppn+i, pagebuf);
					dd_write(dest_ppn+i, pagebuf);
					memset(pagebuf, '\0', PAGE_SIZE);
				}else{
					memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
					sparebuf.lbn = lbn;
					sparebuf.lsn = lsn;
					sparebuf.is_free = -1;
					memcpy(pagebuf+SECTOR_SIZE, &sparebuf, SPARE_SIZE);
					dd_write(dest_ppn+i, pagebuf);
				}
			}
			mapping_table[lbn] = free_block;
			dd_erase(pbn);
			dd_read(pbn*PAGES_PER_BLOCK, pagebuf);
			sparebuf.lbn = -1;
			sparebuf.lsn = -1;
			sparebuf.is_free = TRUE;
			memcpy(pagebuf+SECTOR_SIZE, &sparebuf, SPARE_SIZE);
			dd_write(pbn*PAGES_PER_BLOCK, pagebuf);
			free_block = pbn;
			return;
		}else{
			//해당 위치에 데이터가 없으면 그냥 write
			//page buf, sparebuf 초기화
			memset(pagebuf, '\0', PAGE_SIZE);
			memset(&sparebuf, '\0', SPARE_SIZE);
			//현재 데이터에 해당하는 내용 입력
			sparebuf.lbn = lbn;
			sparebuf.lsn = lsn;
			sparebuf.is_free = -1;
			memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
			memcpy(pagebuf+SECTOR_SIZE, &sparebuf, SPARE_SIZE);
			dd_write(ppn, pagebuf);
			return;
		}
	}else{
		for(int i=0; i<DATABLKS_PER_DEVICE; i++){ 
			if(mapping_table[i] == -1 && i != free_block){
				//pbn, ppn 값 구하기
				pbn = i;
				ppn = pbn*PAGES_PER_BLOCK + offset;
				//mapping table update
				mapping_table[lbn] = pbn;
				//mapping_table 복구를 위해 첫번째 블럭 lbn값 수정
				dd_read(pbn*PAGES_PER_BLOCK, pagebuf);
				sparebuf.lbn = lbn;
				sparebuf.lsn = -1;
				sparebuf.is_free = -1;
				memcpy(pagebuf+SECTOR_SIZE, &sparebuf, SPARE_SIZE);
				printf("PBN!!!!!!!: %d\n", pbn*PAGES_PER_BLOCK);
				dd_write(pbn * PAGES_PER_BLOCK, pagebuf);
				//pagebuf, sparebuf 초기화
				memset(pagebuf, '\0', PAGE_SIZE);
				//해당하는 physical page에 값 쓰기
				sparebuf.lbn = lbn;
				sparebuf.lsn = lsn;
				sparebuf.is_free = -1;
				memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
				memcpy(pagebuf+SECTOR_SIZE, &sparebuf, SPARE_SIZE);
				dd_write(ppn, pagebuf);
				printf("PPN!!!!!!!!: %d\n", ppn);
				return;
			}
		}
		//여기까지 내려오면 용량이 꽉차서 맞는 페이지를 못찾은거임
		//가용 페이지 없음
		fprintf(stderr, "Can't find empty PAGE\n");
		exit(1);
	}

	return;
}

void ftl_print()
{
	printf("lbn|pbn\n");
	for(int i=0; i<BLOCKS_PER_DEVICE; i++){
		printf("%3d%3d\n", i, mapping_table[i]);
	}
	printf("free block's pbn=%d\n", free_block);
	return;
}

