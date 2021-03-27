#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char* argv[]){
    FILE *to_insert;
    long front_size;
    long back_size;

    if(argc ==4){
        to_insert = fopen(argv[1], "r");

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
        //파일 내용을 읽어오기 위해 파일 포인터 초기화
        fseek(to_insert, 0, SEEK_SET);

        char *front_buffer = (char*)malloc(sizeof(char)*front_size);
        char *back_buffer = (char*)malloc(sizeof(char)*back_size);
        
        fread(front_buffer, sizeof(char), front_size, to_insert);
        fread(back_buffer, sizeof(char), back_size, to_insert);
        fclose(to_insert);
        
        to_insert = fopen(argv[1], "w");

        fwrite(front_buffer, sizeof(char), front_size, to_insert);
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