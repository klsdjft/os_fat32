#include<stdio.h>
#include"fs.h"
#include"tool.h"
#include<memory.h>
#include<ctype.h>

int my_cd(const ARGP arg,FileSystemInfop fileSystemInfop){
	char name[12]; 
	const char helpstr[]=
"\
����        �����ļ���\n\
�﷨��ʽ    cd name\n\
name       �����ļ��е�����\n\
��ע       �ļ���ǿ��תΪ��д���ļ����������8λ\n";
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
    			//DEBUG("hear\n");
    			memset(name,' ',12);
    			my_strcpy(name,arg->argv[0],strlen(arg->argv[0]));
    			name[11]='\0';
    			for(int i=0;i<11;i++){
    				name[i]=toupper(name[i]);
    			}
    			//DEBUG("%s|\n",name);
				break;
    		} 
    	case 0:
    		DEBUG("δ�����ļ���\n");
    		return SUCCESS;
    	default:
    	error:;
            strcpy(error.msg,"������������\n\x00");
            printf("������������\n");
            return ERROR;
    }
    
    
	u32 pathNum=fileSystemInfop->pathNum;
	u32 cut;
//	while(TRUE){
	do{
		do_read_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,pathNum));
		for(cut=0;cut<SPCSIZE/32;cut++){
			char lin[12];
			my_strcpy(lin,fat_ds.fat[cut].name,11);//�ǻ�ȡ�ļ������ƣ�����
			lin[11]='\0';
			
//			if(fat_ds.fat[cut].DIR_Attr==0){//��ʲô����������� 
//   				break;
//			}else 
		//	DEBUG("in1%s|\n",lin);
			//DEBUG("in2%s|\n",name);
			if((fat_ds.fat[cut].DIR_Attr&ATTR_DIRECTORY) 
					&& strcmp(name,lin)==0 ){//ǰ�벿��ʲô��˼��������read�ľ��岽�� 
				fileSystemInfop->pathNum=(u32)( (((u32)fat_ds.fat[cut].DIR_FstClusHI)<<16)
																	| (u32)fat_ds.fat[cut].DIR_FstClusLO );
				//DEBUG("%u\n",fileSystemInfop->pathNum);
				//DEBUG("%u\n",fat_ds.fat[cut].DIR_FstClusHI);
				//DEBUG("%u\n",fat_ds.fat[cut].DIR_FstClusLO);
				if(strcmp(lin,".          ")==0){

				}else if (strcmp(lin,"..         ")==0){
					for(int i=strlen(fileSystemInfop->path)-1;i>=0;i--){
						if(fileSystemInfop->path[i]=='/'){
							fileSystemInfop->path[i]=0x00;
							break;
						}
					}
				}else{
					strcat(fileSystemInfop->path,"/");
					strcat(fileSystemInfop->path,lin);
					for(int i=strlen(fileSystemInfop->path)-1;i>=0;i--){
						if(fileSystemInfop->path[i]==' '){
							fileSystemInfop->path[i]=0x00;
						}else{
							break;
						}
					}
				}
				
				// printf("�ɹ�\n");
				return SUCCESS;	    	
			}else{
				continue;
			}
		}
		pathNum=getNext(fileSystemInfop,pathNum);
	}while(pathNum!=FAT_FREE && pathNum!=FAT_END);
	printf("�ļ��в�����\n");
	return SUCCESS;
}
