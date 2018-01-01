#include<stdio.h>
#include"fs.h"
#include"tool.h"
#include<memory.h>
#include<ctype.h>
#include<math.h>

int my_write(const ARGP arg,FileSystemInfop fileSystemInfop){
	const char helpstr[]=
"\
����        д�뵱ǰĿ¼�µ�ĳ���ļ������ļ�βΪ����\n\
�﷨��ʽ    close name type [offset]\n\
name        д���ļ���\n\
type        д��ģʽ0�ض� 1׷�� 2����\n\
���Ǹ���д��offset��Ч��Ϊ���ǵ���ʼλ��";
 	// FAT_DS_BLOCK4K fat_ds;
    char name[12];
    int type=-1;
    u32 offset=0;
    if(fileSystemInfop->flag==FALSE){
        strcpy(error.msg,"δָ���ļ�ϵͳ\n\x00");
        printf("δָ���ļ�ϵͳ\n");
        return ERROR;
    }
    switch(arg->len){
        case 3:
            offset=ctoi(arg->argv[1]);
            if(offset==INF){
                goto error;
            }
    	case 2:
            if(nameCheckChange(arg->argv[0],name)==ERROR){
                strcpy(error.msg,"�ļ�����������ڷǷ��ַ�\n\x00");
                printf("�ļ�����������ڷǷ��ַ�\n");
                return ERROR;
            }
            for(int i=0;i<11;i++){
                name[i]=toupper(name[i]);
            }
            name[11]='\0';
            DEBUG("|%s|\n",name);
            type=ctoi(arg->argv[1]);
            if(type==2){
                if(arg->len!=3){
                    goto error;
                }
            }else if(type==INF){
                strcpy(error.msg,"д��ģʽ�Ƿ�\n\x00");
                printf("д��ģʽ�Ƿ�\n");
                return ERROR; 
            }
            break;
    		
        case 1:
            if(strcmp(arg->argv[0],"/?")==0){
    			printf(helpstr);
    			return SUCCESS;
    		}else{
                goto error;
            }
            break;
    	case 0:
    		break;
    	default:
    	error:;
            strcpy(error.msg,"������������\n\x00");
            printf("������������\n");
            return ERROR;
    }
    
    FAT_DS_BLOCK4K fat_ds;
	u32 pathNum=fileSystemInfop->pathNum;
    Opendfilep opendf;
	u32 cut;
    do{
		do_read_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,pathNum));
		for(cut=0;cut<SPCSIZE/32;cut++){
			char lin[12];
			my_strcpy(lin,fat_ds.fat[cut].name,11);
			lin[11]='\0';
			if(fat_ds.fat[cut].name[0]=='\xE5' || fat_ds.fat[cut].name[0]=='\x00'){
                continue;
            }else if(strcmp(lin,name)==0&& (fat_ds.fat[cut].DIR_Attr&ATTR_ARCHIVE)!=0 ){
                for(int i=0;i<OPENFILESIZE;i++){
                    opendf = &(fileSystemInfop->Opendf[i]);
                    if(pathNum == opendf->Dir_Clus && opendf->flag==TRUE && strcmp(opendf->File_name,name)==0){
                         int num=0;
                        char buf[ARGLEN*10];
                        printf("��EOF����\n");
                        while(scanf("%c",&buf[num])!=EOF && buf[num]!=26){
                            num++;
                        }
                        for(int i=0;i<num;i++){
                            DEBUG("%d|",buf[i]);
                        }
                        write_in(i,type,offset,num,(void*)buf,fileSystemInfop);
                        return SUCCESS;
                    }
                }
                printf("�ļ�δ��\n");
                return SUCCESS;
            }
		}
		pathNum=getNext(fileSystemInfop,pathNum);
	}while(pathNum!=FAT_FREE && pathNum!=FAT_END);
    printf("�ļ�������\n");



    return SUCCESS;
}

int write_in(int fnum,int type,u32 start,u32 size,void* buf,FileSystemInfop fileSystemInfop){
    if(fnum<0&&fnum>=OPENFILESIZE){
        return -1;
    }
    FAT_DS_BLOCK4K fat_ds;
    Opendfilep opendf = &(fileSystemInfop->Opendf[fnum]);
    /* д��δ�򿪵��ļ� */
    if(opendf->flag==FALSE){
        return 0;
    }
    do_read_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,opendf->Dir_Clus));
    u32 lin;
    switch(type){
        case TRUNCATION:
            if(fat_ds.fat[opendf->numID].DIR_FileSize!=0){
                lin=(u32)( (((u32)fat_ds.fat[opendf->numID].DIR_FstClusHI)<<16) |(u32)fat_ds.fat[opendf->numID].DIR_FstClusLO );
                while(lin!=FAT_END&&lin!=FAT_SAVE&&lin!=FAT_FREE){
                    lin=delfree(fileSystemInfop,lin);
                }
                fat_ds.fat[opendf->numID].DIR_FileSize=0;
                fat_ds.fat[opendf->numID].DIR_FstClusLO=0;
                fat_ds.fat[opendf->numID].DIR_FstClusHI=0;
                do_write_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,opendf->Dir_Clus));
            }
            return write_real(fnum,0,size,buf,fileSystemInfop);
            break;
        case ADDITIONAL:
            lin=fat_ds.fat[opendf->numID].DIR_FileSize;
            return write_real(fnum,lin,size,buf,fileSystemInfop);
            break;
        case COVER:
            lin=start;
            if(lin+size>fat_ds.fat[opendf->numID].DIR_FileSize){
                size=fat_ds.fat[opendf->numID].DIR_FileSize-lin;
            }
            return write_real(fnum,lin,size,buf,fileSystemInfop);
            break;
        default:
            /* ���ͷǷ� */
            return -2;
    }
}

int write_real(int fnum,int start,int size,void* buf,FileSystemInfop fileSystemInfop){
    FAT_DS_BLOCK4K fat_ds;
    /* �ļ��������Ƿ� */
    if(fnum<0&&fnum>=OPENFILESIZE){
        return -1;
    }
    /* ��ʼλ�÷Ƿ� */
    if(start<0){
        return -2;
    }
    /* д�볤�ȷǷ� */
    if(size<0){
        return -3;
    }else if(size==0){
        return 0;
    }
    Opendfilep opendf = &(fileSystemInfop->Opendf[fnum]);
    /* д��δ�򿪵��ļ� */
    if(opendf->flag==FALSE){
        return 0;
    }
    /* ǿ���ƶ��ļ�ָ�� */
    do_read_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,opendf->Dir_Clus));
	int fileclus = (u32)( (((u32)fat_ds.fat[opendf->numID].DIR_FstClusHI)<<16) |(u32)fat_ds.fat[opendf->numID].DIR_FstClusLO );
    /* 0��չ��־ */
    int flagZero=FALSE;
    if(fileclus==FAT_FREE){
        fileclus=newfree(fileSystemInfop,0);
        fat_ds.fat[opendf->numID].DIR_FstClusHI=(u16)(fileclus>>16);
        fat_ds.fat[opendf->numID].DIR_FstClusLO=(u16)(fileclus&0x0000ffff);
        fat_ds.fat[opendf->numID].DIR_FileSize=SPCSIZE;
        opendf->File_Clus=fileclus;
        flagZero=TRUE;
    }
    while(SPCSIZE*ceil(fat_ds.fat[opendf->numID].DIR_FileSize/(1.0*SPCSIZE))<start){
        fileclus=newfree(fileSystemInfop,fileclus);
        fat_ds.fat[opendf->numID].DIR_FileSize=SPCSIZE*ceil(fat_ds.fat[opendf->numID].DIR_FileSize/(1.0*SPCSIZE))+SPCSIZE;
    }
    opendf->writep=start;
    BLOCK4K block4k;
    /* Ѱ��Ҫд�Ĵ��̿� */
    fileclus=(u32)( (((u32)fat_ds.fat[opendf->numID].DIR_FstClusHI)<<16) |(u32)fat_ds.fat[opendf->numID].DIR_FstClusLO );
    for(int i=0;i<opendf->writep/SPCSIZE;i++){
        fileclus=getNext(fileSystemInfop,fileclus);
    }
    if(flagZero==TRUE){
        fat_ds.fat[opendf->numID].DIR_FileSize=opendf->writep;
    }
    /* ��ʼд�� */
    int len=size;
    int fileclusold;
    /* 4kд�벹�� */
    if(opendf->writep%SPCSIZE!=0){
        do_read_block4k(fileSystemInfop->fp,&block4k,L2R(fileSystemInfop,fileclus));
        int lin;
        if(len<(SPCSIZE-opendf->writep%SPCSIZE)){
            lin=len;
            len=0;
        }else{
            lin=(SPCSIZE-opendf->writep%SPCSIZE);
            len-=lin;
        }
        my_strcpy(&(((char*)&block4k)[(opendf->writep%SPCSIZE)]),(char*)buf,lin);
        do_write_block4k(fileSystemInfop->fp,&block4k,L2R(fileSystemInfop,fileclus));
        opendf->writep+=lin;
        if(opendf->writep>fat_ds.fat[opendf->numID].DIR_FileSize){
            fat_ds.fat[opendf->numID].DIR_FileSize=opendf->writep;
        }
        fileclusold=fileclus;
        fileclus=getNext(fileSystemInfop,fileclus);
    }
    while(len!=0){
        /* ûд�굫�������һ�� */
        if(fileclus==FAT_END||fileclus==FAT_SAVE||FAT_FREE){
            fileclus=newfree(fileSystemInfop,fileclusold);
        }
        do_read_block4k(fileSystemInfop->fp,&block4k,L2R(fileSystemInfop,fileclus));
        int lin;
        if(len<SPCSIZE){
            lin=len;
            len=0;
        }else{
            lin=SPCSIZE;
            len-=lin;
        }
        my_strcpy(&(((char*)&block4k)[(opendf->writep%SPCSIZE)]),(char*)buf,lin);
        do_write_block4k(fileSystemInfop->fp,&block4k,L2R(fileSystemInfop,fileclus));
        opendf->writep+=lin;
        if(opendf->writep>fat_ds.fat[opendf->numID].DIR_FileSize){
            fat_ds.fat[opendf->numID].DIR_FileSize=opendf->writep;
        }
        fileclusold=fileclus;
        fileclus=getNext(fileSystemInfop,fileclus);
    }
    do_write_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,opendf->Dir_Clus));
    return size-len;
}
