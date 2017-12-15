#include"fs.h"
#include"tool.h"
#include<memory.h>

int my_exitsys(const ARGP arg,FileSystemInfop fileSystemInfop){
    const char helpstr[]=
"\
����        �˳��ļ�ϵͳ\n\
�﷨��ʽ    exit\n";

    switch(arg->len){
        case 1:
            if(strcmp(arg->argv[0],"/?")==0 && arg->len==1){
                printf(helpstr);
                return ERROR;
            }
            goto error;
            break;
        case 0:
            break;
        default:
        error:;
            strcpy(error.msg,"������������\n\x00");
            printf("������������\n");
            return ERROR;
    }
    if(fileSystemInfop->flag){
        fclose(fileSystemInfop->fp);
        memset(fileSystemInfop,0,sizeof(FileSystemInfo));
    }
    return SUCCESS;
}