#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iso646.h>

const int base = 1000 * 1000 * 1000;

typedef struct {
    uint32_t *number;
    int32_t len;
} uint1024_t;


void scan_value(uint1024_t *number) {
    // 2^1024 - 309 знаков
    char str[310];
    scanf("%309s", str);
    int str_len = strlen(str);
    int item_counter;//кол-во элементов в массиве
    //выясняем сколько элементов нам нужно
    if (str_len % 9 == 0)
        item_counter = str_len / 9;
    else
        item_counter = (str_len / 9) + 1;
    //после выяснения сколько у нас элементовб выделяем память
    uint1024_t total_number;
    total_number.len = item_counter;
    total_number.number = malloc(item_counter * sizeof(uint32_t));//выделяем память для всех элементов массива (кол-во элементов * вес int)
    //заполняем массив int
    for (int i = str_len, j = -1; i > 0; i -= 9) {//бежим с конца строки
        j++;//указатель массива(бежим с шагом в 9)
        str[i] = '\0';
        // Устанавливаем в i-ый элемент завершающий ноль,
        // записываем в массив число = str[i-9:i], если i >= 9
        // иначе записываем всю оставшующся строку
        if (i >= 9)//если это не последняя девятка чисел
            total_number.number[j] = atoi(str + i - 9);//переводим строку в int и добавляем в массив
        else//дозаливаем последнюю девятку
            total_number.number[j] = atoi(str);
    }
    *number = total_number;
}

void printf_value(uint1024_t number) {
    // Проверка на то, что первый элемент - 0
    // последующий вывод первого элемента
    if (number.number[number.len - 1] == 0 and number.len == 1)//если мы ввели число 0, то выводим 0
        printf("0\n");
    else//выводим сначала последнюю девятку чисел
        printf("%d", number.number[number.len - 1]);
    //теперь выводим целые девятки
    for (int i = number.len - 2; i >= 0; --i)//начиная с предпоследнего индекса и бежим к началу массива
        printf("%09d", number.number[i]);//мы выводм по 9 т.к добавляем ведущие нули
    printf("\n");
}

uint1024_t conversion(unsigned int number) {//переводим int в uint1024_t
    uint1024_t total_number;
    // Если x < взятого основания, то записываем число в массив размерность которого 1,
    // иначе размерность - 2.
    if (number < base) {//если длина числа меньше 9, записываем его в массив с 1 элем, длинной 1 и его в индекс [1]
        total_number.number = malloc(1 * sizeof(uint32_t));
        total_number.number[0] = number;
        total_number.len = 1;
    } else {//если длина больше 9, то массив из 2ч элем и в [0]- последнии 9 чисел, в [1]- оставшиеся
        total_number.number = malloc(2 * sizeof(uint32_t));
        total_number.len = 2;
        total_number.number[0] = number % base;
        total_number.number[1] = number / base;
    }
    return total_number;
}

uint1024_t addition(uint1024_t first_number, uint1024_t second_number) {//сложение

    int remainder = 0;//для переполнения
    int result_size;
    if (first_number.len>second_number.len){
        result_size=first_number.len;
    }else{
        result_size=second_number.len;
    }
    
    //выделяем память для result(там нули) пока что размер как max число,но мы можем его увеличить
    uint1024_t total_number;
    total_number.len = result_size;
    total_number.number = calloc(result_size, sizeof(uint32_t));

    for (int i = 0; i < result_size or remainder; i++) {//бежим по каждому элементу из массива и складываем столбиком
        // увеличиваем размер массива на один
        if (i == result_size) {//если у мы пробежали максимальное число, но у нас очтался перенос
            total_number.number = realloc(total_number.number, (result_size + 1) * sizeof(int32_t));//массив, его размер-динамически меняем
            total_number.number[result_size] = 0;
            total_number.len++;
        }


        total_number.number[i] = remainder + (i < second_number.len ? second_number.number[i] : 0) + (i < first_number.len ? first_number.number[i] : 0);//перенос+y(или 0)+x(или 0)
        //делаем новый carry
        remainder = total_number.number[i] >= base;//если получившийся элемент больше 9 символов, то последнии 9 записываем в [i], а оставшиеся в carry
        if (remainder)
            total_number.number[i] -= base;
    }
    return total_number;
}
uint1024_t minus(uint1024_t first_number, uint1024_t second_number) {//вычитание

    int remainder = 0;
    int result_size;
    if (first_number.len>second_number.len){
        result_size=first_number.len;
    }else{
        result_size=second_number.len;
    }

    uint1024_t total_number;
    total_number.len = result_size;
    total_number.number = calloc(result_size, sizeof(uint32_t));

    // Вычисление выражения x - y в result.num
    for (int i = 0; i < result_size or remainder; ++i) {
        if (first_number.number[i] < (i < second_number.len ? second_number.number[i] : 0) + remainder) {
            total_number.number[i] = base + ((i < first_number.len ? first_number.number[i] : 0) - (i < second_number.len ? second_number.number[i] : 0) - remainder);
            remainder = 1;
        } else {
            total_number.number[i] = (i < first_number.len ? first_number.number[i] : 0) - (i < second_number.len ? second_number.number[i] : 0) - remainder;
            remainder = 0;
        }
    }

    while (total_number.number[total_number.len - 1] == 0 and total_number.len > 1)
        total_number.len--;
    total_number.number = realloc(total_number.number, total_number.len * sizeof(uint32_t));//просто уменьшаем память на число "не 0" отрезая нули

    return total_number;
}

uint1024_t multiplication(uint1024_t first_number, uint1024_t second_number) {//умножение

    int result_size = first_number.len + second_number.len;//выделяем память
    
    uint1024_t result; 
    result.len = result_size;
    result.number = calloc(result_size, sizeof(uint32_t));

    for (int i = 0; i < first_number.len; ++i)//фиксируем нижнюю цифру и перемножаем на все верхнии
        for (int j = 0, remainder = 0; j < second_number.len or remainder; ++j) {
            //i+j-индекс куда мы записываем ответ(может там уже было число)
            //бывшее число+(x * y(если он не закончился, иначе 0))
            long long cur = result.number[i + j] + first_number.number[i] * 1ll * (j < second_number.len ? second_number.number[j] : 0) + remainder;//наше получившееся число может быть очень большим
            result.number[i + j] = (uint32_t)(cur % base);//если получилось число больше 9 символов, записываем последнии 9 в ответ, остальное переполнение в carry
            remainder = (int)(cur / base);
        }
    //убираем ведущие нули
    while (result.number[result.len - 1] == 0 and result.len > 1)
        result.len--;
    result.number = realloc(result.number, result.len * sizeof(uint32_t));
    return result;
}

int main(int argc, char *argv[]) {
    uint1024_t x, y;
    int i;
    scan_value(&x);
    scan_value(&y);
    scanf("%d", &i);

    printf("Умножение:");
    printf_value(multiplication(x, y));//умножение
    printf("конвертация:");
    printf_value(conversion(i));//конвертация
    printf("сложение:");
    printf_value(addition(x, y));//
    printf("вычитание:");
    printf_value(minus(x, y));//вычитание*/

}
