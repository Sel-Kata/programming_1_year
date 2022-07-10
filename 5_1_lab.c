#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Заголовок BMP составляет 54 байта + 8 байтов палитры(BITMAPFILEHEADER(14б)+BITMAPINFO(40б))

typedef struct image_structure_tag{
    uint8_t *header;
    uint8_t *array_for_picture;//массив для картинки
    uint8_t *picture;//само изображение
    int file_size;
    int header_size;
}imgStr;

imgStr bmp_to_struct(FILE *file, int* _width, int* _height){
    imgStr final_array;//будем заносить информацию в структуру res
    uint8_t *header = malloc(62 * sizeof(uint8_t));
    final_array.header_size = 62;
    // Заголовок BMP составляет 54 байта + 8 байтов палитры
    fread(header, 1, 62, file);

    // little-endian порядок(слева направо байты лежат)
    uint32_t temp=((uint8_t)header[12]) << 16) + (((uint8_t)header[13]) << 24;
    
    uint32_t displacement = header[10] + (((uint8_t)header[11]) << 8) + (temp));//смещение байт
    uint32_t temp2=((uint8_t)header[20]) << 16) + (((uint8_t)header[21]) << 24;
    uint32_t displacement_last=((uint8_t)header[19]) << 8;

    int width = header[18] + (displacement_last + temp2);
    uint32_t temp3=((uint8_t)header[24]) << 16) + (((uint8_t)header[25]) << 24;
    uint32_t displacement_last=((uint8_t)header[23]) << 8;
    int h = header[22] + (displacement_last + temp3);
    if (displacement > 62){
        free(header);
        header = malloc(displacement * sizeof(uint8_t));
        fseek(file, 0, SEEK_SET);
        fread(header, 1, displacement, file);
        final_array.header_size = (int) displacement;
    }

    printf("offset = %d\n", offset);

    // наше колличество байт должно быть кратно 4
    int lineSize = (width / 32) * 4;//наша ширина в байтах
    if (width % 32 != 0) {//если вдруг не кратное число,то округляем в большую сторону что б был дополнительный байт
        lineSize += 4;
    }
    int fileSize = lineSize * h;//ширина*высоту=fileSize

    uint8_t *img = malloc(width * h);
    uint8_t *data = malloc(fileSize); // готовим место для картинки

    // ищем откуда читать
    fseek(file, (long) offset, SEEK_SET);
    // читаем картинку
    fread(data, 1, fileSize, file);
    
    // декодируем биты
    int cur_byte;
    for(int i = 0, reversed_i = h - 1; i < h ; i++, reversed_i--) {//идём с левого нижнего угла
        for(int j = 0; j < width; j++) {
            cur_byte = j / 8;
            int where=i * lineSize + cur_byte;

            uint8_t data_byte = data[where];//скинируемая линия

            uint8_t maska = 0x80 >> j % 8; // 2^7
            int pos = reversed_i * width + j;
            if (data_byte & maska)
                img[pos]=1;
            else
                img[pos]=0;
        }
    }

    *_width = width; *_height = h;
    final_array.file_size = fileSize;
    final_array.header = header;

    final_array.array_for_picture = img;
    final_array.picture = data;
    
    return final_array;
}


int main(int argc, char *argv[]) {

    if (argc < 5 ){
        printf("Неверное число аргументоув: %d\n", argc);
        return 1;
    }else if (argc > 9){
        printf("Неверное число аргументоув: %d\n", argc);
        return 1;
    }else if (argc % 2 == 0){
        printf("Неверное число аргументоув: %d\n", argc);
        return 1;
    }

    char *thisFileName;
    char *where_to_put;
    uint64_t max_iter = (uint64_t) INFINITY;
    uint32_t frequency = 1;

    for (int i = 1; i < argc; i += 2){
        printf("%s ", argv[i]);
        if (strcmp(argv[i], "--input") == 0){
            thisFileName = argv[i + 1];
            printf("%s\n", thisFileName);
        }
        else if (strcmp(argv[i], "--output") == 0){
            where_to_put = argv[i + 1];
            printf("%s\n", where_to_put);
        }
        else if (strcmp(argv[i], "--dump_freq") == 0){
            frequency = atoi(argv[i + 1]);
            printf("%u\n", frequency);
        }
        else if (strcmp(argv[i], "--max_iter") == 0){
            max_iter = atoi(argv[i + 1]);
            printf("%lld\n", max_iter);
        }
        else {
            printf("ошибка, не тот параметр: %s\n", argv[i]);
            return 1;
        }
    }
    //file_name="/Users/selihovkinaekaterina/Desktop/forC/Pulsar.bmp";
    //dir_name="/Users/selihovkinaekaterina/Desktop/forC/";
    //max_iter=20;
    //frequency=1;

    FILE *input_file;
    input_file = fopen(f_Name, "rb");

    if (input_file == NULL){
        printf("Нельзя открыть файл %s\n", thisFileName);
        return 1;
    }

    int width, height;
    //читаем файл и разбиваем его на структуру
    imgStr bmp_struct = bmp_to_struct(input_file, &width, &height);
    uint8_t *array_for_picture = bmp_struct.array_for_picture;
    uint8_t *bmp_header = bmp_struct.header;
    uint8_t *picture = bmp_struct.picture;

    int n = height;
    int m = width;
    uint8_t **array_for_step1;

    array_for_step1 = (uint8_t**) malloc((n + 2) * sizeof (uint8_t*));
    for (int i = 0; i < n + 2; i++){
        array_for_step1[i] = (uint8_t*) malloc((m + 2) * sizeof (uint8_t));
    }


//объявляем 2 двумерных массива- они в начале одинаковы(это наша картинка). берём 1 массив и проводим 1 итерацию игры(образуем 2е поколение)->
//  ->записываем во второй массив. Теперь берём второй массив и делаем итерацию с ним в игре -> pзаписываем в первый массив
//  В итоге получается некий шаг(т.е один массив хранит текущее значение картинки, а другой предыдущее
// В тот в котором хранится предыдущее мы записываем новый шаг(новое значение картинки) и он становится текущим
    uint8_t **array_for_step2;
    array_for_step2 = (uint8_t**) malloc((n + 2) * sizeof (uint8_t*));
    for (int i = 0; i < n + 2; i++){
        array_for_step2[i] = (uint8_t*) malloc((m + 2) * sizeof (uint8_t));
    }

    for(int i = 0 ; i < height ; i++){
        for(int j = 0 ; j < width ; j++){
            if (array_for_picture[i * width + j]){//если клетка не закрашена ->0

                array_for_step1[i + 1][j + 1] = '0';
                array_for_step2[i + 1][j + 1] = '0';
            }
            else{
                // printf("%c ", '1');
                array_for_step1[i + 1][j + 1] = '1';
                array_for_step2[i + 1][j + 1] = '1';
            }

        }
    }

    uint64_t iter = 0;
    uint32_t freq_counter = 0;
    int file_counter = 0;
    uint8_t **cur_matrix;//тот массив который мы будем записывать в файл после игры
    //начинаем выводить поколения игры
    while (iter < max_iter){//проверяем сколько поколений
        if (iter % 2 == 0){
            current_array=array_for_step2;
            final_array=array_for_step1;
            t=length;
            r=width
            
            for (int k = 1; i < length + 1; k++){
                for (int j = 1; j < width + 1; j++){

                if (current_array[i][j] == '0'){//если эта клетка не живая
                    int cnt = 0;
                    for (int temporary = -1; temporary <= 1; temporary++){
                        for (int p = -1; p <= 1; p++){
                            if (current_array[k + temporary][j + p] == '1'){
                                cnt++;
                            }
                        }
                    }
                    if (cnt == 3){//если у нас 3 живых соседа -> новая жизнь
                        final_array[k][j] = '1';
                    }
                    else{//иначе клетка умирает
                        final_array[k][j] = '0';
                    }
                }
                else if (current_array[k][j] == '1'){//если клетка уже была живая
                    int cnt = -1;
                    for (int temporary = -1; temporary <= 1; temporary++){
                        for (int p = -1; p <= 1; p++){
                            if (current_array[k + temporary][j + p] == '1'){
                                cnt++;
                            }
                        }
                    }
                    if (cnt != 2 && cnt != 3){//если 2-3 живых соседа, клетка продолжает жить
                        final_array[k][j] = '0';
                    }
                    else{//иначе умирает
                        final_array[k][j] = '1';
                    }
                }
            }
        }
            
            
            
            cur_matrix = array_for_step1;//новый массив который запишем в новый файл
        }
        else {
            
            current_array=array_for_step1;
            final_array=array_for_step2;
            n=length;
            m=width
            cur_matrix = array_for_step2;//новый массив который запишем в новый файл
            for (int k = 1; i < length + 1; i++){
        for (int j = 1; j < width + 1; j++){

            if (current_array[k][j] == '0'){//если эта клетка не живая
                int cnt = 0;
                for (int temporary = -1; temporary <= 1; temporary++){
                    for (int p = -1; p <= 1; p++){
                        if (current_array[k + temporary][j + p] == '1'){
                            cnt++;
                        }
                    }
                }
                if (cnt == 3){//если у нас 3 живых соседа -> новая жизнь
                    final_array[k][j] = '1';
                }
                else{//иначе клетка умирает
                    final_array[k][j] = '0';
                }
            }
            else if (current_array[k][j] == '1'){//если клетка уже была живая
                int cnt = -1;
                for (int temporary = -1; temporary <= 1; temporary++){
                    for (int p = -1; p <= 1; p++){
                        if (current_array[k + temporary][j + p] == '1'){
                            cnt++;
                        }
                    }
                }
                if (cnt != 2 && cnt != 3){//если 2-3 живых соседа, клетка продолжает жить
                    final_array[k][j] = '0';
                }
                else{//иначе умирает
                    final_array[k][j] = '1';
                }
            }
        }
    }
            
        }
        freq_counter++;//частота
        if (freq_counter == frequency){//если это нужное нам повторение которое мы должны вывести
            FILE *output_file;
            freq_counter = 0;//как только увидели что поколение сошлось мы сбрасываем счётчик
            file_counter++;

            //создам новый файл в который запишем новую картинку
            char file_path[200] = "";
            char cur_slice[120] = "";
            //strcpy(cur_slice, slice_name);
            int length = snprintf(NULL, 0, "%d", file_counter);
            char str[length + 1];
            snprintf(str, length + 1, "%d", file_counter);//поколение начинается с 1 -> +1
            
            strcat(cur_slice, str);
            strcat(file_path, where_to_put);
            strcat(file_path, "\\");
            strcat(file_path, cur_slice);
            strcat(file_path, ".bmp\0");

            output_file = fopen(file_path, "wb");
            if (output_file == NULL){
                printf("не открыть файл");
                return 1;
            }


            //записываем в виде картинки(data) полученый массив(cur_matrix)
            int cur_pos = 0;
            uint8_t cur_byte = 0;
            // uint8_t mask = 0x80;

            for (int k = n; i >= 1; i--) {
                for (int j = 1; j <= m; j++) {

                    uint8_t mask = 0x80 >> (j - 1) % 8;
                    if (cur_matrix[k][j] == '0') {
                        cur_byte = cur_byte | mask;
                    }

                    if ((j % 8) == 0 || j == m) { // кладём 8 бит в текущий байт
                        picture[cur_pos] = cur_byte;
                        cur_pos++;
                        cur_byte = 0;
                        mask = 0x80;
                    }
                }
                while ((cur_pos) % 4 != 0) {
                    cur_pos++;
                }
            }
            fwrite(bmp_header, sizeof(uint8_t), bmp_struct.header_size, output_file);//перезаписываем header в новый файл
            fwrite(picture, sizeof(uint8_t), bmp_struct.file_size, output_file);
            printf("Сделали одну итерацию игры\n");
        }
        iter++;
    }


    printf("Успешно завершено\n");
    fclose(input_file);
    free(picture);
    free(array_for_picture);
    free(array_for_step1);
    free(array_for_step2);
    
    return 0;
}
