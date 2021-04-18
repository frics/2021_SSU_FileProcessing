#include <stdio.h>
#include<string.h>
#include<stdlib.h>

#define RECORD_SIZE 250
// 필요하면 hearder file을 추가할 수 있음

int main(int argc, char **argv)
{

	//int형 변수의 크기가 4 bytes이기 때문에 header를 4bytes로 선언
	int header;
	char *record = (char*)malloc(sizeof(char)*RECORD_SIZE);
	FILE *pFile;
	
	if(argc == 3){
		if((pFile = fopen(argv[2], "wb+")) == NULL){
			fprintf(stderr, "open error for %s\n", argv[2]);
			exit(1);
		}
		header = atoi(argv[1]);
		fwrite(&header, sizeof(int), 1, pFile);
		memset(record, '\0', RECORD_SIZE);

		for(int i=0; i<header; i++)
			fwrite(record, sizeof(char), RECORD_SIZE, pFile);
		
	}else{
		exit(1);
	}
	fclose(pFile);	
	free(record);
	
	return 0;
}
