// 6.自定义ASCII字符串对比.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
int ascii_check(const char*, const char*);
int main()
{
    std::cout << ascii_check("abc123A", "1bc123A") << std::endl;
}

int ascii_check(const char* sor, const char* desc) {
    //如果是一样的字符串， 返回1
    int sor_size = 0;
    while (sor[sor_size]) {
        sor_size++;
    }
    std::cout << "原始字符串长度:" << sor_size << std::endl;

    int desc_size = 0;
    while (desc[desc_size]) {
        desc_size++;
    }
    std::cout << "对比的字符串长度:" << desc_size << std::endl;

    if (sor_size != desc_size) return 0;
    int check = 0;
    for (; check < sor_size; check++){
        if (sor[check] == desc[check]) continue;
        break;
    }
    if (sor_size != check) return 0;
    return 1;
}

