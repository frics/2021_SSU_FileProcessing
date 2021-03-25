#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#define MAX_FILE_NAME 50
#define BUFFER_SIZE 10

int get_file_size();
int read();
int copy();

int main(int argc, char* argv[]){

    char* buffer = (char*)malloc(sizeof(char)*BUFFER_SIZE);
    FILE *read;
    FILE *write;

    if(argc == 3){
       read = fopen(argv[1], "r");
       //fopen의 r옵션은 파일이 존재하지 않으면 null을 리턴한다.
        if(read == NULL){
            printf("파일이 존재하지 않습니다.\n");
            exit(1);
        }else{
            int curr_buf_size;
            write = fopen(argv[2], "w+");
            while(feof(read)==0){
                //현재 읽은 버퍼의 크기를 저장함으로서 내용을 복사할때
                //의도하지 않은 내용이 복사되는것을 방지한다.
                curr_buf_size = fread(buffer, sizeof(char), BUFFER_SIZE, read);
                fwrite(buffer, sizeof(char), curr_buf_size, write);
                memset(buffer, '\0', BUFFER_SIZE);
            }
            printf("파일 복사가 완료되었습니다.\n");
        }
    }else{
        printf("인자가 부족합니다.\n");
        exit(1);
    }
    fclose(read);
    fclose(write);

    return 0;
    
}