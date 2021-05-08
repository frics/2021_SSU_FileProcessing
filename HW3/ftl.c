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
	int spare[2];
}spare;
spare spare_table[BLOCKS_PER_DEVICE * PAGES_PER_BLOCK];
int free_block;
//
// flash memory를 처음 사용할 때 필요한 초기화 작업, 예를 들면 address mapping table에 대한
// 초기화 등의 작업을 수행한다. 따라서, 첫 번째 ftl_write() 또는 ftl_read()가 호출되기 전에
// file system에 의해 반드시 먼저 호출이 되어야 한다.
//
void ftl_open()
{
	char *blockbuf;
	char *pagebuf = (char *)malloc(sizeof(char) * PAGE_SIZE);
	spare sparebuf;
	
	//
	// address mapping table 초기화 또는 복구
	for(int i=0; i<BLOCKS_PER_DEVICE; i++)
		mapping_table[i] = -1;
	for(int i=0; i<BLOCKS_PER_DEVICE * PAGES_PER_BLOCK; i++){
		spare_table[i].lsn = -1;
		spare_table[i].lbn = -1;
	}
	
	if((flashfp = fopen("flashmemory", "r+b")) == NULL){
		flashfp = fopen("flashmemory", "w+b");
		blockbuf = (char *)malloc(BLOCK_SIZE);
		memset(blockbuf, 0xFF, BLOCK_SIZE);

		for(int i = 0; i < BLOCKS_PER_DEVICE; i++)
		{
			fwrite(blockbuf, BLOCK_SIZE, 1, flashfp);
		}

		free(blockbuf);
	}else{
		for(int i = 0; i<BLOCKS_PER_DEVICE * PAGES_PER_BLOCK; i++){
			dd_read(i, pagebuf);
			memcpy(&sparebuf, pagebuf+SECTOR_SIZE, SPARE_SIZE);
			memcpy(&spare_table[i], &sparebuf, SPARE_SIZE);
			if(i%PAGES_PER_BLOCK == 0 && sparebuf.lbn != -1)
				mapping_table[sparebuf.lbn] =i%PAGES_PER_BLOCK;
		}
	}

	free_block = DATABLKS_PER_DEVICE;
	// free block's pbn 초기화
		
    	// address mapping table에서 lbn 수는 DATABLKS_PER_DEVICE 동일
	
	return;
}

//
// 이 함수를 호출하는 쪽(file system)에서 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 함
// (즉, 이 함수에서 메모리를 할당 받으면 안됨)
//
void ftl_read(int lsn, char *sectorbuf)
{
	int ppn;
	int lbn = lsn / PAGES_PER_BLOCK;
	int offset = lsn % PAGES_PER_BLOCK;
	char *pagebuf = (char *)malloc(sizeof(char) * PAGE_SIZE);

	//ppn에 lbn과 매치하여 pb n을 담는다. 
	if((ppn = mapping_table[lbn]) < 0){
		fprintf(stderr, "ftl_read error any pbn match on lbn[%d]", lbn);
		exit(1);
	}

	ppn = ppn*PAGES_PER_BLOCK + offset;

	dd_read(ppn, pagebuf);
	memcpy(sectorbuf, pagebuf, SECTOR_SIZE);
	sectorbuf[SECTOR_SIZE] = '\0';

	spare tmp;
	memcpy(&tmp, pagebuf+SECTOR_SIZE, SPARE_SIZE);
	free(pagebuf);
	

	return;
}

//
// 이 함수를 호출하는 쪽(file system)에서 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 함
// (즉, 이 함수에서 메모리를 할당 받으면 안됨)
//
void ftl_write(int lsn, char *sectorbuf)
{
	//printf("\n\n\n\n\nWRITE DEBUG lsn : %d, buf : %c\n", lsn, sectorbuf[0]);
	//어디에 쓸지 탐색
	//1. mapping table에서 해당 위치 블럭이 사용됬느닞  
	int ppn, pbn;
	int lbn = lsn/PAGES_PER_BLOCK;
	int offset = lsn%PAGES_PER_BLOCK;
	char *pagebuf = (char *)malloc(PAGE_SIZE);
	spare sparebuf;
	//printf("lbn : %d, offset : %d\n", lbn, offset);
	//색터 내용 저장
	
	//해당 블럭이 이미 사용됬는지
	//사용되었으면 해당 페이지가 이미 사용이 됬는지
	//해당 페이지가 사용되었으면  update 아니라면 그냥 write
	if((pbn = mapping_table[lbn]) != -1){
		ppn = pbn*PAGES_PER_BLOCK + offset;
		if(spare_table[ppn].lsn == lsn){
		//	printf("블럭 overwrite %d\n", free_block);
			//overwrite
			int prev_block_num = pbn;
			int prev_page_num = pbn*PAGES_PER_BLOCK;
			int dest_block_num = free_block;
			int dest_page_num = free_block*PAGES_PER_BLOCK;
			
			//fre블럭에 써줌
			for(int i=0; i< PAGES_PER_BLOCK; i++){
				if(spare_table[prev_page_num+i].lsn != lsn){
					
					spare_table[dest_page_num+i].lbn = spare_table[prev_page_num + i].lbn;
					spare_table[dest_page_num+i].lsn = spare_table[prev_page_num + i].lsn;
					spare_table[prev_page_num + i].lbn = -1;
					spare_table[prev_page_num + i].lsn = -1;
					dd_read(prev_page_num+i, pagebuf);
					memcpy(pagebuf+SECTOR_SIZE, &spare_table[dest_page_num + i], SPARE_SIZE);
					dd_write(dest_page_num+i, pagebuf);

				}else{
					spare_table[dest_page_num+i].lbn = lbn;
					spare_table[dest_page_num+i].lsn = lsn;
					memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
					memcpy(pagebuf+SECTOR_SIZE, &spare_table[dest_page_num+i], SPARE_SIZE);
					dd_write(dest_page_num+i, pagebuf);
				}
				memset(pagebuf, '\0', PAGE_SIZE);
			}
			printf("lbn : %d, free : %d\n", lbn, dest_block_num);
			mapping_table[lbn] = dest_block_num;
			free_block = prev_block_num;
			dd_erase(prev_block_num);
		
		}
		else{//기존 블럭에 삽입
			printf("기존 블럭에 삽입\n");
			//spare 내용 저장
			spare_table[ppn].lbn = lbn;
			spare_table[ppn].lsn = lsn;
			memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
			memcpy(pagebuf+SECTOR_SIZE, &spare_table[ppn], SPARE_SIZE);
			dd_write(ppn, pagebuf);
		}
	}else{
		//빈 블럭 찾기
		printf("빈블럭을 찾아보자 !!!!\n");
		for(int i=0; i<BLOCKS_PER_DEVICE; i++){
			if(mapping_table[i] == -1 && i != free_block){
				pbn = i * PAGES_PER_BLOCK;
			
				mapping_table[lbn] = i;
				spare_table[pbn].lbn = lbn;
				spare_table[pbn].lsn = -1;
				dd_read(pbn, pagebuf);
				memcpy(pagebuf+SECTOR_SIZE, &spare_table[pbn], SPARE_SIZE);
				dd_write(pbn, pagebuf);

				memset(pagebuf, '\0', PAGE_SIZE);
			
				spare_table[pbn+offset].lbn = lbn;
				spare_table[pbn+offset].lsn = lsn;
				memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
				memcpy(pagebuf+SECTOR_SIZE, &spare_table[pbn+offset], SPARE_SIZE);
				dd_write(pbn+offset, pagebuf);
				break;
			}
		}
	}

	return;
}

void ftl_print()
{

	
	printf("lbn|pbn\n");
	for(int i=0; i<BLOCKS_PER_DEVICE; i++){
		printf("%3d|%3d\n", i, mapping_table[i]);
	}

//	for(int i=0; i<BLOCKS_PER_DEVICE * PAGES_PER_BLOCK; i++){
//		printf("spare : lbn %d | lsn %d\n", spare_table[i].lbn, spare_table[i].lsn);
//	}

	printf("free block's pbn=%d\n", free_block);
	return;
}