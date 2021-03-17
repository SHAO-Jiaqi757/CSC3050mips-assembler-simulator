#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
using namespace std;


int main() {
    // ifstream out("/Users/jiaqishao/Downloads/mc");
    // ifstream res("/Users/jiaqishao/Desktop/project1/3050/mc.out");
    // string real;
    // string result;
    // int count = 1;
    // while (getline(out, real) && getline(res, result)) {
    //     if (real != result) {
    //         cout << count << endl;
    //     }
    //     count++;
    // }

    // string line;
    // ifstream fs("./mc.out");
    
    // while (getline(fs, line)) {
    //      size_t found_l = line.find('\"');
    //     size_t found_r = line.rfind('\"');
    //     string str = line.substr(found_l + 1, found_r - found_l - 1);
    //     cout << "ascii / asciiz :  " << str << endl;
    //     cout << str << endl;
    //     cout << line << endl;
    // }

    int a = 5242880;
    int b =0 ;
    int c = (unsigned) b + (unsigned) a;
    cout << hex << c << endl;
    // printf("%d \n", (unsigned)a);

    
   
}