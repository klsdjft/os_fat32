#include"fs.h"
#include"tool.h"
#include<stdio.h>
#include<stdlib.h>
const char WXML[]="��Ч����\n";
int main(){
    printf("��ӭʹ���ļ�ϵͳ\n");
    char cmd[ARGLEN];
    ARG argv;
    FileSystemInfo fileSysInfo;
    while(TRUE){
        scanf("%s",cmd);
        getargv(&argv);
        if(strcmp(cmd,"format")==0){
            my_format(&argv);
        }else if(strcmp(cmd,"load")==0){
            my_load(&argv,&fileSysInfo);
        }else if(strcmp(cmd,"exit")==0){
            if(argv.len==0){
                break;
            }else{
                printf(WXML);
            }
        }else{
            printf("�Ƿ�����\n");
        }
    }

    return 0;
}