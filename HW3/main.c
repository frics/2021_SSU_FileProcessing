// 
// 과제3의 채점 프로그램은 기본적으로 아래와 같이 동작함
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "blockmap.h"


//FILE *flashfp;

/****************  prototypes ****************/
void ftl_open();
void ftl_write(int lsn, char *sectorbuf);
void ftl_read(int lsn, char *sectorbuf);
void ftl_print();

//
// 이 함수는 file system의 역할을 수행한다고 생각하면 되고,
// file system이 flash memory로부터 512B씩 데이터를 저장하거나 데이터를 읽어 오기 위해서는
// 각자 구현한 FTL의 ftl_write()와 ftl_read()를 호출하면 됨
//
int main(int argc, char *argv[])
{
	
    char sectorbuf[SECTOR_SIZE];
	int lsn, i;

/*
    flashfp = fopen("flashmemory", "w+b");
	if(flashfp == NULL)
	{
		printf("file open error\n");
		exit(1);
	}
*/	   
    //
    // flash memory의 모든 바이트를 '0xff'로 초기화한다.
    // (512+16) * 4 = 2112바이트
	
    


	ftl_open(); 
	
	ftl_print();   // ftl_read(), ftl_write() 호출하기 전에 이 함수를 반드시 호출해야 함
	char ch = 'A';

	for(int i=0; i<8; i++){
		memset(sectorbuf, ch, SECTOR_SIZE);
		ftl_write(i, sectorbuf);
		memset(sectorbuf, '\0', SECTOR_SIZE);	
		//ftl_read(i, sectorbuf);
		//printf("Logical Page Number[%d] : %s\n", i, sectorbuf);
		ch++;
		
	}
	for(int i=8-1; i>=0; i--){
		ftl_read(i, sectorbuf);
		printf("Logical Page Number[%d] : %s\n", i, sectorbuf);
		}
	
	// ftl_write() 및 ftl_read() 테스트 코드 작성
	//
	//ftl_open();
	//ftl_print();

	//fclose(flashfp);

	return 0;
}
