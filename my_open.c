#include<stdio.h>
#include"fs.h"
#include"tool.h"
#include<memory.h>
#include<ctype.h>
int my_open(const ARGP arg,FileSystemInfop fileSystemInfop){
	char name[12]; 
	const char helpstr[]=
"\
����        �򿪵�ǰĿ¼���ļ�\n\
�﷨��ʽ    open name\n\
		   \n";
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
				DEBUG("|%s|\n",name);
				break;
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
	u32 fileclus;
	Opendfilep opendf;
	do{
		do_read_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,pathNum));
		for(cut=0;cut<SPCSIZE/32;cut++){
			char lin[12];
			my_strcpy(lin,fat_ds.fat[cut].name,11);
			lin[11]='\0';
            
            if(strcmp(lin,name)==0){
            	if(fat_ds.fat[cut].DIR_Attr&ATTR_DIRECTORY){
            		printf("���ܴ��ļ���\n");
            		strcpy(error.msg,"���ܴ��ļ���\n\x00");
            		return SUCCESS;
            	} 
            	else{
            		fileclus = (u32)( (((u32)fat_ds.fat[cut].DIR_FstClusHI)<<16) |(u32)fat_ds.fat[cut].DIR_FstClusLO );
            		for(int i =0;i<OPENFILESIZE;i++){
            			//ֻ�ܴ�һ���ļ�
            			opendf = &(fileSystemInfop->Opendf[i]);
            			if(opendf->flag == TRUE){
            				if((opendf->Dir_Clus == pathNum)&&(opendf->File_Clus ==fileclus) && (strcmp(opendf->File_name,name)==0)){
            					printf("ֻ�ܴ�һ���ļ�\n");
            					return SUCCESS;
            				}
            				else continue;
            			}else{
            				opendf->flag = TRUE;
            				opendf->Dir_Clus = pathNum;
            				opendf->File_Clus = fileclus;
							opendf->readp = 0;
							opendf->writep = 0;
							opendf->numID = cut;
            				strcpy(opendf->File_name,name);
	            			printf("%s%d\n","���ļ��ɹ� �ļ���������",i);
	            			return SUCCESS;
            			}
            		}
            		printf("���ļ����Ѵﵽ��󣬴�ʧ��\n");
            		return SUCCESS;


                }
            }else{
				continue;
			}
        }
		pathNum=getNext(fileSystemInfop,pathNum);
	}while(pathNum!=FAT_FREE && pathNum!=FAT_END);
	printf("δ�ҵ�Ŀ���ļ�����ʧ��!\n");
	return SUCCESS;
}