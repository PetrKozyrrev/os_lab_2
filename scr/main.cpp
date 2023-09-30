#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>

double from_16_to_10(std::string &arr){
    double res = 0;
    for(int i=0;i<arr.size();++i){
        if(arr[i]>='0' and arr[i]<='9'){
            res += (arr[i]-'0')*pow(16,i);
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
            res += d*pow(16,i);
        }
    }
    return res;
}

double sum_array(std::vector<double>& arr){
    double sm = 0;
    for(auto el: arr){
        sm += el;
    }
    return sm;
}

int main(int argc,char* argv[]){
    std::string line;
    std::vector<std::string> array_str;
    std::vector<double> array_double;
    
    std::ifstream file("../test/test2.txt");
    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            array_str.push_back(line);
        }
    }

    if(array_str.size() == 0){
        std::cerr << "Empty file\n";
        return 1;
    }

    for(auto elem: array_str){
        array_double.push_back(from_16_to_10(elem));
    }
    std::cout.setf(std::ios::fixed);
    std::cout.precision(0);
    std::cout<<sum_array(array_double)/(array_double.size()) <<std::endl;
    // for(auto elem:array){
    //     std::cout.setf(std::ios::fixed);
    //     std::cout.precision(0); 
    //     std::cout<<from_16_to_10(elem)<<std::endl;
    // }

    file.close();
}