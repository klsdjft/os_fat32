/*
    ʵ�ְ����Ƽ���������̲������ļ�ϵͳ
    ���ر�Ҫ����
    ���� �����������
*/
#include"fs.h"
#include"tool.h"
#include<string.h>
//���ļ���δ�ر�
int my_load(const ARGP arg,FileSystemInfop fileSystemInfop){
    char fileName[ARGLEN]="fs.vhd";
    const char helpstr[]=
"\
����        �����ļ�ϵͳ\n\
�﷨��ʽ    load [namefile]\n\
namefile    ��������ļ�·������ǰĿ¼�¿�ʼ�� Ĭ�� fs.vhd\n";
    FILE *fp=NULL;
    MBR mbr;
    BS_BPB bpb;

    switch(arg->len){
        case 1:
            if(strcmp(arg->argv[0],"/?")==0 && arg->len==1){
                printf(helpstr);
                return SUCCESS;
            }else{
                strcpy(fileName,arg->argv[0]);
                break;
            }
            break;
        case 0:
            break;
        default:
            strcpy(error.msg,"������������\n\x00");
            printf("������������\n");
            return ERROR;
    }

    fp=fopen(fileName,"rb+");
    if(fp==NULL){
        strcpy(error.msg,"�ļ��򿪴���\n\x00");
        printf("�ļ���������\n");
        return ERROR;
    }
    fileSystemInfop->flag=TRUE;
    fileSystemInfop->fp=fp;
    strcpy(fileSystemInfop->fileName,fileName);
    strcpy(fileSystemInfop->path,"/");

    //MBR����
    do_read_block(fp,(BLOCK*)&mbr,0,0);
    fileSystemInfop->MBR_size=mbr.mbr_in[0].all;
    fileSystemInfop->MBR_start=mbr.mbr_in[0].strart_chan;


    do_read_block(fp,(BLOCK*)&bpb,fileSystemInfop->MBR_start/8,fileSystemInfop->MBR_start%8);
    fileSystemInfop->BPB_BytsPerSec=bpb.BPB_BytsPerSec;
    fileSystemInfop->BPB_SecPerClus=bpb.BPB_SecPerClus;
    fileSystemInfop->BPB_RsvdSecCnt=bpb.BPB_RsvdSecCnt;
    fileSystemInfop->BPB_NumFATs=bpb.BPB_NumFATs;
    fileSystemInfop->BPB_HiddSec=bpb.BPB_HiddSec;
    fileSystemInfop->BPB_TotSec32=bpb.BPB_TotSec32;
    fileSystemInfop->BPB_FATSz32=bpb.BPB_FATSz32;
    fileSystemInfop->BPB_RootClis=bpb.BPB_RootClis;
    fileSystemInfop->BPB_BkBootSec=bpb.BPB_BkBootSec;

// bs_pbp.BPB_FATSz32*bs_pbp.BPB_NumFATs+bs_pbp.BPB_RsvdSecCnt+mbr.mbr_in[0].strart_chan+(bs_pbp.BPB_RootClis-2)*bs_pbp.BPB_SecPerClus;
    fileSystemInfop->rootNum=fileSystemInfop->MBR_start+
                    fileSystemInfop->BPB_RsvdSecCnt+
                    fileSystemInfop->BPB_FATSz32*fileSystemInfop->BPB_NumFATs+
                    (fileSystemInfop->BPB_RootClis-2)*fileSystemInfop->BPB_SecPerClus;
    u32 start=fileSystemInfop->MBR_start+
                    fileSystemInfop->BPB_RsvdSecCnt;
    for(u32 i=0;i<fileSystemInfop->BPB_NumFATs;i++){
        fileSystemInfop->FAT[i]=start+i*fileSystemInfop->BPB_FATSz32;
    }
    fileSystemInfop->pathNum=fileSystemInfop->BPB_RootClis;
    
    DEBUG("%s ���سɹ�!\n",fileSystemInfop->fileName);
    DEBUG("��Ŀ¼�غ� %d\n",fileSystemInfop->rootNum);
    DEBUG("�� %d %d\n",fileSystemInfop->FAT[0],fileSystemInfop->FAT[1]);
    return SUCCESS;
}