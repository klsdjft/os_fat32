#include<stdio.h>
#include"fs.h"
#include"tool.h"
#include<memory.h>
#include<ctype.h>

int my_close(const ARGP arg,FileSystemInfop fileSystemInfop){
	char name[12];
	const char helpstr[]=
"\
����        �رյ�ǰĿ¼��ĳ���ļ�\n\
�﷨��ʽ    close name\n\
		   \n";
 	// FAT_DS_BLOCK4K fat_ds;
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
	FAT_DS_BLOCK4K fat_ds;
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
			if((fat_ds.fat[cut].DIR_Attr&ATTR_ARCHIVE)&&strcmp(lin,name)==0){ 
                //�ļ�
				for(int i=0;i<OPENFILESIZE;i++){
                    opendf = &(fileSystemInfop->Opendf[i]);
                    if(pathNum == opendf->Dir_Clus && opendf->flag==TRUE && strcmp(opendf->File_name,name)==0){
						close_in(i,fileSystemInfop);
                        return SUCCESS;
                    }
                }
                printf("�ļ�δ��\n");
                return SUCCESS;
			}
		}
		pathNum=getNext(fileSystemInfop,pathNum);
	}while(pathNum!=FAT_FREE && pathNum!=FAT_END);

    return SUCCESS;

}

int close_in(int fnum,FileSystemInfop fileSystemInfop){
    /* �ļ��������Ƿ� */
    if(fnum<0&&fnum>=OPENFILESIZE){
        return ERROR;
    }
    Opendfilep opendf = &(fileSystemInfop->Opendf[fnum]);
	opendf->flag=FALSE;
	return SUCCESS;
}