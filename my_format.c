#include"fs.h"
#include"tool.h"
#include<stdlib.h>
#include<string.h>
/*
    ����һ������ ����С��256MB ���ô���2097152MB(2TB)
    Ĭ��һ�����̿�512B һ��4KB 8����
*/
#define MIN 256
#define MAX 2097152
//��λ�ֽ�
void vhdset(HD_FTRp vhd,u64 size){
    strcpy(vhd->cookie,"conectix");
    vhd->features=BigtoLittle32(0x00000002);
    vhd->ff_version=BigtoLittle32(0x00010000);
    vhd->data_offset=BigtoLittle64(0xFFFFFFFFFFFFFFFF);
    vhd->timestamp=0x00000000;
    strcpy(vhd->crtr_app,"myfs");
    vhd->crtr_ver=BigtoLittle32(0x00060001);
    // strcpy((char*)vhd->crtr_os,"Wi2k");
    vhd->crtr_os=BigtoLittle32(0x5769326b);
    vhd->orig_size=BigtoLittle64(size);
    vhd->curr_size=BigtoLittle64(size);
    vhd->geometry=BigtoLittle32(0x03eb0c11);
    vhd->type=BigtoLittle32(2); //����
    vhd->checksum=0;
    // strcmp(vhd->uuid,"                ");
    memset(vhd->uuid,0xff,sizeof(vhd->uuid));
    memset(vhd->reserved,0,sizeof(vhd->reserved));
    u32 chksum=0;
    u8* p=(u8*)vhd;
    for(int i=0;i<512;i++){
        chksum+=p[i];
    }
    vhd->checksum=BigtoLittle32(~chksum);
}
//sizeΪ�ֽ���
void MBRset(MBRp mbr,int size){
    mbr->sign=0x12345678;
    (mbr->mbr_in[0]).flag=0;
    (mbr->mbr_in[0]).FSflag=0x0b;
    mbr->mbr_in[0].strart_chan=0x00000008;
    mbr->mbr_in[0].all=size/BLOCKSIZE-mbr->mbr_in[0].strart_chan;
    mbr->end=0xAA55;
}
void FSInfoset(FSInfop fsi){
    memset(fsi,0,BLOCKSIZE);
    fsi->FSI_LeadSig=0x41615252;
    fsi->FSI_StrucSig=0x61417272;
    fsi->FSI_Free_Count=0xff;
    fsi->FSI_Nxt_free=0xFFFFFFFF;
    fsi->end=0xAA550000;
}

//size Ϊ�ֽ���
void BS_BPSset(BS_BPBp bs_bpb,int size,int hiden){
    my_strcpy(bs_bpb->BS_jmpBoot,"\xEB\x58\x90",sizeof(bs_bpb->BS_jmpBoot));
    my_strcpy(bs_bpb->BS_OEMName,"MSDOS5.0",sizeof(bs_bpb->BS_OEMName));
    bs_bpb->BPB_BytsPerSec=512;
    bs_bpb->BPB_SecPerClus=8;//
    bs_bpb->BPB_RsvdSecCnt=32;
    bs_bpb->BPB_NumFATs=2;
    bs_bpb->BPB_RootEntCnt=0;
    bs_bpb->BPB_TotSec16=0;
    bs_bpb->BPB_Media=0xf8;
    bs_bpb->BPB_FATSz16=0;
    bs_bpb->BPB_SecPerTrk=0x3f;
    bs_bpb->BPB_NumHeads=0xff;
    bs_bpb->BPB_HiddSec=hiden;
    bs_bpb->BPB_TotSec32=size/BLOCKSIZE;

    int x=size/SPCSIZE/(512/4);
    bs_bpb->BPB_FATSz32=x+(8-x%8);//ǿ��4k����
    bs_bpb->BPB_ExtFlags=0;
    bs_bpb->BPB_FSVer=0;
    bs_bpb->BPB_RootClis=2;
    bs_bpb->BPB_FSInfo=1;
    bs_bpb->BPB_BkBootSec=6;
    memset(bs_bpb->BPB_Reserved,0,sizeof(bs_bpb->BPB_Reserved));
    bs_bpb->BS_DrvNum=0x80;
    bs_bpb->BS_Reserved1=0;
    bs_bpb->BS_BootSig=0x29;
    // char BS_VolID[4];
    my_strcpy(bs_bpb->BS_VolLab,"NO NAME    ",11);
    my_strcpy(bs_bpb->BS_FilSysType,"FAT32    ",8);
    bs_bpb->end=0xAA55;
}

int my_format(const ARGP arg){
    char fatName[8]="WLT    ";
    char fileName[ARGLEN]="fs.vhd";
    const char helpstr[]=
"\
����         ��ʽ���ļ�ϵͳ\n\
�﷨��ʽ     format size [name [namefile]]\n\
szie        ���̴�С ��λMB\n\
name        �����  Ĭ�� WTL\n\
namefile    ��������ļ�·������ǰĿ¼�¿�ʼ�� Ĭ�� fs.vhd\n";
    DEBUG("%d",sizeof(BS_BPB));
    BLOCK4K block4k;
    FILE *fp=NULL;
    switch(arg->len){
        case 3:
            my_strcpy(fileName,arg->argv[2],ARGLEN);
        case 2:
            my_strcpy(fatName,arg->argv[1],8);
        case 1:
            if(strcmp(arg->argv[0],"/?")==0 && arg->len==1){
                printf(helpstr);
                return SUCCESS;
            }else{
                break;
            }
            break;
        default:
            strcpy(error.msg,"������������\n\x00");
            printf("������������\n");
            return ERROR;
    }
    // if(arg->len>3){
    //     strcpy(error.msg,"������������\n\x00");
    //     printf("������������\n");
    //     return ERROR;
    // }
    
    DEBUG("%s %d",arg->argv[0],ctoi(arg->argv[0]));
    int size=ctoi(arg->argv[0]);
    if(size<MIN||size>MAX){
        strcpy(error.msg,"��������������\n\x00");
        printf("��������������\n");
        return ERROR;
    }
    int cut=size*K*K/SPCSIZE;
    DEBUG("%d %d\n",sizeof(BLOCK),sizeof(BLOCK4K));
    fp=fopen(fileName,"wb");
    if(fp==NULL){
        strcpy(error.msg,"�ļ��򿪴���\n\x00");
        printf("�ļ���������\n");
        return ERROR;
    }
    //�����ļ�����
    memset(&block4k,0,SPCSIZE);
    for(int i=0;i<cut;i++){
        do_write_block4k(fp,&block4k,-1);
    }
    //����vhd��ʽ
    HD_FTR vhd;
    vhdset(&vhd,size*K*K);
    do_write_block(fp,(BLOCK*)&vhd,-1,0);
    
    //���̷���
    MBR mbr;
    MBRset(&mbr,size*K*K);
    do_write_block(fp,(BLOCK*)&mbr,0,0);

    //fatBPB
    BS_BPB bs_pbp;
    BS_BPSset(&bs_pbp,
        size*K*K-mbr.mbr_in[0].strart_chan*BLOCKSIZE,
        mbr.mbr_in[0].strart_chan);
    do_write_block(fp,(BLOCK*)&bs_pbp,mbr.mbr_in[0].strart_chan*BLOCKSIZE/SPCSIZE,(mbr.mbr_in[0].strart_chan*BLOCKSIZE%SPCSIZE)/BLOCKSIZE);
    
    //��ʼ����Ŀ¼
    int start=bs_pbp.BPB_FATSz32*bs_pbp.BPB_NumFATs+bs_pbp.BPB_RsvdSecCnt+mbr.mbr_in[0].strart_chan+(bs_pbp.BPB_RootClis-2)*bs_pbp.BPB_SecPerClus;
    DEBUG("��ʼ���� %d\n",start);

    BLOCK block;
    FAT_DSp fatdsp=(FAT_DSp)&block;
    memset(fatdsp,0,sizeof(FAT_DS));
    my_strcpy(fatdsp->name,fatName,8);
    my_strcpy(fatdsp->named,"   ",3);
    fatdsp->DIR_Attr=ATTR_VOLUME_ID;
    do_write_block(fp,&block,start/8,start%8);

    FSInfo fsi;
    FSInfoset(&fsi);
    // do_write_block(fp,(BLOCK*)&bs_pbp,mbr.mbr_in[0].strart_chan*BLOCKSIZE/SPCSIZE,(mbr.mbr_in[0].strart_chan*BLOCKSIZE%SPCSIZE)/BLOCKSIZE);
    do_write_block(fp,(BLOCK*)&fsi,mbr.mbr_in[0].strart_chan*BLOCKSIZE/SPCSIZE,(mbr.mbr_in[0].strart_chan*BLOCKSIZE%SPCSIZE)/BLOCKSIZE+1);
    
    //fat���� (��Ŀ¼��ʼ��)
    int str_fat1;
    str_fat1=bs_pbp.BPB_RsvdSecCnt+mbr.mbr_in[0].strart_chan;
    int str_fat2=bs_pbp.BPB_RsvdSecCnt+bs_pbp.BPB_FATSz32+mbr.mbr_in[0].strart_chan;
    DEBUG("%d %d\n",str_fat1,str_fat2);
    FAT fat;
    fat.fat[0]=0x0ffffff8;
    for(u32 i=1;i<=bs_pbp.BPB_RootClis;i++){
        fat.fat[i]=FAT_END;
    }
    // fat.fat[bs_pbp.BPB_RootEntCnt]=
    do_write_block(fp,(BLOCK*)&fat,str_fat1/8,str_fat1%8);
    do_write_block(fp,(BLOCK*)&fat,str_fat2/8,str_fat2%8);

    fclose(fp);
    DEBUG("��������ɹ�\n");
    // if()
    return SUCCESS;
}
