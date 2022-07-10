#include <stdio.h>
#include <stdlib.h>
#include <iso646.h>
#include <string.h>
#include <locale.h>

// 10 байт занимаемых хэддером
const int bytes_in_frame = 11; // кол-во байт занимаемых фрэймом

#pragma pack(push, 1)
typedef struct id3v2Header {
    char identifier[3];
    char version[2];
    char flag;
    char size[4];
} id3v2Header;

typedef struct id3v2Frame {
    char frameId[4];
    char size[4];
    char flag[2];
    char unicode;
} id3v2Frame;
#pragma pack(pop)

id3v2Header header;
id3v2Frame frame;

//сдвигаем на 7i так как 7-ой байт не используется
int toInt(char byte[4], int is_header){ //байты в инт
    if (header.version[0] == 4 || is_header)
        return byte[0] << 21 | byte[1] << 14 | byte[2] << 7 | byte[3];
    return byte[0] << 24 | byte[1] << 16 | byte[2] << 8 | byte[3];
}

void toBytes(int our_number, char* symb, int is_it_header) {
    int bit = 127;
    if (header.version[0] == 4 || is_it_header) {
        symb[3] = (char)(our_number & bit);
        symb[2] = (char)((our_number >> 7) & bit);
        symb[1] = (char)((our_number >> 14) & bit);
        symb[0] = (char)((our_number >> 21) & bit);
    }
    else {
        symb[3] = (char)(our_number & bit);
        symb[2] = (char)((our_number >> 8) & bit);
        symb[1] = (char)((our_number >> 16) & bit);
        symb[0] = (char)((our_number >> 24) & bit);
    }
}

void show(char *to_file) {
    FILE *fin = fopen(to_file, "rb");
    if (fin == NULL) {
        printf("Error: Нельзя открыть файл\n");
        printf("Корректное написание: progName.exe --filepath=<path> --command\n==============\n");
        printf("Команды: \n--show\n--set=<propname> --value=<propvalue>\n--get=<propname>\n==============");
        exit(2);
    }
    int headerSizeInInt;
    int frameFillingSize;

    fread(&header, sizeof(char), 10, fin);
    headerSizeInInt = toInt(header.size, 1);
    printf("struct size: %d\n", headerSizeInInt);
    fread(&frame, 1, bytes_in_frame, fin);

    while (frame.frameId[0] != 0 and ftell(fin) < headerSizeInInt) {
        frameFillingSize = toInt(frame.size, 0);
        char *frameText = calloc(frameFillingSize, 1);
        fgets(frameText, frameFillingSize, fin);

        printf("id: %5s || size: %5d", frame.frameId, frameFillingSize);
        printf("value: %s\n", frameText);

        fread(&frame, 1, bytes_in_frame, fin);
        free(frameText);
    }
}

int get(char *to_file, char* id){
    FILE *fin = fopen(to_file, "rb");
    if (fin == NULL) {
        printf("Error: Нельзя открыть файл\n");
        printf("Корректное написание: progName.exe --filepath=<path> --command\n==============\n");
        printf("Команды: \n--show\n--set=<propname> --value=<propvalue>\n--get=<propname>\n==============");
        exit(2);
    }


    int headerSizeInt;
    fread(&header, 1, 10, fin);
    headerSizeInt = toInt(header.size, 1);
    fread(&frame, 1, bytes_in_frame, fin);
    int posis = -1;

    while (frame.frameId[0] != 0 and ftell(fin) < headerSizeInt) {
        int frameFillingSize = toInt(frame.size, 0);
        char *frameFilling = calloc(frameFillingSize, 1);
        fgets(frameFilling, frameFillingSize, fin);

        if (!strcmp(frame.frameId, id)){
            printf("id: %5s || size: %5d || value: ", frame.frameId, frameFillingSize);

            printf("%s\n", frameFilling);
            posis = ftell(fin) - frameFillingSize - 10;
            return posis;     // возращает позицию начала нужного фрейма
        }

        fread(&frame, 1, bytes_in_frame, fin);
        free(frameFilling);
    }

    return posis;
}


void set(char *to_file, char* propName, char* propValue) {
    FILE* fin = fopen(to_file, "rb");
    FILE* fout = fopen("temp", "wb");

    if (fin == NULL or fout == NULL){
        printf("Error: Can't open a file\n");
        printf("Корректное написание: progName.exe --filepath=<path> --command\n==============\n");
        printf("Команды: \n--show\n--set=<propname> --value=<propvalue>\n--get=<propname>\n==============");
        exit(2);
    }
    int nowHeaderSize;
    int endPos;
    int frameSize;
    int nowPos;
    int diff;


    int pos = get(to_file, propName);
    fread(&header, 1, 10, fin);        //считываем хеддер
    char* buf = calloc(pos - 10, 1);            //считываем всё от хеддера до нужного фрейма.
    fread(buf, 1, pos - 10, fin);

    id3v2Frame nuwrFrame;
    fread(&nuwrFrame, 1, bytes_in_frame, fin);       // считываем нужный фрейм
    frameSize = toInt(nuwrFrame.size, 0);

    fseek(fin, 0, SEEK_END);    // запоминаем конечную позицию для считывания
    endPos = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    fread(&header, 1, 10, fin);

    nowHeaderSize = toInt(header.size, 1) + ((int)strlen(propValue) - frameSize)+1;  // нашли новый размер хэддера в инт
    toBytes(nowHeaderSize, header.size, 1); // перевод в байты

    fwrite(&header, 1, 10, fout);      // запись хеддера

    fread(buf, 1, pos - 10, fin); // перемещаем указатель до нужного фрейма
    fwrite(buf, 1, pos - 10, fout); // записываем всё до нужного фрейма

    fread(&nuwrFrame, 1, bytes_in_frame, fin);   // переместили указатель до начала значения фрейма
    char *frameFilling = calloc(frameSize, 1);
    fgets(frameFilling, frameSize, fin); //переместили указатель до конца значения фрейма

    toBytes((int)strlen(propValue)+1, nuwrFrame.size, 0); // меняем размер фрейма
    fwrite(&nuwrFrame, 1, bytes_in_frame, fout); // записали нужный фрейм
    fwrite(propValue, 1, (int)strlen(propValue), fout); // записали значение нужно фрейма

    //добавляем музыку(копируем)
    nowPos = ftell(fin);
    diff = endPos - nowPos;


    uint8_t *lastBytes = malloc(diff);
    fread(lastBytes, 1, diff, fin);
    fwrite(lastBytes, 1, diff, fout);
    printf("Изменили:");
    get("temp",propName);
    //printf("id: %5s|| value: %s\n", frame.frameId, frameText);

}


void update(char *to_file, char *propName, char *propValue) {
    FILE *fin = fopen(to_file, "rb");
    FILE *fout = fopen("temp", "wb");

    if (fin == NULL or fout == NULL) {
        printf("Error: Нельзя открыть файл\n");
        printf("Корректное написание: progName.exe --filepath=<path> --command\n==============\n");
        printf("Команды: \n--show\n--set=<propname> --value=<propvalue>\n--get=<propname>\n==============");
        exit(2);
    }
    int endPos;
    int headerInSizeInt;
    int newHeaderSize;
    int curPos;
    int diff;

    fseek(fin, 0, SEEK_END);    // запоминаем конечную позицию для считывания
    endPos = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    id3v2Frame newFrame;
    strcpy(newFrame.frameId, propName);
    toBytes((int) strlen(propValue) + 1, newFrame.size, 0);
    newFrame.flag[0] = 0;
    newFrame.flag[1] = 1;
    newFrame.unicode = 0;

    fread(&header, 1, 10, fin);
    headerInSizeInt = toInt(header.size, 1);
    fread(&frame, 1, bytes_in_frame, fin);

    newHeaderSize = headerInSizeInt + 12 + (int) strlen(propValue);//мы просто изменяем на +1 frame
    toBytes(newHeaderSize, header.size, 1);                    //  изменяем размер хеддера
    fwrite(&header, 1, 10, fout);                          // записываем новый хеддер

    //копируем все фреймы в новый файл
    while (frame.frameId[0] != 0 and ftell(fin) < headerInSizeInt) {  // находим позицию последнего фрейма

        int frameTextSize = toInt(frame.size, 0);//размер фрейма
        char *frameText = calloc(frameTextSize - 1, 1);
        fgets(frameText, frameTextSize, fin);

        fwrite(&frame, 1, bytes_in_frame, fout);
        fwrite(frameText, 1, frameTextSize - 1, fout);

        fread(&frame, 1, bytes_in_frame, fin);
        free(frameText);
    }
    //создаём последний(новый) фрейм
    fwrite(&newFrame, 1, bytes_in_frame, fout);
    fwrite(propValue, 1, strlen(propValue), fout);//ставим значение(которое передаём в функцию)

    //добавляем музыку(копируем)
    curPos = ftell(fin) - 10;
    diff = endPos - curPos;

    uint8_t *lastBytes = malloc(diff);
    fread(lastBytes, 1, diff, fin);
    fwrite(lastBytes, 1, diff, fout);

    printf("Добавили 1 frame");
    show(fout);

    //перезаписываем файл(т.е  подменяем наш исходный файл на новый)
    remove(to_file);
    rename("temp", to_file);
}

int main(int argc, char **argv) {

    //setlocale(LC_ALL, "Russian");

    if (!(argc < 3 or argc > 4)) {
        printf("Error: Неверное число аргументов\n");
        printf("Корректное написание: progName.exe --filepath=<path> --command\n==============\n");
        printf("Команды: \n--show\n--set=<propname> --value=<propvalue>\n--get=<propname>\n==============");
        return 1;
    }

    strtok(argv[1], "=");//считываем до "=" название файла
    char *to_file = strtok(NULL, "=");


    if (strstr(argv[2], "--show") != NULL) {
        show(to_file);
        //далее после того как считали файл и команда не show, то нужно считать 2й аргумент, опять читаем до "="
    } else if (strstr(argv[2], "--get") != NULL) {
        strtok(argv[2], "=");
        char *propName = strtok(NULL, "=");
        get(to_file, propName);

    } else if (strstr(argv[2], "--set") != NULL) {
        strtok(argv[2], "=");
        char *prop_Name = strtok(NULL, "=");
        strtok(argv[3], "=");
        char *propValue = strtok(NULL, "=");
        if (get(to_file, prop_Name) == -1) {//если такого id нет, то добавляем
            update(to_file, prop_Name, propValue);
        }
        else {//если такой id есть, т просто заменяем
            set(to_file, prop_Name, propValue);
        }

    } else {
        printf("Error: Незнакомая команда\n");
        printf("Корректное написание: progName.exe --filepath=<path> --command\n==============\n");
        printf("Команды: \n--show\n--set=<propname> --value=<propvalue>\n--get=<propname>\n==============");
    }
}
