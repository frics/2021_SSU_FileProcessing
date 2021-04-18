#include <stdio.h> //printf();
#include <stdlib.h> //exit(1);
#include <sys/time.h> //gettimeofday();

#define RECORD_SIZE 250

//필요하면 header file 추가 가능

int main(int argc, char **argv)
{
	//파일로부터 헤더를 읽어서 저장
	int num_of_records;
	//파일로부터 레코드를 읽어드릴 char형 포인터를 record size만큼 동적할당한다.
	char *record = (char*)malloc(sizeof(char)*RECORD_SIZE);
	FILE *pFile;
	struct timeval start, end;
	long long elapsed;

	if(argc == 2){
		if((pFile = fopen(argv[1], "rb")) == NULL){
			fprintf(stderr, "open error for %s\n", argv[1]);
			exit(1);
		}
		//4bytes짜리 헤더를 read해 read할 레코드의 개수를 파악한다.
		fread(&num_of_records, sizeof(int), 1, pFile);
		//측정 시작
		gettimeofday(&start, NULL);
		//헤더에 저장된 개수만큼 250bytes짜리  record를 헤더에 저장된 개수만큼 읽는다.
		for(int i= 0; i < num_of_records; i++){
			fread(record, sizeof(char), RECORD_SIZE, pFile);
		}
		//측정 종료
		gettimeofday(&end, NULL);
		
		elapsed = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;;
	
		printf("#records: %d elapsed_time: %lld us\n", num_of_records, elapsed);

	}else{
		exit(1);
	}
	fclose(pFile);
	free(record);

	return 0;
}
