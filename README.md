# **Лабараторная работа №2**
## **Козырев Пётр Вариант 14**


### Цель работы:

Целью является приобретение практических навыков в:
- Управление потоками в ОС 
- Обеспечение синхронизации между потоками


### Задание:

Составить программу на языке Си, обрабатывающую данные в многопоточном режиме. При 
обработки использовать стандартные средства создания потоков операционной системы 
(Windows/Unix). Ограничение максимального количества потоков, работающих в один момент 
времени, должно быть задано ключом запуска вашей программы.
Так же необходимо уметь продемонстрировать количество потоков, используемое вашей 
программой с помощью стандартных средств операционной системы.
В отчете привести исследование зависимости ускорения и эффективности алгоритма от входных 
данных и количества потоков. Получившиеся результаты необходимо объяснить.

### Вариант задания:

***Вариант 14*** Есть набор 128 битных чисел, записанных в шестнадцатеричном представлении, 
хранящихся в файле. Необходимо посчитать их среднее арифметическое. Округлить 
результат до целых. Количество используемой оперативной памяти должно задаваться 
"ключом".

### Решение:

main.cpp
```
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <pthread.h>
#include <mutex>
#include <chrono>
#include "bigint.h"

std::mutex mutex;

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
        if(i < array_str.size()){
            mutex.lock();
            sm += from_16_to_10(array_str[i]);
            mutex.unlock();
        }
    }
    pthread_exit(0);
}

int main(int argc,char* argv[]){

    if(argc != 3){
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

    const int threadCount = atoi(argv[1]);  // количество потоков
    int memory = atoi(argv[2]);  // в байтах
    int global_arr_size = 0;

    if(memory < 16){
        std::cerr << "Less memory\n";
        return 1;
    }

    while(std::getline(file,line)){

        //std::cout << "New cycle\n";

        array_str.push_back(line);
        int loc_memory = 16;
            
        if (file.is_open()){
            while (loc_memory < memory and std::getline(file, line)){
                array_str.push_back(line);
                loc_memory += 16;
                //std::cout << loc_memory << std::endl;
            }
        }   

        // for(auto elem : array_str){
        //     std::cout << elem << " ";
        // }

        //std::cout << std::endl;

        pthread_t t[threadCount];
        someArgs_t args[threadCount];

        int arr_size = array_str.size();
        global_arr_size += arr_size;
        int step = ((arr_size) /threadCount) + 1;

        if(arr_size == 0){
            std::cerr << "Empty array\n";
            break;
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

        array_str.clear();
    }

    file.close();

    if(global_arr_size == 0){
        std::cerr << "Empty file\n"; 
        return 1;
    }
    big_integer averg = sm/global_arr_size;
    std::cout<< averg <<std::endl;

    
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    std::cout << elapsed_ns.count() << " ns\n";
}
```

bigint.h
```
#include <vector>
#include <string>
#include <ostream>
#include <iomanip>
#include <sstream>

class big_integer {
    // основание системы счисления (1 000 000 000)
    static const int BASE = 1000000000;
 
    // внутреннее хранилище числа
    std::vector<int> _digits;
 
    // знак числа
    bool _is_negative;
 
    void _remove_leading_zeros();
    void _shift_right();
 
public:
    // класс-исключение, бросаемое при делении на ноль
    class divide_by_zero: public std::exception {  };
 
    big_integer();
    big_integer(std::string);
    big_integer(signed char);
    big_integer(unsigned char);
    big_integer(signed short);
    big_integer(unsigned short);
    big_integer(signed int);
    big_integer(unsigned int);
    big_integer(signed long);
    big_integer(unsigned long);
    big_integer(signed long long);
    big_integer(unsigned long long);
 
    friend std::ostream& operator <<(std::ostream&, const big_integer&);
    operator std::string() const;
    const big_integer operator +() const;
    const big_integer operator -() const;
    const big_integer operator ++();
    const big_integer operator ++(int);
    const big_integer operator --();
    const big_integer operator --(int);
    friend bool operator ==(const big_integer&, const big_integer&);
    friend bool operator <(const big_integer&, const big_integer&);
    friend bool operator !=(const big_integer&, const big_integer&);
    friend bool operator <=(const big_integer&, const big_integer&);
    friend bool operator >(const big_integer&, const big_integer&);
    friend bool operator >=(const big_integer&, const big_integer&);
    friend const big_integer operator +(big_integer, const big_integer&);
    big_integer& operator +=(const big_integer&);
    friend const big_integer operator -(big_integer, const big_integer&);
    big_integer& operator -=(const big_integer&);
    friend const big_integer operator *(const big_integer&, const big_integer&);
    big_integer& operator *=(const big_integer&);
    friend const big_integer operator /(const big_integer&, const big_integer&);
    big_integer& operator /=(const big_integer&);
    friend const big_integer operator %(const big_integer&, const big_integer&);
    big_integer& operator %=(const big_integer&);
 
    bool odd() const;
    bool even() const;
    const big_integer pow(big_integer) const;
};
```

bigint.cpp
```
#include "bigint.h"

// создает длинное целое число со значением 0
big_integer::big_integer() {
    this->_is_negative = false;
}
 
// создает длинное целое число из C++-строки
big_integer::big_integer(std::string str) {
    if (str.length() == 0) {
        this->_is_negative = false;
    }
    else {
        if (str[0] == '-') {
            str = str.substr(1);
            this->_is_negative = true;
        }
        else {
            this->_is_negative = false;
        }
 
        for (long long i = str.length(); i > 0; i -= 9) {
            if (i < 9)
                this->_digits.push_back(atoi(str.substr(0, i).c_str()));
            else
                this->_digits.push_back(atoi(str.substr(i - 9, 9).c_str()));
        }
 
        this->_remove_leading_zeros();
    }
}
 
// удаляет ведущие нули
void big_integer::_remove_leading_zeros() {
    while (this->_digits.size() > 1 && this->_digits.back() == 0) {
        this->_digits.pop_back();
    }
 
    if (this->_digits.size() == 1 && this->_digits[0] == 0) this->_is_negative = false;
}
 
// печатает число в поток вывода
inline std::ostream& operator <<(std::ostream& os, const big_integer& bi) {
    if (bi._digits.empty()) os << 0;
    else {
        if (bi._is_negative) os << '-';
        os << bi._digits.back();
        char old_fill = os.fill('0');
        for (long long i = static_cast<long long>(bi._digits.size()) - 2; i >= 0; --i) os << std::setw(9) << bi._digits[i];
        os.fill(old_fill);
    }
 
    return os;
}
 
// сравнивает два числа на равенство
bool operator ==(const big_integer& left, const big_integer& right) {
    if (left._is_negative != right._is_negative) return false;
    if (left._digits.empty()) {
        if (right._digits.empty() || (right._digits.size() == 1 && right._digits[0] == 0)) return true;
        else return false;
    }
    
    if (right._digits.empty()) {
        if (left._digits.size() == 1 && left._digits[0] == 0) return true;
        else return false;
    }
 
    if (left._digits.size() != right._digits.size()) return false;
    for (size_t i = 0; i < left._digits.size(); ++i) if (left._digits[i] != right._digits[i]) return false;
 
    return true;
}
 
// возвращает копию переданного числа
const big_integer big_integer::operator +() const {
    return big_integer(*this);
}
 
// возвращает переданное число с другим знаком
const big_integer big_integer::operator -() const {
    big_integer copy(*this);
    copy._is_negative = !copy._is_negative;
    return copy;
}
 
// проверяет, является ли левый операнд меньше правого
bool operator <(const big_integer& left, const big_integer& right) {
    if (left == right) return false;
    if (left._is_negative) {
        if (right._is_negative) return ((-right) < (-left));
        else return true;
    }
    else if (right._is_negative) return false;
    else {
        if (left._digits.size() != right._digits.size()) {
            return left._digits.size() < right._digits.size();
        }
        else {
            for (long long i = left._digits.size() - 1; i >= 0; --i) {
                if (left._digits[i] != right._digits[i]) return left._digits[i] < right._digits[i];
            }
            
            return false;
        }
    }
}
 
// сравнивает два числа на неравенство
bool operator !=(const big_integer& left, const big_integer& right) {
    return !(left == right);
}
 
// проверяет, является ли левый операнд меньше либо равен правого
bool operator <=(const big_integer& left, const big_integer& right) {
    return (left < right || left == right);
}
 
// проверяет, является ли левый операнд больше правого
bool operator >(const big_integer& left, const big_integer& right) {
    return !(left <= right);
}
 
// проверяет, является ли левый операнд больше либо равен правого
bool operator >=(const big_integer& left, const big_integer& right) {
    return !(left < right);
}
 
// складывает два числа
const big_integer operator +(big_integer left, const big_integer& right) {
    if (left._is_negative) {
        if (right._is_negative) return -(-left + (-right));
        else return right - (-left);
    }
    else if (right._is_negative) return left - (-right);
    int carry = 0;
    for (size_t i = 0; i < std::max(left._digits.size(), right._digits.size()) || carry != 0; ++i) {
        if (i == left._digits.size()) left._digits.push_back(0);
        left._digits[i] += carry + (i < right._digits.size() ? right._digits[i] : 0);
        carry = left._digits[i] >= big_integer::BASE;
        if (carry != 0) left._digits[i] -= big_integer::BASE;
    }
 
    return left;
}
 
// прибавляет к текущему числу новое
big_integer& big_integer::operator +=(const big_integer& value) {
    return *this = (*this + value);
}
 
// префиксный инкремент
const big_integer big_integer::operator++() {
    return (*this += 1);
}
 
// преобразует число к строке
big_integer::operator std::string() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}
 
// преобразует signed char к big_integer
big_integer::big_integer(signed char c) {
    if (c < 0) this->_is_negative = true;
    else this->_is_negative = false;
    this->_digits.push_back(std::abs(c));
}
 
// преобразует unsigned char к big_integer
big_integer::big_integer(unsigned char c) {
    this->_is_negative = false;
    this->_digits.push_back(c);
}
 
// преобразует signed short к big_integer
big_integer::big_integer(signed short s) {
    if (s < 0) this->_is_negative = true;
    else this->_is_negative = false;
    this->_digits.push_back(std::abs(s));
}
 
// преобразует unsigned short к big_integer
big_integer::big_integer(unsigned short s) {
    this->_is_negative = false;
    this->_digits.push_back(s);
}
 
// преобразует signed int к big_integer
big_integer::big_integer(signed int i) {
    if (i < 0) this->_is_negative = true;
    else this->_is_negative = false;
    this->_digits.push_back(std::abs(i) % big_integer::BASE);
    i /= big_integer::BASE;
    if (i != 0) this->_digits.push_back(std::abs(i));
}
 
// преобразует unsigned int к big_integer
big_integer::big_integer(unsigned int i) {
    this->_digits.push_back(i % big_integer::BASE);
    i /= big_integer::BASE;
    if (i != 0) this->_digits.push_back(i);
}
 
// преобразует signed long к big_integer
big_integer::big_integer(signed long l) {
    if (l < 0) this->_is_negative = true;
    else this->_is_negative = false;
    this->_digits.push_back(std::abs(l) % big_integer::BASE);
    l /= big_integer::BASE;
    if (l != 0) this->_digits.push_back(std::abs(l));
}
 
// преобразует unsigned long к big_integer
big_integer::big_integer(unsigned long l) {
    this->_digits.push_back(l % big_integer::BASE);
    l /= big_integer::BASE;
    if (l != 0) this->_digits.push_back(l);
}
 
// преобразует signed long long к big_integer
big_integer::big_integer(signed long long l) {
    if (l < 0) { this->_is_negative = true; l = -l; }
    else this->_is_negative = false;
    do {
        this->_digits.push_back(l % big_integer::BASE);
        l /= big_integer::BASE;
    } while (l != 0);
}
 
// преобразует unsigned long long к big_integer
big_integer::big_integer(unsigned long long l) {
    this->_is_negative = false;
    do {
        this->_digits.push_back(l % big_integer::BASE);
        l /= big_integer::BASE;
    } while (l != 0);
}
 
// постфиксный инкремент
const big_integer big_integer::operator ++(int) {
    *this += 1;
    return *this - 1;
}
 
// префиксный декремент
const big_integer big_integer::operator --() {
    return *this -= 1;
}
 
// постфиксный декремент
const big_integer big_integer::operator --(int) {
    *this -= 1;
    return *this + 1;
}
 
// вычитает два числа
const big_integer operator -(big_integer left, const big_integer& right) {
    if (right._is_negative) return left + (-right);
    else if (left._is_negative) return -(-left + right);
    else if (left < right) return -(right - left);
    int carry = 0;
    for (size_t i = 0; i < right._digits.size() || carry != 0; ++i) {
        left._digits[i] -= carry + (i < right._digits.size() ? right._digits[i] : 0);
        carry = left._digits[i] < 0;
        if (carry != 0) left._digits[i] += big_integer::BASE;
    }
 
    left._remove_leading_zeros();
    return left;
}
 
// вычитает из текущего числа новое
big_integer& big_integer::operator -=(const big_integer& value) {
    return *this = (*this - value);
}
 
// перемножает два числа
const big_integer operator *(const big_integer& left, const big_integer& right) {
    big_integer result;
    result._digits.resize(left._digits.size() + right._digits.size());
    for (size_t i = 0; i < left._digits.size(); ++i) {
        int carry = 0;
        for (size_t j = 0; j < right._digits.size() || carry != 0; ++j) {
            long long cur = result._digits[i + j] + 
                left._digits[i] * 1LL * (j < right._digits.size() ? right._digits[j] : 0) + carry;
            result._digits[i + j] = static_cast<int>(cur % big_integer::BASE);
            carry = static_cast<int>(cur / big_integer::BASE);
        }
    }
 
    result._is_negative = left._is_negative != right._is_negative;
    result._remove_leading_zeros();
    return result;
}
 
// домножает текущее число на указанное
big_integer& big_integer::operator *=(const big_integer& value) {
    return *this = (*this * value);
}
 
// сдвигает все разряды на 1 вправо (домножает на BASE)
void big_integer::_shift_right() {
    if (this->_digits.size() == 0) {
        this->_digits.push_back(0);
        return;
    }
    this->_digits.push_back(this->_digits[this->_digits.size() - 1]);
    for (size_t i = this->_digits.size() - 2; i > 0; --i) this->_digits[i] = this->_digits[i - 1];
    this->_digits[0] = 0;
}
 
// делит два числа
const big_integer operator /(const big_integer& left, const big_integer& right) {
    if (right == 0) throw big_integer::divide_by_zero();
    big_integer b = right;
    b._is_negative = false;
    big_integer result, current;
    result._digits.resize(left._digits.size());
    for (long long i = static_cast<long long>(left._digits.size()) - 1; i >= 0; --i) {
        current._shift_right();
        current._digits[0] = left._digits[i];
        current._remove_leading_zeros();
        int x = 0, l = 0, r = big_integer::BASE;
        while (l <= r) {
            int m = (l + r) / 2;
            big_integer t = b * m;
            if (t <= current) {
                x = m;
                l = m + 1;
            }
            else r = m - 1;
        }
 
        result._digits[i] = x;
        current = current - b * x;
    }
 
    result._is_negative = left._is_negative != right._is_negative;
    result._remove_leading_zeros();
    return result;
}
 
// делит текущее число на указанное
big_integer& big_integer::operator /=(const big_integer& value) {
    return *this = (*this / value);
}
 
// возвращает остаток от деления двух чисел
const big_integer operator %(const big_integer& left, const big_integer& right) {
    big_integer result = left - (left / right) * right;
    if (result._is_negative) result += right;
    return result;
}
 
// присваивает текущему числу остаток от деления на другое число
big_integer& big_integer::operator %=(const big_integer& value) {
    return *this = (*this % value);
}
 
// проверяет, является ли текущее число нечетным
bool big_integer::odd() const {
    if (this->_digits.size() == 0) return false;
    return this->_digits[0] & 1;
}
 
// проверяет, является ли текущее число четным
bool big_integer::even() const {
    return !this->odd();
}
 
// возводит текущее число в указанную степень
const big_integer big_integer::pow(big_integer n) const {
    big_integer a(*this), result(1);
    while (n != 0) {
        if (n.odd()) result *= a;
        a *= a;
        n /= 2;
    }
 
    return result;
}
```

CMakeLists.txt
```
cmake_minimum_required(VERSION 3.10)
project(os_lab_2)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS_DEBUG "... -lpthread")
SET(CMAKE_CXX_FLAGS_RELEASE "... -lpthread")

add_executable(app main.cpp bigint.h bigint.cpp)
```