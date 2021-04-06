#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char* argv[]){
    long to_read;
    FILE *read;
    FILE *write;

    if(argc == 4){
       read = fopen(argv[1], "r");
       //fopen의 r옵션은 파일이 존재하지 않으면 null을 리턴한다.
        if(read == NULL){
            printf("파일이 존재하지 않습니다.\n");
            exit(1);
        }
        //파일에서 읽을 바이트수를 저장한다.
        to_read = atoi(argv[3]);
        //입력한 오프셋만큼 이동시킨다.
        fseek(read, atoi(argv[2]), SEEK_SET);
        //읽을 크기의 buffer 배열을 할당한다.
        char* buffer = (char*)malloc(sizeof(char)*to_read);
        //사용자가 입력한 offset부터 size만큼의 바이트를 buffer배열에 저장한다.
        fread(buffer, sizeof(char), to_read, read);
        printf("%s", buffer);
        free(buffer);
        printf("\n파일 출력이 완료되었습니다.\n");
    
    }else{
        printf("인자가 부족합니다.\n");
        exit(1);
    }
    
    fclose(read);

    return 0;
    
}