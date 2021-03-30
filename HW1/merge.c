#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void add(FILE *from, FILE  *to){
    long file_size;
    fseek(from, 0, SEEK_END);
    file_size = ftell(from);
    fseek(from, 0, SEEK_SET);
    char *buffer = (char*)malloc(sizeof(char)*file_size);
    //현재 읽은 버퍼의 크기를 저장함으로서 내용을 복사할때
    //의도하지 않은 내용이 복사되는것을 방지한다.
    fread(buffer, sizeof(char), file_size, from);
    fwrite(buffer, sizeof(char), file_size, to);
    free(buffer);
}
int main(int argc, char* argv[]){
    
    FILE *fp1;
    FILE *fp2;
    FILE *merged;

    if(argc == 4){
        //병합할 파일 2개를 읽기 모드로 open한다.
        fp1 = fopen(argv[1], "r");
        fp2 = fopen(argv[2], "r");

        //fopen의 r옵션은 파일이 존재하지 않으면 null을 리턴한다.
        //open할 파일 중 1개라도 존재하지 않을 시 프로그램 종료
        if(fp1 == NULL || fp2 == NULL){
            printf("파일이 존재하지 않습니다.\n");
            exit(1);
        }
        //병합한 내용을 저장할 파일을 쓰기 모드로 생성한다.
        //이미 동일한 이름의 파일이 존재하면 덮어쓴다.
        merged = fopen(argv[3], "w");
        //merged 파일 포인터에 fp1, fp2의 내용을 합친다.
        add(fp1, merged);
        add(fp2, merged);
        printf("파일 합병 완료되었습니다.\n");
    
    }else{
        printf("인자가 부족합니다.\n");
        exit(1);
    }
    fclose(fp1);
    fclose(fp2);
    fclose(merged);

    return 0;
    
}