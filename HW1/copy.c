#include<stdio.h>
#include<fcntl.h>
#define MAX_FILE_NAME 50


int read();
int copy();
int main(int argc, char* argv[]){

    for(int i=0; i<argc; i++){
        printf("인자는 %s입니다.\n", argv[i]);
    }
    char c;
    int fd;
    char filename[MAX_FILE_NAME];
    
    fd = fopen(argv[1], O_RDONLY);
    
    return 0;
    
}