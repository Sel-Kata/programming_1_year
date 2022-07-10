#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    printf("hello world");
    FILE *file;
    if (argc == 3 && strcmp(argv[2], " ") != 0)
    {
        char ch;
        int count_cl = 0;
        int count_str = 0;
        int count_cr = 0;
        int condition = 0;
        int res_sr_cr = 0;
        /**int count_sr_cr=0;*/
        int count=0;

        file = fopen("test_lab.txt", "r");

        while ((ch = fgetc(file)) != EOF)
        {

            count_cr = count_cr + 1;
            count =  count+1;
            if (isspace(ch)) /**if (ch == " "|| ch == "\n" ||  ch == "\t")*/
            {
                if (ch == '\n')
                    count_str = count_str + 1;

                condition = 1;
            }
            else if (condition == 1)
            {
                condition = 0;
                ++count_cl; /**подсчёт слов*/
            }
        }
        if (condition == 1)
        {
            ++count_cl; /**подсчёт подследнего слова*/
            /*--count_str; но уменьшаем строки так как перенос стоит, а следующая строка пустая*/
        }
        res_sr_cr = count_cr/count_str;/**мы делим сумму кол-ва байт по стокам/кол-во строк*/
    
        if (strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--lines") == 0)
        {
            printf("Line amount: %d\n", count_str);
        }
        else if (strcmp(argv[1], "-w") == 0 || strcmp(argv[1], "--words") == 0)
        {
            printf("Word amount: %d\n", count_cl);
        }
        else if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--bytes") == 0)
        {
            printf("Byte amount: %d\n", count_cr);
        }
        else if (strcmp(argv[1], "-srb") == 0 || strcmp(argv[1], "--srbytes") == 0)
        {
            printf("Avarage amount of byte: %d\n", res_sr_cr);
        }
        else
        {
            fprintf(stderr, "Your argument count is incorrect\n");
            return -1;
        }
        fclose(file);
    }
    else
    {
        fprintf(stderr, "Invalid number of arguments\n");
        return -1;
    }
    return 0;
}
