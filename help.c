#include<stdio.h>
#include"fs.h"
const char help_str[]="�ļ�ϵͳ\n\
����fat32��ע�ļ�������Ϊ������̹���\n\
����      ����\n\
format    ��ʽ���ļ�ϵͳ\n\
load      �����ļ�ϵͳ\n\
mkdir     �����ļ���\n\
cd        �����ļ���\n\
create    �����ļ�\n\
dir       �г���ǰĿ¼���ļ�\n\
rm        �Ƴ��ļ�\n\
rmdir     �Ƴ����ļ���\n\
open      ���ļ�\n\
close     �ر��ļ�\n\
write     д�ļ�\n\
read      ���ļ�\n\
help      ��ʾ��ʾ\n\
\n";

int my_help(){
    printf("%s",help_str);
    return SUCCESS;
}