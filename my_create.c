#include"fs.h"
#include"tool.h"
#include<memory.h>
#include<ctype.h>

int my_create(const ARGP arg,FileSystemInfop fileSystemInfop){
    const char helpstr[]=
"\
����        �����ļ�\n\
�﷨��ʽ    create name\n\
name       �����ļ��е�����\n\
��ע       �ļ���ǿ��תΪ��д���ļ���Ҫ��������������ʽ\n";
    char name[ARGLEN];
    FAT_DS_BLOCK4K fat_ds;
    if(fileSystemInfop->flag==FALSE){
        strcpy(error.msg,"δָ���ļ�ϵͳ\n\x00");
        printf("δָ���ļ�ϵͳ\n");
        return ERROR;
    }
    switch(arg->len){
        case 1:
            if(strcmp(arg->argv[0],"/?")==0){
                printf(helpstr);
                return SUCCESS;
            }else{
                if(nameCheckChange(arg->argv[0],name)==ERROR){
                    strcpy(error.msg,"�ļ�����������ڷǷ��ַ�\n\x00");
                    printf("�ļ�����������ڷǷ��ַ�\n");
                    return ERROR;
                }
                for(int i=0;i<11;i++){
                    name[i]=toupper(name[i]);
                }
                name[11]='\0';
            }
            break;
        case 0:
            DEBUG("�ļ�����\n");
            return SUCCESS;
        default:
        error:;
            strcpy(error.msg,"������������\n\x00");
            printf("������������\n");
            return ERROR;
    }

    u32 pathNum=fileSystemInfop->pathNum;
    u32 cut;
    while(TRUE){
        //�����һҳ��Ŀ¼��
        do_read_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,pathNum));
        for(cut=0;cut<SPCSIZE/32;cut++){
            char lin[12];
            my_strcpy(lin,fat_ds.fat[cut].name,11);
            lin[11]='\0';
            if(fat_ds.fat[cut].name[0]=='\xE5' || fat_ds.fat[cut].name[0]=='\x00'){
                break;
            }else if((fat_ds.fat[cut].DIR_Attr) && 
                            strcmp(name,lin)==0 ){
                strcpy(error.msg,"�ļ��Ѵ���\n\x00");
                printf("�ļ��Ѵ���\n");
                return ERROR;
            }
        }
        //ȡ����һ��Ŀ¼��
        if(cut==SPCSIZE/32){
            u32 lin=pathNum;
            pathNum=getNext(fileSystemInfop,pathNum);
            if(pathNum==FAT_END){
                //ȫ��û�з���һ��
                pathNum=newfree(fileSystemInfop,lin);
                if(pathNum==FAT_FREE){
                    strcpy(error.msg,"���̿ռ䲻��\n\x00");
                    printf("���̿ռ䲻��\n");
                    return ERROR;
                }
            }
        }else{
            //�ҵ��˿յ�
            memset(&fat_ds.fat[cut],0,sizeof(FAT_DS));
            my_strcpy(fat_ds.fat[cut].name,name,11);
            fat_ds.fat[cut].DIR_Attr=ATTR_ARCHIVE;
            
            //д���½��ļ�
            do_write_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,pathNum));
            DEBUG("�����ɹ�\n");
            break;
        }
    }
    return SUCCESS;
}