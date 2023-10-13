#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <pthread.h>
#include <mutex>
#include <chrono>
#include "bigint.h"

// структура потока для передачи в функцию

typedef struct someArgs_tag {

    int id;      // номер потока
    
    int start;   // начало диапазона который суммирует поток
    
    int finish;  // конец диапазона

} someArgs_t;


// Перевод из шестнадцатеричной в десятичную

big_integer from_16_to_10(std::string &arr){
    big_integer res = 0;
    for(int i = 0; i < arr.size(); ++i){
        if(arr[i] >= '0' and arr[i] <= '9'){
            big_integer st = 16;
            st = st.pow(arr.size()-i-1);
            st *= ((int)(arr[i]-'0'));
            res += st;
        }
        else{
            int d;
            switch (arr[i])
            {
                case 'A': d = 10; break;
                case 'B': d = 11; break;
                case 'C': d = 12; break;
                case 'D': d = 13; break;
                case 'E': d = 14; break;
                case 'F': d = 15; break;
                default: throw std::string("Input error");
            }
            big_integer st = 16;
            st = st.pow(arr.size()-i-1);
            st *= d;
            res += st;
        }
    }
    return res;
}

big_integer sm = 0;  // итоговая сумма чисел из файла

std::vector<std::string> array_str;  // массив для хранения чисел в строчном представлении

// подсчет суммы

void* sum_array(void* args){
    someArgs_t *arg = (someArgs_t*) args;

    for(int i = arg->start; i < arg->finish; ++i){
        if(i < array_str.size())
            sm += from_16_to_10(array_str[i]);
    }
    pthread_exit(0);
}

int main(int argc,char* argv[]){

    if(argc != 2){
        std::cerr << "Key error\n";
        return 1;
    }

    // считывание чисел из файла

    std::string line;    
    std::string path = "../../test/";
    std::string name;
    std::cout << "Input test file name: ";
    std::cin >> name;
    std::cout << std::endl; 

    auto start_time = std::chrono::steady_clock::now();

    path += name;
    std::ifstream file(path);

    if (file.is_open()){
        while (std::getline(file, line)){
            array_str.push_back(line);
        }
    }

    file.close();


    const int threadCount = atoi(argv[1]);  // количество потоков

    pthread_t t[threadCount];
    someArgs_t args[threadCount];

    int arr_size = array_str.size();
    int step = ((arr_size) /threadCount) + 1;

    if(arr_size == 0){
        std::cerr << "Empty file\n";
        return 1;
    }

    for(int i{0};i<threadCount;++i){
        args[i].id = i;

        if(i == 0)
            args[i].start = 0;
        else
            args[i].start = i*step;

        args[i].finish = (i+1)*step ;
    }

    for(int i{0};i<threadCount;++i){
        pthread_create(&t[i], NULL, sum_array, (void*)&args[i]);
    }

    for(int i{0};i<threadCount;++i){
        pthread_join(t[i],NULL);
    }

    big_integer averg = sm/arr_size;
    std::cout<< averg <<std::endl;

    
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    std::cout << elapsed_ns.count() << " ns\n";
}