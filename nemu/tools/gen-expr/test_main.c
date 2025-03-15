#include <stdio.h>
int main()
{
    FILE *file;

    // 打开文件以读取
    file = fopen("./tools/gen-expr/build/input", "r");
    if (file == NULL)
    {
        printf("无法打开文件\n");
        return -1;
    }
    char expresion[65536];
    int res;
    for (int i = 0; i < 65536; i++)
    {
        expresion[i] = '\0';
    }
    int count = 0;
    // 逐行读取数据，直到文件末尾
    while (fscanf(file, "%u %s", &res, expresion) == 2)
    {
        if (res != expr(expresion, NULL))
        {
            printf("No : %s\n correct : %u\n now : %u\n", expresion, res, expr(expresion, NULL));
        }
        count++;
    }

    // 关闭文件
    fclose(file);
    printf("finish %d test!\n", count);
}