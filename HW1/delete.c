#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char* argv[]){
    FILE *to_delete;
    long front_size;
    long back_size;
    long delete_size;

    if(argc ==4){
        to_delete = fopen(argv[1], "r");

        if(to_delete == NULL){
            printf("파일이 존재하지 않습니다.\n");
            exit(1);
        }
        
        front_size = atoi(argv[2])-1;
        fseek(to_delete, 0, SEEK_END);
        back_size = ftell(to_delete)-front_size;
        fseek(to_delete, 0, SEEK_SET);
        delete_size = atoi(argv[3]);
        char *front_buffer = (char*)malloc(sizeof(char)*front_size);\
        char *back_buffer = (char*)malloc(sizeof(char)*back_size);
        //버퍼의 사이즈가 음수일 경우 읽지 않는다.
        if(front_size > 0)
            fread(front_buffer, sizeof(char), front_size, to_delete);
        if(back_size > 0)
            fread(back_buffer, sizeof(char), back_size, to_delete);
    
        fclose(to_delete);
        
        to_delete = fopen(argv[1], "w");
        if(front_size > 0)
            fwrite(front_buffer, sizeof(char), front_size, to_delete);
        if(back_size-delete_size > 0)
            fwrite(back_buffer+delete_size, sizeof(char), back_size-delete_size, to_delete);
        
        
    }else{
        printf("인자의 개수가 맞지 않습니다.\n");
        exit(1);
    }
    fclose(to_delete);

    return 0;
}