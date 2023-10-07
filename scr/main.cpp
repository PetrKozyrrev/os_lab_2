#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mutex;

double from_16_to_10(std::string &arr){
    double res = 0;
    for(int i=0;i<arr.size();++i){
        if(arr[i]>='0' and arr[i]<='9'){
            res += (arr[i]-'0')*pow(16,arr.size()-i-1);
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
            }
            res += d*pow(16,arr.size()-i-1);
        }
    }
    return res;
}

double sm = 0;
std::vector<double> array_numbers;

void sum_array(int th_id,int step){
    int index = th_id*step - 1;
    for(int i=0;i<step;++i){
        if(index+i<array_numbers.size())
            sm+=array_numbers[index+i];
    }
}

int main(int argc,char* argv[]){

    auto start_time = std::chrono::steady_clock::now();

    if(argc != 2){
        std::cerr << "Key error\n";
        return 1;
    }

    const int threadCount = atoi(argv[1]); // количество потоков

    std::thread t[threadCount]; 

    std::string line;
    std::vector<std::string> array_str;
    
    std::string path = "../test/";
    std::string name;
    std::cout << "Input test file name: ";
    std::cin >>name;
    std::cout << std::endl; 
    path+=name;
    std::ifstream file(path);

    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            array_str.push_back(line);
        }
    }
    file.close();

    int arr_size = array_str.size();
    int step = arr_size/threadCount + 1;

    if(arr_size == 0){
        std::cerr << "Empty file\n";
        return 1;
    }

    for(auto elem: array_str){
        array_numbers.push_back(from_16_to_10(elem));
    }

    for(int i{0};i<threadCount;++i){
        t[i] = std::thread(sum_array,i,step);
    }
    for(int i{0};i<threadCount;++i){
        t[i].join();
    }

    std::cout.setf(std::ios::fixed);
    std::cout.precision(0);
    std::cout<<sm/(arr_size) <<std::endl;

    

    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    std::cout << elapsed_ns.count() << " ns\n";
}