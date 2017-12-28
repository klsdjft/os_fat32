#include<stdio.h>
#include"fs.h"
#include"tool.h"
#include<memory.h>
#include<ctype.h>


int isEmpty(FileSystemInfop fileSystemInfop,u32 pathNum){
	u32 cut;
	FAT_DS_BLOCK4K fat_ds;
	// u32 delfileNum;
    do{
		do_read_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,pathNum));
		for(cut=0;cut<SPCSIZE/32;cut++){
			if(fat_ds.fat[cut].name[0]=='\xe5' || fat_ds.fat[cut].name[0]=='\x00'){
				//��ɾ���� ���
				continue;
			}else{
                char name[12];
                my_strcpy(name,fat_ds.fat[cut].name,11);
                name[11]='\0';
                DEBUG("|%s|\n",name);
                DEBUG("%s\n",DIR_d);
                DEBUG("%s\n",DIR_dd);
                if(strcmp(name,DIR_d)==0 || strcmp(name,DIR_dd)==0){
                    continue;
                }else{
                    return FALSE;
                }
            }
		}
		pathNum=getNext(fileSystemInfop,pathNum);
	}while(pathNum!=FAT_FREE && pathNum!=FAT_END);
	return TRUE;
}



//��ʱ�趨Ϊֻ��ɾ����ǰĿ¼�µ��ļ�,�������ǿ�Ŀ¼
int my_rmdir(const ARGP arg,FileSystemInfop fileSystemInfop){
	char delname[12];
	const char helpstr[]=
"\
����		ɾ���ļ���\n\
��ʽ		rm name\n\
name	  ��Ҫɾ�����ļ�����\n";

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
				if(nameCheckChange(arg->argv[0],delname)==ERROR){
                    strcpy(error.msg,"�ļ�������������ڷǷ��ַ�\n\x00");
                    printf("�ļ�������������ڷǷ��ַ�\n");
                    return ERROR;
                }
                for(int i=0;i<11;i++){
                    delname[i]=toupper(delname[i]);
                }
                delname[11]='\0';
				DEBUG("|%s|\n",delname);
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
	FAT_DS_BLOCK4K fat_ds;
	u32 delfileNum;

	do{
		do_read_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,pathNum));
		for(cut=0;cut<SPCSIZE/32;cut++){
			char name[12];
			my_strcpy(name,fat_ds.fat[cut].name,11);
			name[11]='\0';
			if(fat_ds.fat[cut].name[0]=='\xe5'){
				//��ɾ����
				continue;
			}
			DEBUG("|%s|\n|%s|\n",delname,name);
			if( (fat_ds.fat[cut].DIR_Attr&ATTR_DIRECTORY) && strcmp(delname,name)==0 ){
				//�ҵ���Ҫɾ��Ŀ¼
                delfileNum=(u32)( (((u32)fat_ds.fat[cut].DIR_FstClusHI)<<16) |(u32)fat_ds.fat[cut].DIR_FstClusLO );
                if(isEmpty(fileSystemInfop,delfileNum)==FALSE){
                    //���ж�
                    strcpy(error.msg,"�ļ��зǿ�\n\x00");
                    printf("�ļ��зǿ�\n");
                    return ERROR;
                }
                while(delfileNum!=FAT_END && delfileNum!=FAT_FREE){
					delfileNum=delfree(fileSystemInfop,delfileNum);
				}
				fat_ds.fat[cut].name[0]='\xe5';
				do_write_block4k(fileSystemInfop->fp,(BLOCK4K*)&fat_ds,L2R(fileSystemInfop,pathNum));
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