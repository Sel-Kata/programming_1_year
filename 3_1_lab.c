#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <iso646.h>

typedef struct node{//односвязный список
    char* value;
    struct node* next;
}node;


void push(node** head, char* data){
    node* tmp = malloc(sizeof(node));
    tmp->value = calloc(strlen(data) + 1, sizeof(char));
    strcpy(tmp->value, data);
    tmp->next = *head;
    (*head) = tmp;
}

char* pop(node** head){
    if (*head == NULL){
        exit(-1);
    }
    char* res = (**head).value;
    *head = (*head)->next;
    return res;
}

const char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun","Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

typedef struct data {
    char *addres;
    char *localTime;
    char *timeZone;
    char *request;
    char *status;
    char *bytes;
} data;

data getData(char *request) {//эта функция разделяет нашу строку на части
    data final_version;

    final_version.addres = strtok(request, "-");

    strtok(NULL, "[");
    final_version.localTime = strtok(NULL, " ");

    final_version.timeZone = strtok(NULL, "]");

    strtok(NULL, "\"");
    final_version.request = strtok(NULL, "\"");

    final_version.status = strtok(NULL, " ");
    final_version.bytes = strtok(NULL, "\n");

    return final_version;

}

//время Unix-измеряется количеством секунд, прошедших с «эпохи» (начало 1970 года по UTC)
int getTime(char *reqTime) {
    int final_version;
    struct tm localTime = {};

    localTime.tm_mday = atoi(strtok(reqTime, "/"));//считываем пока не дойдём до "/"
    int res;
    char mon;
    for (res = 0; res < 12 and strcmp(mon, month[res]); res++);
    int res3=res + 1;
    localTime.tm_mon = res3;//мы продолжаем поиск в оригинальной строке->ставим NULL
    localTime.tm_year = atoi(strtok(NULL, ":")) - 1900; // years since 1900
    localTime.tm_hour = atoi(strtok(NULL, ":"));
    localTime.tm_min = atoi(strtok(NULL, ":"));
    localTime.tm_sec = atoi(strtok(NULL, ":"));

    final_version = mktime(&localTime);//конвертирует время в календарное время(1 число на выход)

    return final_version;
}

void timeWindowSearch(long long **final_version, long long ReqCnt, int timeWindowInterval, int *tmReq) {//поиск временного окна размером в tmWindow
    //res массив [0] - левая граница, [1] - правая граница, [2] - количество запросов
    //ReqCnt- номер строки
    //tmWindow -временное окно
    //tmReq -массив со временем запроса
    long long mxLen = 0, where_we = 1;

    for (long long i = 0; i < ReqCnt; i++) {//перебор всех времён
        while (where_we < ReqCnt and tmReq[where_we] - tmReq[i] <= timeWindowInterval) {//число на правой - число на левой укладывается в наше временное окно мы увеличиваем правую границу
            where_we++;
        }
        if (tmReq[where_we] - tmReq[i] > timeWindowInterval) {//смотрим максимальная ли длина
            if (where_we - i > mxLen) {//ищем макс.временое окно
                mxLen = where_we - i;
                (*final_version)[0] = tmReq[i];//начало временного окна
                (*final_version)[1] = tmReq[where_we - 1];//конец временного окна
                (*final_version)[2] = mxLen;//его длина
            }
        } else {
            if (where_we - i > mxLen) {
                mxLen = where_we - i;
                (*final_version)[0] = tmReq[i];
                (*final_version)[1] = tmReq[where_we - 1];
                (*final_version)[2] = mxLen;
            }
            break;
        }
    }

}
void timeWindowSearchMin(long long **final_version, long long ReqCnt, int timeWindowInterval, int *tmReq) {//поиск временного окна размером в tmWindow
    //resM массив [0] - левая граница, [1] - правая граница, [2] - количество запросов
    //ReqCnt- номер строки
    //tmWindow -временное окно
    //tmReq -массив со временем запроса
    long long minLen = 1000000000000009, where_we = 1;

    for (long long i = 0; i < ReqCnt; i++) {//перебор всех времён
        while (where_we < ReqCnt and tmReq[where_we] - tmReq[i] <= timeWindowInterval) {//число на правой - число на левой укладывается в наше временное окно мы увеличиваем правую границу
            where_we++;
        }
        if (tmReq[where_we] - tmReq[i] > timeWindowInterval) {
            if (where_we - i < minLen) {//ищем мин.временое окно
                minLen = where_we - i;
                (*final_version)[0] = tmReq[i];//начало временного окна
                (*final_version)[1] = tmReq[where_we - 1];//конец временного окна
                (*final_version)[2] = minLen;//его длина
            }
        } else {
            if (where_we - i < minLen) {
                minLen = where_we - i;
                (*final_version)[0] = tmReq[i];
                (*final_version)[1] = tmReq[where_we - 1];
                (*final_version)[2] = minLen;
            }
            break;
        }
    }

}

int dataCorrect(data curData) {//проверка, что запрос вообще верен
    return !(curData.localTime == NULL or curData.request == NULL or curData.status == NULL or
             curData.timeZone == NULL or curData.addres == NULL or curData.bytes == NULL);
}

int main(int argc, char *argv[]) {

    FILE *our_file = fopen("/Users/selihovkinaekaterina/Desktop/forC/NASA_access_log_Jul95", "r");

    node *requests = NULL;//создали очередь
    int period = 2;//размер временного окна(2й аргумент)(когда количество запросов на сервер было максимально)
    long long linesLimit = 128; // Будем динамически выделять память, увеличивая её в два раза при переполнении
    long long stringCnt = 0;
    long long read_errors = 0;//сколько у нас ошибок 5**
    char *curReq = calloc(2048, sizeof(char));//массив
    int *requestTimes = calloc(linesLimit, sizeof(long long));//массив куда записываем время в секундах каждого запроса

    while (!feof(our_file)) {//пока не конец файла

        fgets(curReq, 2048, our_file);
        data curReqData = getData(curReq);//мы разделили наш запрос на части и теперь это тип data

        if (dataCorrect(curReqData)) {//если запрос вообще верен
            if (stringCnt == linesLimit) {
                linesLimit *= 2;
                requestTimes = realloc(requestTimes, linesLimit * sizeof(long long));
            }

            requestTimes[stringCnt] = getTime(curReqData.localTime);//заносим в массив время запроса

            if (curReqData.status[0] == '5') {//статус начинается с 5(серверные ошибки)(500-внутр.ошибка, 501-Метод запроса не поддерживается сервером и не может быть обработан...503-cервер недоступен)
                push(&requests, curReqData.request); // помещаем искомый запрос в очередь. pop() - возвращает первый пришедший, popBack - последний пришедший.
                read_errors++;
            }
            stringCnt++;
        }
    }

    long long *timeWindowInterval = calloc(3, sizeof(long long)); // [0] - левая граница, [1] - правая граница, [2] - количество запросов.=
    timeWindowSearch(&timeWindowInterval, stringCnt, period, requestTimes);

    long long *tmWindowIntervalForMin = calloc(3, sizeof(long long));
    timeWindowSearchMin(&tmWindowIntervalForMin, stringCnt, period, requestTimes);
    printf("Минимальное количество запросов(%lld) от %lld до %lld\n\n", tmWindowIntervalForMin[2], tmWindowIntervalForMin[0], tmWindowIntervalForMin[1]);

    printf("Длина временного окна: %d\n", period);
    printf("Максимальное количество запросов(%lld) от %lld до %lld\n\n", timeWindowInterval[2], timeWindowInterval[0],timeWindowInterval[1]);

    printf("В файле  %lld 5xx errors\n\nСписок запросов:\n", read_errors);

    for (long long i = 0; i < read_errors; i++)//печатаем все ошибки 5**
        printf("%s\n", pop(&requests));

    return 0;
}
