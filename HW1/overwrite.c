#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void overwrite(FILE *to, long offset, char *str){
    //사용자가 입력한 offset부터 새로 덮어쓰기 위해 
    //파일 포인터를 fseek를 이용하여 입력한 offset으로 이동시킨다.
    fseek(to, offset, SEEK_SET);
    //해당 위치부터 사용자 입력한 문자열의 bytes만큼 내용을 덮어쓴다.
    fwrite(str, sizeof(char), strlen(str), to);
}

int main(int argc, char* argv[]){
    FILE *to_overwrite;

    if(argc == 4){
        //r+는 현재 있는 파일을 열어 읽고,쓰기를 모두 덮어쓸 수 있다.
        to_overwrite = fopen(argv[1], "r+");
        //fopen의 r옵션은 파일이 존재하지 않으면 null을 리턴한다.
        if(to_overwrite == NULL){
            printf("파일이 존재하지 않습니다.\n");
            exit(1);
        }
        overwrite(to_overwrite, atoi(argv[2]), argv[3]);
        printf("파일 덮어쓰기가 완료되었습니다.\n");
        
    }else{
        printf("인자의 개수가 맞지 않습니다.\n");
        exit(1);
    }
    fclose(to_overwrite);

    return 0;
}