#include <fstream>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <unordered_map>
#include <sstream>
#include <bitset>
#include <vector>
#include "../include/assembler.h"

using namespace std;



int Assembler::is_valid_label(std::string label)
{
    // a valid label begin with _ or a letter (a-z)
    return (label.front() == '_') || (isalpha(label.front()));
}

int32_t pc_ = 0x400000;
int instruction_count = 0;

// MIPS Fields
/*
 
 < R-type >
  31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 |    op(6bits)    |   rs(5bits)  |   rt(5bits)  |   rd(5bits)  | shamt(5bits) |   fucnt(6bits)  |
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 
 
 < I-type >
  31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 |    op(6bits)    |   rs(5bits)  |   rt(5bits)  |         constant or address (16bits)          |
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 

 < J-type >
  31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 |    op(6bits)    |                            address (30bits)                                 |
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 
*/

void Assembler::set_regMap()
{
    regMap = { {"zero", 0},
               {"at", 1},
               {"v0", 2},
               {"v1", 3},
               {"a0", 4},
               {"a1", 5},
               {"a2", 6},
               {"a3", 7},
               {"t0", 8},
               {"t1", 9},
               {"t2", 10},
               {"t3", 11},
               {"t4", 12},
               {"t5", 13},
               {"t6", 14},
               {"t7", 15},
               {"s0", 16},
               {"s1", 17},
               {"s2", 18},
               {"s3", 19},
               {"s4", 20},
               {"s5", 21},
               {"s6", 22},
               {"s7", 23},
               {"t8", 24},
               {"t9", 25},
               {"k0", 26},
               {"k1", 27},
               {"gp", 28},
               {"sp", 29},
               {"fp", 30},
               {"ra", 31}
    };
}
void Assembler::translate_R_type(int op, int rs, int rt, int rd, int shamt, int funct)
{
    int32_t ans = 0;
    ans = (op << 26);
    ans |= (rs << 21);
    ans |= (rt << 16);
    ans |= (rd << 11);
    ans |= (shamt << 6);
    ans |= funct;
    instruction_machine_code.push_back(ans);
    // std::string res = std::bitset<32>(ans).to_string();
    // cout << res << endl;
    // instructions[instr_index++] = ans;
}

void Assembler::translate_I_type(int op, int rs, int rt, int addr)
{
    int32_t ans = 0;
    ans = (op << 26);
    ans |= (rs << 21);
    ans |= (rt << 16);
    ans |= (addr & 65535);
    instruction_machine_code.push_back(ans);
}

void Assembler::translate_J_type(int op, int addr)
{
    int32_t ans = 0;
    ans = (op << 26);
    ans |= addr;

    instruction_machine_code.push_back(ans);
    // std::string res = std::bitset<32>(ans).to_string();
    // cout << res << endl;
    // instructions[instr_index++] = ans;
}

vector<string> split(const string &str, const string &delim)
{
    vector<string> res;
    if ("" == str)
        return res;

    char *strs = new char[str.length() + 1];
    strcpy(strs, str.c_str());

    char *d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char *p = strtok(strs, d);
    while (p)
    {
        string s = p;
        res.push_back(s);
        p = strtok(NULL, d);
    }

    return res;
}

// find all label addresses and extract instructions
void Assembler::readFile(std::string filename)
{

    int32_t pc_ = 0x400000;
    ifstream infile(filename);
    std::string line;

    while (std::getline(infile, line))
    {
        // find ".text" part
        if (line.find(".text") != std::string::npos)
            break;
    }

    while (std::getline(infile, line))
    {
        // comments

        if (line.find('#') != string::npos)
        {
            // find comments
            line = line.substr(0, line.find('#'));
        }

        if (line.length() == 0) // blank line
            continue;

        // If there is no space between label and instructions
        size_t found_label = line.find(":");
        if (found_label != string::npos) {
            line.insert(found_label+1, " ");
            // add space for following split;
        }

        vector<string> res = split(line, ",$\t\r() ");

        if (res.size() == 0)
            continue;
        if (res[0].back() == ':')
        {
            res[0].pop_back();  // drop ":"
            labels.insert({res[0], pc_});
            if (res.size() != 1)
            {
                // case2, label following with instruction

                res.erase(res.begin());
                instructions.push_back(res);
            }
            else
                continue;
        }

        else
        {
            instructions.push_back(res);
            
        }
        // cout << "PC >>> " << hex << pc_ << endl;
        pc_ += 4;
    }

    // for (auto pair: labels) {
    //     cout << "label: " << pair.first << " ||| address: " << hex << pair.second << endl;
    // }
    infile.close();
}

void Assembler::translateInstruc(vector<string> instruction)
/*
* paramters: vector<string> instruction
* instruction is the return value from split()
* check instruction[0] to obtain the instruction type
*/
{
    std::string machine_code = "";
    if (instruction[0] == "add")
    { // add rd rs rt
        // machine_code += ("000000" + regMap[instruction[2]] + regMap[instruction[3]]+regMap[instruction[1]] + "00000" + ""
        
        translate_R_type(0, regMap[instruction[2]], regMap[instruction[3]], regMap[instruction[1]], 0, 0x20);
    }
    else if (instruction[0] == "addu")
    {
        translate_R_type(0, regMap[instruction[2]], regMap[instruction[3]], regMap[instruction[1]], 0, 0x21);
    }
    else if (instruction[0] == "addi")

    {
        translate_I_type(8, regMap[instruction[2]], regMap[instruction[1]], stoi(instruction[3]));
    }
    else if (instruction[0] == "addiu")
    {
        translate_I_type(9, regMap[instruction[2]], regMap[instruction[1]], stoi(instruction[3]));
    }
    else if (instruction[0] == "and")
    {
        // add rd, rs, rt
        translate_R_type(0, regMap[instruction[2]], regMap[instruction[3]], regMap[instruction[1]], 0, 0x24);
    }
    else if (instruction[0] == "andi")
    {
        // andi rt, rs, imm
        translate_I_type(0xc, regMap[instruction[2]], regMap[instruction[1]], stoi(instruction[3]));
    }
    else if (instruction[0] == "clo")
    {
        // clo rd, rs
        translate_R_type(0x1c, regMap[instruction[2]], 0, regMap[instruction[1]], 0, 0x21);
    }
    else if (instruction[0] == "clz")
    {
        translate_R_type(0x1c, regMap[instruction[2]], 0, regMap[instruction[1]], 0, 0x20);
    }
    else if (instruction[0] == "div")
    {
        // div rs, rt
        translate_R_type(0, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 0x1a);
    }
    else if (instruction[0] == "divu")
    {
        translate_R_type(0, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 0x1b);
    }
    else if (instruction[0] == "mult")
    {
        translate_R_type(0, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 0x18);
    }
    else if (instruction[0] == "multu")
    {
        translate_R_type(0, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 0x19);
    }
    else if (instruction[0] == "mul")
    {
        // mul rd, rs, rt
        translate_R_type(0x1c, regMap[instruction[2]], regMap[instruction[3]], regMap[instruction[1]], 0, 2);
    }
    else if (instruction[0] == "madd")
    {
        // madd rs, rt
        translate_R_type(0x1c, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 2);
    }
    else if (instruction[0] == "msub")
    {
        translate_R_type(0x1c, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 4);
    }
    else if (instruction[0] == "maddu")
    {
        translate_R_type(0x1c, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 1);
    }
    else if (instruction[0] == "msubu")
    {
        translate_R_type(0x1c, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 5);
    }
    else if (instruction[0] == "nor")
    {
        translate_R_type(0, regMap[instruction[2]], regMap[instruction[3]], regMap[instruction[1]], 0, 0x27);
    }
    else if (instruction[0] == "or")
    {
        translate_R_type(0, regMap[instruction[2]], regMap[instruction[3]], regMap[instruction[1]], 0, 0x25);
    }
    else if (instruction[0] == "ori")
    {
        translate_I_type(0xd, regMap[instruction[2]], regMap[instruction[1]], stoi(instruction[3]));
    }
    else if (instruction[0] == "sll")
    {
        translate_R_type(0, 0, regMap[instruction[2]], regMap[instruction[1]], stoi(instruction[3]), 0);
    }
    else if (instruction[0] == "sllv")
    {
        translate_R_type(0, regMap[instruction[3]], regMap[instruction[2]], regMap[instruction[1]], 0, 4);
    }
    else if (instruction[0] == "sra")
    {
        translate_R_type(0, 0, regMap[instruction[2]], regMap[instruction[1]], stoi(instruction[3]), 3);
    }
    else if (instruction[0] == "srav")
    {
        translate_R_type(0, regMap[instruction[3]], regMap[instruction[2]], regMap[instruction[1]], 0, 7);
    }
    else if (instruction[0] == "srl")
    {
        translate_R_type(0, 0, regMap[instruction[2]], regMap[instruction[1]], stoi(instruction[3]), 2);
    }
    else if (instruction[0] == "srlv")
    {
        translate_R_type(0, regMap[instruction[3]], regMap[instruction[2]], regMap[instruction[1]], 0, 6);
    }
    else if (instruction[0] == "sub")
    {
        // sub rd, rs, rt
        translate_R_type(0, regMap[instruction[2]], regMap[instruction[3]], regMap[instruction[1]], 0, 0x22);
    }
    else if (instruction[0] == "subu")
    {
        // subu rd, rs, rt
        translate_R_type(0, regMap[instruction[2]], regMap[instruction[3]], regMap[instruction[1]], 0, 0x23);
    }
    else if (instruction[0] == "xor")
    {
        translate_R_type(0, regMap[instruction[2]], regMap[instruction[3]], regMap[instruction[1]], 0, 0x26);
    }
    else if (instruction[0] == "xori")
    {
        // xori rt, rs, imm
        translate_I_type(0xe, regMap[instruction[2]], regMap[instruction[1]], stoi(instruction[3]));
    }
    else if (instruction[0] == "lui")
    {
        // lui rt, imm
        translate_I_type(0xf, 0, regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "slt")
    {
        // slt rd, rs, rt
        translate_R_type(0, regMap[instruction[2]], regMap[instruction[3]], regMap[instruction[1]], 0, 0x2a);
    }
    else if (instruction[0] == "sltu")
    {
        // sltu rd, rs, rt
        translate_R_type(0, regMap[instruction[2]], regMap[instruction[3]], regMap[instruction[1]], 0, 0x2b);
    }
    else if (instruction[0] == "slti")
    {
        // slti rt, rs, imm
        translate_I_type(0xa, regMap[instruction[2]], regMap[instruction[1]], stoi(instruction[3]));
    }
    else if (instruction[0] == "sltiu")
    {
        translate_I_type(0xb, regMap[instruction[2]], regMap[instruction[1]], stoi(instruction[3]));
    }
    else if (instruction[0] == "beq")
    {
        // beq rs, rt, label
        if (is_valid_label(instruction[3]))
        {
            translate_I_type(4, regMap[instruction[1]], regMap[instruction[2]], (labels[instruction[3]] - pc_ - 4) >> 2);
        }
        else
            translate_I_type(4, regMap[instruction[1]], regMap[instruction[2]], stoi(instruction[3]));
    }
    else if (instruction[0] == "bgez")
    {
        // bgez rs, label
        if (is_valid_label(instruction[2]))
        {
            translate_I_type(1, regMap[instruction[1]], 1, (labels[instruction[2]] - pc_ - 4) >> 2);
        }
        else
            translate_I_type(1, regMap[instruction[1]], 1, stoi(instruction[2]));
    }
    else if (instruction[0] == "bgezal")
    {
        // bgezal rs, label

        if (is_valid_label(instruction[2]))
        {
            translate_I_type(1, regMap[instruction[1]], 0x11, (labels[instruction[2]] - pc_ - 4) >> 2);
        }
        else
            translate_I_type(1, regMap[instruction[1]], 0x11, stoi(instruction[2]));
    }
    else if (instruction[0] == "bgtz")
    {
        // bgtz rs, label
        if (is_valid_label(instruction[2]))
        {
            translate_I_type(7, regMap[instruction[1]], 0, (labels[instruction[2]] - pc_ - 4) >> 2);
        }
        else
            translate_I_type(7, regMap[instruction[1]], 0, stoi(instruction[2]));
    }
    else if (instruction[0] == "blez")
    {
        // real: labels[instruction[2]]
        // pc current: pc

        if (is_valid_label(instruction[2]))
        {
            translate_I_type(6, regMap[instruction[1]], 0, (labels[instruction[2]] - pc_ - 4) >> 2);
        }
        else
            translate_I_type(6, regMap[instruction[1]], 0, stoi(instruction[2]));
    }
    else if (instruction[0] == "bltzal")
    {
        if (is_valid_label(instruction[2]))
        {
            translate_I_type(1, regMap[instruction[1]], 0x10,
                             (labels[instruction[2]] - pc_ - 4) >> 2);
        }
        else
            translate_I_type(1, regMap[instruction[1]], 0x10, stoi(instruction[2]));
    }
    else if (instruction[0] == "bltz")
    {
        if (is_valid_label(instruction[2]))
        {
            translate_I_type(1, regMap[instruction[1]], 0, (labels[instruction[2]] - pc_ - 4) >> 2);
        }
        else
            translate_I_type(1, regMap[instruction[1]], 0, stoi(instruction[2]));
    }
    else if (instruction[0] == "bne")
    {
        if (is_valid_label(instruction[3]))
        {
            translate_I_type(5, regMap[instruction[1]], regMap[instruction[2]], (labels[instruction[3]] - pc_ - 4) >> 2);
        }
        else
            translate_I_type(5, regMap[instruction[1]], regMap[instruction[2]], stoi(instruction[3]));
    }
    else if (instruction[0] == "j")
    {
        if (is_valid_label(instruction[1]))
            translate_J_type(2, labels[instruction[1]] >> 2);
        else
            translate_J_type(2, stoi(instruction[1]));
    }
    else if (instruction[0] == "jal")
    {
        if (is_valid_label(instruction[1]))
            translate_J_type(3, labels[instruction[1]] >> 2);
        else
            translate_J_type(3, stoi(instruction[1]));
    }
    else if (instruction[0] == "jalr")
    {
        // jalr rs, rd

        translate_R_type(0, regMap[instruction[1]], 0, regMap[instruction[2]], 0, 9);
    }
    else if (instruction[0] == "jr")
    {
        // jr rs
        translate_R_type(0, regMap[instruction[1]], 0, 0, 0, 8);
    }
    else if (instruction[0] == "teq")
    {
        // teq rs, rt
        translate_R_type(0, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 0x34);
    }
    else if (instruction[0] == "teqi")
    {
        // teqi rs, imm
        translate_I_type(1, regMap[instruction[1]], 0xc, stoi(instruction[2]));
    }
    else if (instruction[0] == "tne")
    {
        translate_R_type(0, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 0x36);
    }
    else if (instruction[0] == "tnei")
    {
        translate_I_type(1, regMap[instruction[1]], 0xe, stoi(instruction[2]));
    }
    else if (instruction[0] == "tge")
    {
        translate_R_type(0, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 0x30);
    }
    else if (instruction[0] == "tgeu")
    {
        translate_R_type(0, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 0x31);
    }
    else if (instruction[0] == "tgei")
    {
        translate_I_type(1, regMap[instruction[1]], 8, stoi(instruction[2]));
    }
    else if (instruction[0] == "tgeiu")
    {
        translate_I_type(1, regMap[instruction[1]], 9, stoi(instruction[2]));
    }
    else if (instruction[0] == "tlt")
    {
        // tlt rs rt
        translate_R_type(0, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 0x32);
    }
    else if (instruction[0] == "tltu")
    {
        translate_R_type(0, regMap[instruction[1]], regMap[instruction[2]], 0, 0, 0x33);
    }
    else if (instruction[0] == "tlti")
    {
        // tlti rs , imm
        translate_I_type(1, regMap[instruction[1]], 0xa, stoi(instruction[2]));
    }
    else if (instruction[0] == "tltiu")
    {
        translate_I_type(1, regMap[instruction[1]], 0xb, stoi(instruction[2]));
    }
    else if (instruction[0] == "lb")
    {
        // lb rt, address
        // makeI_type(0x20, 0, regMap[instruction[1]], stoi(instruction[2]));
        translate_I_type(0x20, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "lbu")
    {
        translate_I_type(0x24, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "lh")
    {
        translate_I_type(0x21, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "lhu")
    {
        translate_I_type(0x25, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "lw")
    {
        // lw rt 24($a1)

        translate_I_type(0x23, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "lwl")
    {
        translate_I_type(0x22, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "lwr")
    {
        translate_I_type(0x26, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "ll")
    {
        translate_I_type(0x30, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "sb")
    {
        translate_I_type(0x28, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "sh")
    {
        translate_I_type(0x29, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "sw")
    {
        translate_I_type(0x2b, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "swl")
    {
        translate_I_type(0x2a, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "swr")
    {
        translate_I_type(0x2e, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "sc")
    {
        translate_I_type(0x38, regMap[instruction[3]], regMap[instruction[1]], stoi(instruction[2]));
    }
    else if (instruction[0] == "mfhi")
    {
        // mfhi rd
        translate_R_type(0, 0, 0, regMap[instruction[1]], 0, 0x10);
    }
    else if (instruction[0] == "mflo")
    {
        translate_R_type(0, 0, 0, regMap[instruction[1]], 0, 0x12);
    }
    else if (instruction[0] == "mthi")
    {
        // mthi rs
        translate_R_type(0, regMap[instruction[1]], 0, 0, 0, 0x11);
    }
    else if (instruction[0] == "mtlo")
    {
        translate_R_type(0, regMap[instruction[1]], 0, 0, 0, 0x13);
    }
    else if (instruction[0] == "syscall")
    {
        translate_R_type(0, 0, 0, 0, 0, 0xc);
    }

    else
    {
        // error
    }

    pc_+=4;
}

// int main()
// {

//     // sw $ra, 0($sp)
//     // translateInstruc(test);
//     readFile("/Users/jiaqishao/Desktop/project1/test.txt");
//     pc = 0x400000;
//     for (auto i : instructions)
//     {
//         translateInstruc(i);
//         pc += 4;
//     }

//     return 0;
// }
