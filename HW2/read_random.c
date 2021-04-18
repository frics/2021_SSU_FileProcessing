#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#define RECORD_SIZE 250
#define SUFFLE_NUM	10000	// 이 값은 마음대로 수정할 수 있음.

void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);
// 필요한 함수가 있으면 더 추가할 수 있음.

int main(int argc, char **argv)
{
	int *read_order_list;
	int num_of_records; // 레코드 파일에 저장되어 있는 전체 레코드의 수
	int offset;
	char *record = (char*)malloc(sizeof(char)*RECORD_SIZE);
	FILE *pFile;
	struct timeval start, end;
	long long elapsed;
	
	if(argc == 2){
		if((pFile = fopen(argv[1], "rb")) == NULL){
			fprintf(stderr, "open error for %s\n", argv[1]);
		}
		fread(&num_of_records, sizeof(int), 1, pFile);
		read_order_list = (int*)malloc(sizeof(int)*num_of_records);

		// 이 함수를 실행하면 'read_order_list' 배열에는 읽어야 할 레코드 번호들이 나열되어 저장됨
		GenRecordSequence(read_order_list, num_of_records);
		
		gettimeofday(&start, NULL);
		for(int i=0; i<num_of_records; i++){
			offset = read_order_list[i]*RECORD_SIZE+4;
			fseek(pFile, offset, SEEK_SET);
			fread(record, sizeof(char), RECORD_SIZE, pFile);
		}
		gettimeofday(&end, NULL);
		elapsed = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;;
	
		printf("#records: %d elapsed_time: %lld us\n", num_of_records, elapsed);
	
	}else{
		exit(1);
	}
	fclose(pFile);
	free(record);
	free(read_order_list);
	return 0;
}

void GenRecordSequence(int *list, int n)
{
	int i, j, k;
	
	srand((unsigned int)time(0));

	for(i=0; i<n; i++)
	{
		list[i] = i;
	}
	
	for(i=0; i<SUFFLE_NUM; i++)
	{
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}
}

void swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}
