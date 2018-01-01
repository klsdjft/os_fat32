#include<stdio.h>
#include"fs.h"
#include"tool.h"
#include<memory.h>
#include<ctype.h>
//��ȡ�ļ���󳤶�Ϊ4098�ֽ�
#define MAXLEN 4098
int my_read(const ARGP arg,FileSystemInfop fileSystemInfop){
	char name[12]; 
	u32 start=0;
	/* 0Ϊ��ȡ���� */
	u32 len=0; 
	const char helpstr[]=
"\
����        ��ȡ�ļ�����\n\
�﷨��ʽ    read name [len[start]]\n\
name	   Ҫ�����ļ���\n\
len			��ȡ���ļ����� Ĭ��Ϊ����\n\
start		��ȡ�Ŀ�ʼλ�� Ĭ��Ϊ0\n";
 	FAT_DS_BLOCK4K fat_ds;
    if(fileSystemInfop->flag==FALSE){
        strcpy(error.msg,"δָ���ļ�ϵͳ\n\x00");
        printf("δָ���ļ�ϵͳ\n");
        return ERROR;
    }
    switch(arg->len){
		case 3:
			start=ctoi(arg->argv[2]);
			if(start==INF){
				goto error;
			}
		case 2:
			len=ctoi(arg->argv[1]);
			if(len==INF){
				goto error;
			}
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
    	case 0:
    		break;
    	default:
    	error:;
            strcpy(error.msg,"������������\n\x00");
            printf("������������\n");
            return ERROR;
    }
    
    
	u32 pathNum=fileSystemInfop->pathNum;
	u32 cut;
	Opendfilep opendf;
	do{
		do_read_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,pathNum));
		for(cut=0;cut<SPCSIZE/32;cut++){
			char lin[12];
			my_strcpy(lin,fat_ds.fat[cut].name,11);
			lin[11]='\0';
			if(fat_ds.fat[cut].name[0]=='\xe5'){
				//��ɾ����
				continue;
			}
            if((fat_ds.fat[cut].DIR_Attr&ATTR_ARCHIVE) && strcmp(name,lin)==0){ 
                //�ļ�
				for(int i=0;i<OPENFILESIZE;i++){
                    opendf = &(fileSystemInfop->Opendf[i]);
                    if(pathNum == opendf->Dir_Clus && opendf->flag==TRUE && strcmp(opendf->File_name,name)==0){
                        char buf[ARGLEN*10];
                        int lin=read_real(i,start,len,(void*)buf,fileSystemInfop);
                        for(int i=0;i<lin;i++){
							printf("%c",buf[i]);
						}
						printf("\n");
						return SUCCESS;
                    }
                }
			}
		}
		pathNum=getNext(fileSystemInfop,pathNum);
	}while(pathNum!=FAT_FREE && pathNum!=FAT_END);
	printf("�ļ�������\n");
	return SUCCESS;
}

int read_real(int fnum,int start,int size,void* buf,FileSystemInfop fileSystemInfop){
	FAT_DS_BLOCK4K fat_ds;
	BLOCK4K block4k;
    /* �ļ��������Ƿ� */
    if(fnum<0&&fnum>=OPENFILESIZE){
        return -1;
    }
	Opendfilep opendf = &(fileSystemInfop->Opendf[fnum]);

	do_read_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,opendf->Dir_Clus));
	if(start>fat_ds.fat[opendf->numID].DIR_FileSize){
		return -1;
	}
	if(size==0){
		size=fat_ds.fat[opendf->numID].DIR_FileSize;
	}
	if(start+size>fat_ds.fat[opendf->numID].DIR_FileSize){
		size=fat_ds.fat[opendf->numID].DIR_FileSize-start;
	}
	
	int where=SPCSIZE;
	u32 fileclus=opendf->File_Clus;
	u32 fileclusold=0;
	if(where<start){
		fileclus=getNext(fileSystemInfop,fileclus);
		where+=SPCSIZE;
	}
	u32 len=size;
	opendf->readp=start;
    /* 4k�����벹�� */
    if(opendf->readp%SPCSIZE!=0){
        do_read_block4k(fileSystemInfop->fp,&block4k,L2R(fileSystemInfop,fileclus));
        int lin;
        if(len<(SPCSIZE-opendf->readp%SPCSIZE)){
            lin=len;
            len=0;
        }else{
            lin=(SPCSIZE-opendf->readp%SPCSIZE);
            len-=lin;
        }
        my_strcpy((char*)buf,&(((char*)&block4k)[(opendf->readp%SPCSIZE)]),lin);
        opendf->readp+=lin;
        fileclusold=fileclus;
        fileclus=getNext(fileSystemInfop,fileclus);
    }
	while(len!=0){
        do_read_block4k(fileSystemInfop->fp,&block4k,L2R(fileSystemInfop,fileclus));
        int lin;
        if(len<SPCSIZE){
            lin=len;
            len=0;
        }else{
            lin=SPCSIZE;
            len-=lin;
        }
        my_strcpy((char*)buf,&(((char*)&block4k)[(opendf->readp%SPCSIZE)]),lin);
        opendf->readp+=lin;
        fileclusold=fileclus;
        fileclus=getNext(fileSystemInfop,fileclus);
    }
	return size-len;

}

// int my_read(int fd,int len,FileSystemInfop fileSystemInfop){

// 	char text[MAXLEN];
// 	int readBytes;
// 	Opendfilep opendf;
	
// 	if( fd >=OPENFILESIZE || fd < 0 ){
// 		printf("%s\n","�ļ����������Ϸ�");
// 		return ERROR;
// 	}
// 	opendf = &(fileSystemInfop->Opendf[fd]);

// 	if( opendf->flag == FALSE ){
// 		printf("%s\n","�����ļ���������Ч");
// 		return ERROR;
// 	}else{
// 		u32 pathNum=opendf->Dir_Clus;
// 		FAT_DS_BLOCK4K fat_ds;
// 		readBytes = do_read(fd,len,text);
// 		//text[readBytes] = '\0';
// 		//������������Ļ
// 		for(int i=0;text[i]!='\0';i++){
// 			printf("%s",text[i]);
// 		}

// 	}



// }
