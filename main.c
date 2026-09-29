#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PASSWORD "885566"

int check_pass(void)
{
    char input[64];
    printf("\n🔒 请输入密码打开文件：");
    scanf("%s", input);
    if(strcmp(input, PASSWORD) == 0)
    {
        printf("✅ 密码正确，正在打开文件...\n");
        return 1;
    }
    else
    {
        printf("❌ 密码错误！返回主菜单\n");
        return 0;
    }
}

void menu1(void)
{
    FILE *fp = fopen("/sdcard/fileA.txt", "r");
    if(fp == NULL)
    {
        printf("⚠️ 文件A不存在\n");
        return;
    }
    char buf[256];
    while(fgets(buf, sizeof(buf), fp))
    {
        printf("%s", buf);
    }
    fclose(fp);
}

void menu2(void)
{
    FILE *fp = fopen("/sdcard/fileB.txt", "r");
    if(fp == NULL)
    {
        printf("⚠️ 文件B不存在\n");
        return;
    }
    char buf[256];
    while(fgets(buf, sizeof(buf), fp))
    {
        printf("%s", buf);
    }
    fclose(fp);
}

void show_menu(void)
{
    int opt;
    while(1)
    {
        printf("\n====================\n");
        printf("      主菜单\n");
        printf(" 1. 读取文件A\n");
        printf(" 2. 读取文件B\n");
        printf(" 0. 退出程序\n");
        printf("====================\n");
        printf("请输入选项：");
        scanf("%d", &opt);

        switch(opt)
        {
            case 1:
                if(check_pass()){
                    menu1();
                }
                break;
            case 2:
                if(check_pass()){
                    menu2();
                }
                break;
            case 0:
                printf("👋 退出\n");
                exit(0);
            default:
                printf("⚠️ 无效选项\n");
        }
    }
}

int main(void)
{
    show_menu();
    return 0;
}
