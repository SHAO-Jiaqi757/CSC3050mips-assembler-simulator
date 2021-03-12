#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <unordered_map>
#include <bitset>
#include "../include/assembler.h"

/*
To simulate main memory, you need to
dynamically allocate a block of memory with C/C++, with a size of 6MB.
*/

using namespace std;

size_t max_size = 6 * 1024 * 1024; // 6MB
size_t text_segment = 1 * 1024 * 1024;
u_int32_t start_addr = 0x400000;
u_int32_t static_data_begin = start_addr + text_segment;
u_int32_t dynamic_data_begin = static_data_begin;
u_int32_t stack_begin = 0xA00000; // goes downwards, address will decrease;

char *real_mem = (char *)malloc(max_size); //  "real_mem" storing the real address of the block of memory allocated.

/*
* map virtual_mem to the real allocated memory
* return a pointer that stores the address of real memory
*/

vector<int32_t> reg_values(32, 0);

u_int32_t pc = 0x400000;

unordered_map<std::string, int32_t> labels;
vector<vector<string>> instructions;
auto mapMem(u_int32_t virtual_mem);
void readData(string filename); //
void readText(string filename);
void putText();
void putData(string mode, string line);

/*
* initialize the registers
*/
void init()
{
    reg_values[29] = 0x1000000;
}

auto mapMem(u_int32_t virtual_mem)
{
    return (real_mem) + virtual_mem - start_addr;
}


void putData(string mode, string line)
{
    if (mode == "word")
    {
        size_t found = line.find_last_of(".word");
        line = line.substr(found + 1);
        vector<string> v = split(line, "\t\r, ");
        for (auto i : v)
        {
            *mapMem(dynamic_data_begin) = stoi(i); // 32-bit quantities
            dynamic_data_begin += 4;
        }
    }

    else if (mode == "ascii")
    {
        size_t found_l = line.find('\"');
        size_t found_r = line.rfind('\"');
        string str = line.substr(found_l + 1, found_r - found_l - 1);

        int n = 1; // one word only available for 4 chars
        int len = str.length();
        size_t i = 0;
        for (i; i < len; i++)
        {
            *mapMem(dynamic_data_begin) = str[i];
            dynamic_data_begin += 1;
        }

        // the rest characters that are going to next memory block
        if (dynamic_data_begin % 4 != 0)
            dynamic_data_begin = (dynamic_data_begin / 4) + 1;
    }
    else if (mode == "half")
    {

        size_t found = line.find_last_of(".half");
        line = line.substr(found + 1);
        vector<string> v = split(line, "\t\r, ");
        for (auto i : v)
        {
            *mapMem(dynamic_data_begin) = stoi(i); // 16-bit quantities
            dynamic_data_begin += 2;
        }

        if (dynamic_data_begin % 4 != 0)
            dynamic_data_begin = (dynamic_data_begin / 4) + 1;
    }
    else if (mode == "byte")
    {
        size_t found = line.find_last_of(".byte");
        line = line.substr(found + 1);
        vector<string> v = split(line, "\t\r, ");
        for (auto i : v)
        {
            *mapMem(dynamic_data_begin) = stoi(i); // 
            dynamic_data_begin += 1;
        }

        if (dynamic_data_begin % 4 != 0)
            dynamic_data_begin = (dynamic_data_begin / 4) + 1;
    }
    else
    {
        cout << "unsupported data type" << endl;
    }
}
void readData(std::string filename)
{

    ifstream infile(filename);
    std::string line;

    while (std::getline(infile, line))
    {
        // find ".data" part
        // ".data" segment is optional;
        if (line.find(".data") != std::string::npos)
            break;
    }

    while (std::getline(infile, line))
    {
        if (line.find(".text") != string::npos) break; // text segment
        if (line.find('#') != string::npos)
        {
            // find comments
            line = line.substr(0, line.find('#'));
        }

        if (line.length() == 0) // blank line
            continue;

        if (line.find(".word") != std::string::npos)
        {

            putData("word", line);
        }

        else if (line.find("ascii") != std::string::npos)
        {
            putData("ascii", line);
        }

        else if (line.find("asciiz") != std::string::npos)
        {
            putData("ascii", line + '\0');
        }

        else if (line.find("half") != std::string::npos)
        {
            *mapMem(dynamic_data_begin) = stoi(split(line, "\t\r ").back()); // 16-bit quantities
            dynamic_data_begin += 4;
        }

        else if (line.find("byte") != std::string::npos)
        {
        }
    }

    infile.close();
}

void putText(vector<int> instruction_machine_code) {
    u_int32_t text_addr = start_addr;
    for (int i: instruction_machine_code) {
        *mapMem(text_addr) = i;
        text_addr+=4;
    }
}


/*
Start simulating
Your code should maintain a PC, which points to the first line of code in the simulated memory.
Your code should have a major loop, simulating the machine cycle. Following the machine cycle,
your code should be able to:
1. Go to your simulated memory to fetch a line of machine code stored at the address PC
indicates.
2. PC=PC+4
3. From the machine code, be able to know what the instruction is and do the corresponding
things. write a C/C++ function for each instruction to do what it's supposed to
*/

int main()
{
    string filename = "";
    init(); // set register
    readData(filename); // get addresss where the dynamic data begins;
    Simulator simulator;
    simulator.readFile(filename);
    putText(simulator.instruction_machine_code);

    u_int32_t static_data_addr = static_data_begin;

    string line = ".half 23, 255, 120";
    putData("half", line);

    // int32_t v = 0x500000;
    // auto res = mapMem(v);
    // cout << hex <<  res;

    return 0;
}