#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char* argv[]){
    FILE *to_insert;
    long front_size;
    long back_size;

    if(argc ==4){
        to_insert = fopen(argv[1], "r+");

        if(to_insert == NULL){
            printf("파일이 존재하지 않습니다.\n");
            exit(1);
        }
        //사용자가 입력한 문자열을 삽입할 시작 위치
        front_size = atoi(argv[2]);
        //fseek으로 파일 전체 크기를 획득
        fseek(to_insert, 0, SEEK_END);
        //파일 전체 크기에서 시작 위치를 빼서 시작 위치 이후의 바이트 크기 획득
        back_size = ftell(to_insert)-front_size;
        //insert 뒷부분의 내용을 백업하기 위해 파일 포인터를 offset위치로 옮긴다.
        fseek(to_insert, front_size, SEEK_SET);

        //뒷부분 내용을 저장한 메모리 할당 및 읽기
        char *back_buffer = (char*)malloc(sizeof(char)*back_size);
        fread(back_buffer, sizeof(char), back_size, to_insert);
        //사용자가 입력한 내용을 insert하기 위해 파일 포인터를 offset위치로 옮긴다.
        fseek(to_insert, front_size, SEEK_SET);
        //사용자가 입력한 내용 및 뒷부분 내용쓰기
        fwrite(argv[3], sizeof(char), strlen(argv[3]), to_insert);
        fwrite(back_buffer, sizeof(char), back_size, to_insert);
        printf("파일에 문자열 삽입이 완료되었습니다.\n");
        
    }else{
        printf("인자의 개수가 맞지 않습니다.\n");
        exit(1);
    }
    fclose(to_insert);

    return 0;
}