#include"fs.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int do_write_block4k(FILE*fp,BLOCK4K* block4k,int offset){
    if(offset==-1){
        return fwrite(block4k,sizeof(BLOCK4K),1,fp);
    }else{
        fseek(fp,offset*SPCSIZE,SEEK_SET);
        return fwrite(block4k,sizeof(BLOCK4K),1,fp);
    }
}
int do_write_block(FILE*fp,BLOCK* block,int offset,int num){
    if(offset==-1){
        return fwrite(block,sizeof(BLOCK),1,fp);
    }else{
        fseek(fp,offset*SPCSIZE+num*BLOCKSIZE,SEEK_SET);
        return fwrite(block,sizeof(BLOCK),1,fp);
    }
}

int do_read_block4k(FILE*fp,BLOCK4K* block4k,int offset){
    if(offset==-1){
        return fread(block4k,sizeof(BLOCK4K),1,fp);
    }else{
        fseek(fp,offset*SPCSIZE,SEEK_SET);
        return fread(block4k,sizeof(BLOCK4K),1,fp);
    }
}
int do_read_block(FILE*fp,BLOCK* block,int offset,int num){
    if(offset==-1){
        return fread(block,sizeof(BLOCK),1,fp);
    }else{
        fseek(fp,offset*SPCSIZE+num*BLOCKSIZE,SEEK_SET);
        return fread(block,sizeof(BLOCK),1,fp);
    }
}

u32 L2R(FileSystemInfop fsip,u32 num){
    // DEBUG("lj %u->%u\n",num,(fsip->MBR_start+
            // fsip->BPB_RsvdSecCnt+
            // fsip->BPB_FATSz32*fsip->BPB_NumFATs)/fsip->BPB_SecPerClus+num-2);
    return (fsip->MBR_start+
            fsip->BPB_RsvdSecCnt+
            fsip->BPB_FATSz32*fsip->BPB_NumFATs)/fsip->BPB_SecPerClus+num-2;
}

u32 getNext(FileSystemInfop fsip,u32 num){
    if(num/(512/4)>fsip->BPB_FATSz32){
        return 0;
    }
    u32 cuNum=num/(512/4);
    u32 index=num%(512/4);
    FAT fat;
    do_read_block(fsip->fp,(BLOCK*)&fat,(fsip->FAT[0]+cuNum)/8,(fsip->FAT[0]+cuNum)%8);
    return fat.fat[index];
}

int newfree(FileSystemInfop fsip,u32 num){
    FAT fat;
    u32 cuNum=num/(512/4);
    u32 index=num%(512/4);
    for(u32 i=0;i<fsip->BPB_FATSz32;i++){
        do_read_block(fsip->fp,(BLOCK*)&fat,(fsip->FAT[0]+i)/8,(fsip->FAT[0]+i)%8);
        for(int j=0;j<512/4;j++){
            if(fat.fat[j]==FAT_FREE){
                fat.fat[j]=FAT_END;
                do_write_block(fsip->fp,(BLOCK*)&fat,(fsip->FAT[0]+i)/8,(fsip->FAT[0]+i)%8);
                if(num!=0){
                    do_read_block(fsip->fp,(BLOCK*)&fat,(fsip->FAT[0]+cuNum)/8,(fsip->FAT[0]+cuNum)%8);
                    fat.fat[index]=i*(512/4)+j;
                    do_write_block(fsip->fp,(BLOCK*)&fat,(fsip->FAT[0]+cuNum)/8,(fsip->FAT[0]+cuNum)%8);
                }
                return i*(512/4)+j;
            }
        }
    }
    return 0;
}

char* my_strcpy(char *to,const char*from,int size ){
    for(int i=0;i<size;i++){
        to[i]=from[i];
    }
    return to;
}

//ȡ�ò��� 
int getargv(ARGP argp){
    char cmd[ARGLEN*10];
    if(gets(cmd)==NULL){
        argp->len=0;
        return SUCCESS;
    }
    int num=0;
    int len=0;
    int flag=0;
    for(u32 i=0;i<strlen(cmd);i++){
        if(len>=ARGLEN){
            return ERROR;
        }
        if(num>=ARGNUM){
            return ERROR;
        }
        if(cmd[i]==' ' ||cmd[i]=='\t' ||cmd[i]=='\r'){
            flag=0;
            continue;
        }
        if(flag==0){
            num++;
            len=0;
        }
        (argp->argv)[num-1][len]=cmd[i];
        (argp->argv)[num-1][len+1]=0;
        len++;
        flag=1;
    }
    argp->len=num;
    return SUCCESS;
}


//�ַ���ת������ ���󷵻�INF
int ctoi(const char* ch){
    int ret=0;
    for(u32 i=0;i<strlen(ch);i++){
        ret*=10;
        if('0'<=ch[i]&&ch[i]<='9'){
            ret+=ch[i]-'0';
        }else{
            return INF;
        }
    }
    return ret;
}