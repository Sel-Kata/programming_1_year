#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


/////////////////////////////////////создаём очередь/////////////////////////////////////////////////////////////////
typedef struct node{
    char* value;
    struct node* next;
}node;

void push(node** head, char* data){
    node* tmp = malloc(sizeof(node));
    tmp->value = malloc((strlen(data) + 1) * sizeof(char));
    strcpy(tmp->value, data);
    tmp->next = *head;
    (*head) = tmp;
}

node* get_second_last(node*head){
    if (head == NULL)
        exit(-2);

    if (head->next == NULL)
        return NULL;

    while (head->next->next)
        head = head->next;
    return head;
}

char* pop(node** head) {
    char* final;
    node *second_last = NULL;
    if (!head || !(*head))
        exit(-1);
    second_last = get_second_last(*head);

    if (second_last == NULL){
        final = (*head)->value;
        free(*head);
        *head = NULL;
    }else{
        final = second_last->next->value;
        free(second_last->next);
        second_last->next = NULL;
    }
    return final;
}
///////////////////////////////////////конец реализации очереди/////////////////////////////////////////////////////////////

const char USAGE_MESSAGE[] = "\t\t==============Архиватор==============\n"
                             "Возможные команды:\n"
                             "--file FILE            \t||\tПередаём имя архива с которым будет работать архиватор\n"
                             "--create               \t||\tКоманда создаёт архив\n"
                             "--extract              \t||\tКоманда извлекает файлы из архива\n"
                             "--list                 \t||\tКоманда показывает, какие файлы содержатся в архиве\n"
                             "FILE1 FILE2 .... FILEN \t||\tПеречисление файлов которые мы кладём в массив\n\n"
                             "Пример:\n"
                             "arc.exe --file data.arc --create a.txt b.bin c.bmp\n"
                             "arc.exe --file data.arc --list\n"
                             "arc.exe --file data.arc --extract\n";


void openCorrect(FILE *file) {
    if (file == NULL) {
        printf("произошла ошибка: нельзя открыть файл\n");
        printf(USAGE_MESSAGE);
        exit(-1);
    }
}


//Структура нашего архива:
// размер всего архива (2б)
// ||имя файла1||размер1||имя файла2||размер2||…||содержание всех файлов

int counting_ranks(uint64_t x){
    int rez = 1;
    while (x > 10){
        rez++;
        x /= 10;
    }
    return rez;
}
//найдём размер всего файла(прибавляя: назавние кажд.файла, дину размера(кажд.фала) и содержимое кажд.фала))
//записываем информацию по каждому файлу:
//считаем размер файла, записываем в виде строик и ставим ||
//читаем название файла(если указан путь, то берём последнее слово) и записываем ставя ||
// переписываем весь header что узнали(всю информацию) а архив
// переписываем содержимое каждого файла друг за другом

void create(char **files, int file_count, char *name_of_archive) {
    FILE *archive = fopen(name_of_archive, "wb");
    FILE *current_file;
    openCorrect(archive);

    FILE *curent_file;

    uint32_t header_size = 2;
    //считаем размер всего файла
    //Размер архива - 2 байта
    //Далее размер имени файла + размер числа (количество разрядов в нём) + 4 палочки разделителя
    for (int i = 0; i < file_count; i++){
        uint64_t file_size;
        FILE *temp = fopen(files[i], "rb");
        fseek(temp, 0, SEEK_END);
        file_size = ftell(temp);
        fclose(temp);
        header_size += (int) strlen(files[i]) + counting_ranks(file_size) + 4;//длина названия файла + размер числа(размера файла) + разделители(||||)
    }
    fwrite(&header_size, 4, 1, archive);//записываем размер всего файла(названия каждого файла+длина размера кажд.+содержание)
    fwrite("||", 1, 2, archive);

    for (int i = 0; i < file_count; i++) {
        curent_file = fopen(files[i], "rb");
        openCorrect(curent_file);

        fseek(curent_file, 0, SEEK_END);
        uint64_t file_size = ftell(curent_file);//записываем размер текущего файла
        int ranks = counting_ranks(file_size);//сколько нужно места чтоб сохранить размер
        char *file_size_as_string = malloc(ranks * sizeof(char));
        sprintf(file_size_as_string, "%lld", file_size);

        fwrite(file_size_as_string, 1, ranks, archive);//записываем размер этого файла это в header в виде строки
        fwrite("||", 1, 2, archive);// размер файла || ...

        fclose(curent_file);

        //теперь записываем имя файла. если указан весь путь, то пишем только только конец строки
        char *current_file_name =malloc(strlen(files[i]) * sizeof(char));
        if (strrchr(files[i], '/') == NULL) {//strrchr-последнее вхождение символа
            strcpy(current_file_name, files[i]);
        } else {
            strcpy(current_file_name, strrchr(files[i], '/') + 1);
        }

        fputs(current_file_name, archive);
        fwrite("||", 1, 2, archive);// размер файла || имя файла
    }

    char byte;

//переписываем содержимое файлов
    for (int i = 0; i < file_count; i++) {
        current_file = fopen(files[i], "rb");
        openCorrect(current_file);
        while (!feof(current_file)) {
            if (fread(&byte, 1, 1, current_file) == 1) {
                fwrite(&byte, 1, 1, archive);
            }
        }
        fclose(current_file);
        remove(files[i]);
    }
    printf("Заархивировали успешно!\n");

    fclose(archive);
}


void extract(char* name_of_archive){
    //считываем размер всего архива
    //откусываем по кусочкам код до ||(т.е у нас идёт по 2 кусочка: размер файла || имя файла ||)
    //кидаем кусочек в очередь
    //число кусочков / 2 = число файлов
    //достаём из очереди размер и имя и создаём файл с таким размером и именем
    //считываем всю информацию этого файла(=размер который мы считали)


    FILE *archive = fopen(name_of_archive, "rb");
    openCorrect(archive);
    //читаем блок с информацией про файлы
    int header_size;
    fread(&header_size, 4, 1, archive);//размер всего архива (2б + 2палочки = 4б)

    char *header_meaning = malloc((header_size + 1) * sizeof(char));
    fread(header_meaning, 1, header_size, archive);

    header_meaning[header_size] = '\0';

    int archive_component_counter = 0;//счётчик кусочков информации которую мы откусываем из header
    node *archive_component;
    char *component = strtok(header_meaning, "||");//т.е откусываем до ||
    while(component){//если такой кусочек есть, то кидаем её в очередь
        if(strlen(component) == 0)
            break;
        push(&archive_component, component);
        component = strtok(NULL, "||");
        ++archive_component_counter;
    }
    free(header_meaning);
    int file_count = archive_component_counter / 2;//так как header состоит из названия файла+размер этого файла, то число файлов /2

    char byte;

    for (int i = 0; i < file_count; i++){
        char* file_we_are_reading_now_size = pop(&archive_component);//из очереди забираем размер файла
        char* file_we_are_reading_now_name = pop(&archive_component);//из очереди забираем имя файла
        FILE * file_we_are_reading_now = fopen(file_we_are_reading_now_name, "wb");
        openCorrect(file_we_are_reading_now);

        //atoll-строку в число
        uint64_t int_file_size = atoll(file_we_are_reading_now_size);//размер текста из файла равен размеру, который мы считали из очереди

        for (uint64_t j = 0; j < int_file_size; j++){//переписываем информацию одного файла
            if (fread(&byte, 1, 1, archive) == 1){
                fwrite(&byte, 1, 1, file_we_are_reading_now);
            }
        }
        printf("\n\tРазъархивировали файл: %s\n", file_we_are_reading_now_name);
        fclose(file_we_are_reading_now);
        free(file_we_are_reading_now_size);
        free(file_we_are_reading_now_name);
    }
    printf("\n\tРазъархивировали все файлы успешно!\n");

    fclose(archive);
    remove(name_of_archive);
}

void list(char* name_of_archive){
    //считываем размер всего архива
    //откусываем по кусочкам код до ||(т.е у нас идёт по 2 кусочка: размер файла || имя файла ||)
    //кидаем кусочек в очередь
    //число кусочков / 2 = число файлов  -> печатаем размер
    //достаём из очереди размер, достаём имя -> имя печатаем

    FILE *archive = fopen(name_of_archive, "rb");
    openCorrect(archive);
    int header_size;
    fread(&header_size, 4, 1, archive);//читаем header, там лежит размер архива

    char *header_meaning = malloc((header_size + 1) * sizeof(char));
    fread(header_meaning, 1, header_size, archive);

    header_meaning[header_size] = '\0';

//количество файлов в архиве: кол-во названий и кол-во размеров /2
    int archive_component_counter = 0;
    node *archive_components;
    char *component = strtok(header_meaning, "||");
    while(component){
        if(strlen(component) == 0)
            break;
        push(&archive_components, component);
        component = strtok(NULL, "||");
        ++archive_component_counter;
    }
    free(header_meaning);

    int file_count = archive_component_counter / 2;

    printf("\nВ архиве: %s: кол-во файлов:%d\n", name_of_archive, file_count);

    for (int i = 0; i < file_count; i++){
        //вынимаем из очереди размер файла(он нам не нужен, но размер-имя лежат парой, так что вынуть нужно)
        char* file_we_are_reading_now_size = pop(&archive_components);
        char* file_we_are_reading_now_name = pop(&archive_components);//вынимаем из очереди имя файла

        printf("%s\n", file_we_are_reading_now_name);

        free(file_we_are_reading_now_size);
        free(file_we_are_reading_now_name);
    }
}

int main(int argc, char **argv) {
    int file_counter;

    if (argc < 4){
        printf("Неверное количество аргументов\n");
        exit(-2);
    } else {
        char** files = malloc(argc * sizeof(char*));
        char* archive = malloc(255 * sizeof(char));//max длина названия 255
        file_counter = 0;

        for (int i = 1; i < argc; i++){
            if (!strcmp("--file", argv[i])){
                archive = argv[i+1];
            }
        }

        if (strlen(archive) == 0){
            printf("Архив не найден\n");
            exit(-3);
        }

        for (int i = 1; i < argc; i++){
            if (!strcmp("--create", argv[i])){
                for (int j = i + 1; j < argc; j++){
                    files[file_counter] = argv[j];
                    file_counter++;
                }
                create(files, file_counter, archive);
            }
            else if (!strcmp("--extract", argv[i])){
                extract(archive);
            }
            else if (!strcmp("--list", argv[i])){
                list(archive);
            }
        }
    }
    return 0;

}
