#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <unordered_map>
#include <bitset>
#include "../include/assembler.h"
#include <fstream>
#include <unistd.h>
#include <fcntl.h>

/*
To simulate main memory, you need to
dynamically allocate a block of memory with C/C++, with a size of 6MB.
*/

using namespace std;

size_t max_size = 6 * 1024 * 1024; // 6MB
size_t text_segment = 1 * 1024 * 1024;
const u_int32_t start_addr = 0x400000;
const u_int32_t static_data_begin = start_addr + text_segment;
u_int32_t dynamic_data_begin = static_data_begin;
u_int32_t stack_begin = 0xA00000; // goes downwards, address will decrease;

char *real_mem = (char *)malloc(max_size); //  "real_mem" storing the real address of the block of memory allocated.

/*
* map virtual_mem to the real allocated memory
* return a pointer that stores the address of real memory
*/

vector<int32_t> reg_values(34, 0); // add LO, HI registers, LO at 32, HI at 33

u_int32_t pc = 0x400000;

unordered_map<std::string, int32_t> labels;
vector<vector<string>> instructions;
char* mapMem(u_int32_t virtual_mem);
void readData(string filename); //
void readText(string filename);
void putText(vector<int> instruction_machine_code);
void putData(string mode, string line);
void readInstruction();

void overflow() {
    cout << "Overflow!" << endl;
    exit(EXIT_FAILURE);
} // TO-DO  print out the error message and terminate the program execution.
void add(int rs, int rt, int rd)
{
    int rs_v = reg_values[rs];
    int rt_v = reg_values[rt];
    int tmp = rs_v + rt_v;
    if (rs_v < 0 && rt_v < 0 && tmp > 0)
        overflow();
    else if (rs_v > 0 && rt_v > 0 && tmp < 0)
        overflow();

    reg_values[rd] = rs_v + rt_v;
}
void addu(int rs, int rt, int rd)
{
    reg_values[rd] = (unsigned)reg_values[rs] + (unsigned)reg_values[rt];
}

void addi(int rs, int rt, int imm)
{
    // rt = rs + imm
    int rs_v = reg_values[rs];
    int tmp = rs_v + imm;
    if (rs_v < 0 && imm < 0 && tmp > 0)
        overflow();
    else if (rs_v > 0 && imm > 0 && tmp < 0)
        overflow();

    reg_values[rt] = rs_v + imm;
}
void addiu(int rs, int rt, int imm)
{
    reg_values[rt] = (unsigned)reg_values[rs] + (unsigned)imm;
}
void and_(int rs, int rt, int rd)
{
    // rd = rs & rt
    reg_values[rd] = reg_values[rt] & reg_values[rs];
}
void andi(int rs, int rt, int imm)
{
    // rt = rs & imm
    reg_values[rt] = reg_values[rs] & imm;
}
void clo(int rs, int rd)
{
    // To Count the number of leading ones in a word
    int tmp = 32;
    for (int i = 31; i >= 0; i--)
    {
        if (rs & (1 << i) == 0)
        {
            tmp = 32 - i;
            break;
        }
    }
    reg_values[rd] = tmp;
}
void clz(int rs, int rd)
{
    //Count the number of leading zeros in a word
    int tmp = 32;
    for (int i = 31; i >= 0; i--)
    {
        if (rs & (1 << i) != 0)
        {
            tmp = 32 - i;
            break;
        }
    }
    reg_values[rd] = tmp;
}
void div_(int rs, int rt)
{
    // (LO, HI) <- rs / rt
    reg_values[32] = reg_values[rs] / reg_values[rt];
    reg_values[33] = reg_values[rs] / reg_values[rt];
}
void divu(int rs, int rt)
{
    reg_values[32] = (unsigned)reg_values[rs] / (unsigned)reg_values[rt];
    reg_values[33] = (unsigned)reg_values[rs] / (unsigned)reg_values[rt];
}
void mult(int rs, int rt)
{
    int64_t tmp = (int64_t)reg_values[rs] * (int64_t)reg_values[rt];
    reg_values[32] = int32_t(tmp);
    reg_values[33] = int32_t(tmp >> 32);
}
void multu(int rs, int rt)
{
    u_int64_t tmp = (u_int64_t)reg_values[rs] * (u_int64_t)reg_values[rt];
    reg_values[32] = u_int32_t(tmp);
    reg_values[33] = u_int32_t(tmp >> 32);
}
void mul(int rs, int rt, int rd)
{
    int64_t tmp = (int64_t)reg_values[rs] * (int64_t)reg_values[rt];
    reg_values[rd] = int32_t(tmp);
}
void madd(int rs, int rt)
{
    // (LO,HI) <- (rs x rt) + (LO,HI)
    int64_t tmp = (int64_t)reg_values[rs] * reg_values[rt] + (int64_t)reg_values[33] << 32 | reg_values[32];
    reg_values[32] = (int32_t)tmp;
    reg_values[33] = (int32_t)(tmp >> 32);
}
void msub(int rs, int rt)
{
    int64_t tmp = ((int64_t)reg_values[33] << 32) | reg_values[32] - (int64_t)reg_values[rs] * reg_values[rt];
    reg_values[32] = (int32_t)tmp;
    reg_values[33] = (int32_t)(tmp >> 32);
}
void maddu(int rs, int rt)
{
    u_int64_t tmp = (u_int64_t)reg_values[rs] * (u_int64_t)reg_values[rt] + (u_int64_t)reg_values[33] << 32 | reg_values[32];
    reg_values[32] = (u_int32_t)tmp;
    reg_values[33] = (u_int32_t)(tmp >> 32);
}
void msubu(int rs, int rt)
{
    u_int64_t tmp = ((u_int64_t)reg_values[33] << 32) | reg_values[32] - (u_int64_t)reg_values[rs] * (u_int64_t)reg_values[rt];
    reg_values[32] = (u_int32_t)tmp;
    reg_values[33] = (u_int32_t)(tmp >> 32);
}
void nor(int rs, int rt, int rd)
{
    // rd <- rs NOR rt
    reg_values[rd] = ~(reg_values[rs] | reg_values[rt]);
}
void or_(int rs, int rt, int rd)
{
    reg_values[rd] = (reg_values[rs] | reg_values[rt]);
}
void ori(int rs, int rt, int imm)
{
    reg_values[rt] = (reg_values[rs] | imm);
}
void sll(int rt, int rd, int shamt)
{
    // rd <- rt << sa
    reg_values[rd] = reg_values[rt] << shamt;
}
void sllv(int rs, int rt, int rd)
{
    // rd <- rt << rs
    reg_values[rd] = reg_values[rt] << reg_values[rs];
}
void sra(int rt, int rd, int shamt)
{
    reg_values[rd] = (reg_values[rt] >> shamt);
}
void srav(int rs, int rt, int rd)
{
    reg_values[rd] = reg_values[rt] >> reg_values[rs];
}
void srl(int rt, int rd, int shamt)
{
    reg_values[rd] = (reg_values[rt] >> shamt) & ((1 << (32 - shamt)) - 1);
}
void srlv(int rs, int rt, int rd)
{
    reg_values[rd] = (reg_values[rt] >> reg_values[rs]) & ((1 << (32 - rs)) - 1);
}
void sub(int rs, int rt, int rd)
{
    //rd <- rs - rt
    int rs_v = reg_values[rs];
    int rt_v = reg_values[rt];
    int tmp = rs_v - rt_v;
    if (rs_v > 0 && rt_v < 0 && tmp < 0)
        overflow();
    else if (rs_v < 0 && rt_v > 0 && tmp > 0)
        overflow();
    else
        reg_values[rd] = tmp;
}
void subu(int rs, int rt, int rd)
{

    reg_values[rd] = (u_int32_t)reg_values[rs] - (u_int32_t)reg_values[rt];
}
void xor_(int rs, int rt, int rd)
{
    reg_values[rd] = reg_values[rs] ^ reg_values[rt];
}
void xori(int rs, int rt, int imm)
{
    reg_values[rt] = reg_values[rs] ^ imm;
}
void lui(int rt, int imm)
{
    reg_values[rt] = imm << 16;
}
void slt(int rs, int rt, int rd)
{
    // To record the result of a less-than comparison
    reg_values[rd] = (reg_values[rs] < reg_values[rt]) ? 1 : 0;
}
void sltu(int rs, int rt, int rd)
{
    reg_values[rd] = ((unsigned)reg_values[rs] < (unsigned)reg_values[rt]) ? 1 : 0;
}
void slti(int rs, int rt, int imm)
{
    // rt <- (rs < immediate)
    reg_values[rt] = (reg_values[rs] < imm) ? 1 : 0;
}
void sltiu(int rs, int rt, int imm)
{
    reg_values[rt] = ((unsigned)reg_values[rs] < (unsigned)imm) ? 1 : 0;
}
void beq(int rs, int rt, int offset)
{
    // PC should keep one word forward.
    int target_offset = offset << 2;
    if (reg_values[rs] == reg_values[rt])
        pc = pc + target_offset;
}
void bgez(int rs, int offset)
{
    int target_offset = offset << 2;
    if (reg_values[rs] >= 0)
        pc = pc + target_offset;
}
void bgezal(int rs, int offset)
{
    // if rs >= 0 then procedure_call

    int target_offset = offset << 2;
    reg_values[31] = pc + 4;
    if (reg_values[rs] >= 0)
        pc = pc + target_offset;
}
void bgtz(int rs, int offset)
{

    int target_offset = offset << 2;
    if (reg_values[rs] > 0)
        pc = pc + target_offset;
}
void blez(int rs, int offset)
{
    int target_offset = offset << 2;
    if (reg_values[rs] <= 0)
        pc = pc + target_offset;
}
void bltzal(int rs, int offset)
{
    int target_offset = offset << 2;
    reg_values[31] = pc + 4;
    if (reg_values[rs] < 0)
        pc = pc + target_offset;
}
void bltz(int rs, int offset)
{
    int target_offset = offset << 2;
    if (reg_values[rs] < 0)
        pc = pc + target_offset;
}
void bne(int rs, int rt, int offset)
{
    int target_offset = offset << 2;
    if (reg_values[rs] != reg_values[rt])
        pc = pc + target_offset;
}
void j(int target)
{
    int pc_4 = pc >> 2 << 28; // 4 bits of pc
    pc = pc_4 + target << 2;
}
void jal(int target)
{
    reg_values[31] = pc + 4;
    int pc_4 = pc >> 2 << 28; // 4 bits of pc
    pc = pc_4 + target << 2;
}
void jalr(int rs, int rd)
{
    reg_values[rd] = pc + 4;
    pc = reg_values[rs];
}
void jr(int rs)
{
    pc = reg_values[rs];
}
void teq(int rs, int rt)
{
    if (reg_values[rs] == reg_values[rt])
    {
        cout << "Trap if equal" << endl;
        exit(EXIT_FAILURE);
    }
}
void teqi(int rs, int imm)
{
    if (reg_values[rs] == imm)
    {
        cout << "Trap if equal immediate" << endl;
        exit(EXIT_FAILURE);
    }
}
void tne(int rs, int rt)
{
    if (reg_values[rs] != reg_values[rt])
    {
        cout << "Trap if not equal" << endl;
        exit(EXIT_FAILURE);
    }
}
void tnei(int rs, int imm)
{
    if (reg_values[rs] != imm)
    {
        cout << "Trap if not equal immediate" << endl;
        exit(EXIT_FAILURE);
    }
}
void tge(int rs, int rt)
{
    if (reg_values[rs] >= reg_values[rt])
    {
        cout << "Trap if greater or equal" << endl;
        exit(EXIT_FAILURE);
    }
}
void tgeu(int rs, int rt)
{
    if ((unsigned)reg_values[rs] >= (unsigned)reg_values[rt])
    {
        cout << "Trap if greater or equal unsigned" << endl;
        exit(EXIT_FAILURE);
    }
}
void tgei(int rs, int imm)
{
    if (reg_values[rs] >= imm)
    {
        cout << "Trap if greater or equal" << endl;
        exit(EXIT_FAILURE);
    }
}
void tgeiu(int rs, int imm)
{
    if ((unsigned)reg_values[rs] >= (unsigned)imm)
    {
        cout << "Trap if greater or equal unsigned" << endl;
        exit(EXIT_FAILURE);
    }
}
void tlt(int rs, int rt)
{
    if (reg_values[rs] < reg_values[rt])
    {
        cout << "Trap if less than" << endl;
        exit(EXIT_FAILURE);
    }
}
void tltu(int rs, int rt)
{
    if ((unsigned)reg_values[rs] < (unsigned)reg_values[rt])
    {
        cout << "Trap if less than unsigned" << endl;
        exit(EXIT_FAILURE);
    }
}
void tlti(int rs, int imm)
{
    if (reg_values[rs] < imm)
    {
        cout << "Trap if less than immediate" << endl;
        exit(EXIT_FAILURE);
    }
}
void tltiu(int rs, int imm)
{
    if ((unsigned)reg_values[rs] < (unsigned)imm)
    {
        cout << "Trap if less than immediate" << endl;
        exit(EXIT_FAILURE);
    }
}
void lb(int base, int rt, int offset)
{
    reg_values[rt] = *mapMem(reg_values[base] + offset * 4);
}
void lbu(int base, int rt, int offset)
{
    reg_values[rt] = (unsigned)*mapMem(reg_values[base] + offset * 4);
}
void lh(int base, int rt, int offset)
{
    reg_values[rt] = *reinterpret_cast<int16_t *>(mapMem(reg_values[base] + offset * 4));
}
void lhu(int base, int rt, int offset)
{
    reg_values[rt] = (unsigned)*reinterpret_cast<int16_t *>(mapMem(reg_values[base] + offset * 4));
}
void lw(int base, int rt, int offset)
{
    reg_values[rt] = *reinterpret_cast<int32_t *>(mapMem(reg_values[base] + offset * 4));
}
void lwl(int base, int rt, int offset)
{
    int initial_content = reg_values[rt];
    int target_address = reg_values[rt] + 4 * offset;
    int mem_content = *reinterpret_cast<int32_t *>(mapMem(target_address));

    int low_2 = base & ((1 << 2) - 1); // lowest two bits of base

    switch (low_2)
    {
    case 0:
        reg_values[rt] = mem_content << 24 + (initial_content & ((1 << 24) - 1));
        break;
    case 1:
        reg_values[rt] = mem_content << 16 + (initial_content & ((1 << 16) - 1));
        break;
    case 2:
        reg_values[rt] = mem_content << 8 + (initial_content & ((1 << 8) - 1));
        break;
    case 3:
        reg_values[rt] = mem_content;

        break;
    default:
        break;
    }
}
void lwr(int base, int rt, int offset)
{
    int initial_content = reg_values[rt];
    int target_address = reg_values[rt] + 4 * offset;
    int mem_content = *reinterpret_cast<int32_t *>(mapMem(target_address));

    int low_2 = base & ((1 << 2) - 1); // lowest two bits of base

    switch (low_2)
    {
    case 0:
        reg_values[rt] = mem_content;
        break;
    case 1:
        reg_values[rt] = (initial_content & (255 << 24)) + (mem_content >> 8);
        break;
    case 2:
        reg_values[rt] = (initial_content & ((1 << 16) - 1)) + (mem_content >> 16);
        break;
    case 3:
        reg_values[rt] = (initial_content & (1 << 24) - 1) + (mem_content >> 24);
        break;
    default:
        break;
    }
}
void ll(int base, int rt, int offset)
{
    // Load Linked Word
    reg_values[rt] = reg_values[base] + offset * 4;
}
void sb(int base, int rt, int offset)
{
    *mapMem(reg_values[base] + offset * 4) = reg_values[rt];
}
void sh(int base, int rt, int offset)
{
    *reinterpret_cast<int16_t *>(mapMem(reg_values[base] + offset * 4)) = reg_values[rt];
}
void sw(int base, int rt, int offset)
{
    *reinterpret_cast<int32_t *>(mapMem(reg_values[base] + offset * 4)) = reg_values[rt];
}
void swl(int base, int rt, int offset)
{
    int initial_content = reg_values[rt];
    int target_address = reg_values[rt] + 4 * offset;
    int32_t *p = reinterpret_cast<int32_t *>(mapMem(target_address));
    int mem_content = *p;

    int low_2 = base & ((1 << 2) - 1); // lowest two bits of base

    switch (low_2)
    {
    case 0:
        *p = mem_content & (((1 << 24) - 1) << 8) + (initial_content >> 24);
        break;
    case 1:
        *p = mem_content & (((1 << 16) - 1) << 16) + (initial_content >> 16);
        break;
    case 2:
        *p = mem_content & (((1 << 8) - 1) << 24) + (initial_content >> 8);
        break;
    case 3:
        *p = initial_content;

        break;
    default:
        break;
    }
}
void swr(int base, int rt, int offset)
{
    int initial_content = reg_values[rt];
    int target_address = reg_values[rt] + 4 * offset;
    int32_t *p = reinterpret_cast<int32_t *>(mapMem(target_address));
    int mem_content = *p;

    int low_2 = base & ((1 << 2) - 1); // lowest two bits of base

    switch (low_2)
    {
    case 0:
        *p = initial_content;
        break;
    case 1:
        *p = (initial_content << 8) + (mem_content & ((1 << 8) - 1));
        break;
    case 2:
        *p = (initial_content << 16) + (mem_content & ((1 << 16) - 1));
        break;
    case 3:
        *p = (initial_content << 24) + (mem_content & ((1 << 24) - 1));
        break;
    default:
        break;
    }
}
void sc(int base, int rt, int offset)
{
    *reinterpret_cast<int32_t *>(mapMem(reg_values[base] + offset * 4)) = reg_values[rt];
}
void mfhi(int rd)
{
    reg_values[rd] = reg_values[33];
}
void mflo(int rd)
{
    reg_values[rd] = reg_values[32];
}
void mthi(int rs)
{
    reg_values[33] = reg_values[rs];
}
void mtlo(int rs)
{
    reg_values[32] = reg_values[rs];
}
void syscall()
{
    // 1, 4, 5, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17
    int v0 = reg_values[2];
    fstream file;

    switch (v0)
    {
    case 1: // print_int
        printf("%d", reg_values[4]);
        break;
    case 4: // print_stirng
        cout << mapMem(reg_values[4]) << endl;
        break;
    case 5: // read_int
        scanf("%d", &reg_values[2]);
        break;
    case 8:
        // For specified length n, string can be no longer than n-1.
        // If less than that, adds newline to end. In either case, then pads with null byte
        // If n = 1, input is ignored and null byte placed at buffer address.
        // If n < 1, input is ignored and nothing is written to the buffer.
        {
            char str[10000];
            memset(str, '\0', sizeof(str));
            int maxlen = reg_values[5];
            int addr = reg_values[4]; // address of input buffer
            scanf("%s", str);
            for (int i = 0; i < maxlen - 1; i++)
            {
                if (str[i] == '\0')
                {
                    *mapMem(addr++) = '\n'; // add new line;
                    break;
                }
                *mapMem(addr++) = str[i];
            }
            break;
        }
    case 9:
        reg_values[2] = dynamic_data_begin; // v0 <- address
        dynamic_data_begin += reg_values[4];
        break;
    case 10:
        exit(0);
        break;
    case 11:
        // Prints ASCII character corresponding to contents of low-order byte.
        // 	$a0 = character to print
        {
            int char_32 = reg_values[4] & ((1 << 8) - 1); // low-order byte (last 8 bits);
            char char_8 = (char)char_32;
            cout << char_8 << endl;
            break;
        }
    case 12:
        // read character
        scanf("%c", reg_values[4]);
        break;
    case 13:
    {
        // $a0 = address of null-terminated string containing filename
        // $a1 = flags
        // $a2 = mode
        // $a0 contains file descriptor (negative if error)
        // three flag values: 0 for read-only, 1 for write-only with create, and 9 for write-only with create and append.
        // The returned file descriptor will be negative if the operation failed.
        // File descriptors 0, 1 and 2 are always open for: reading from standard input, writing to standard output, and writing to standard error, respectively
        {
            int flag = reg_values[5];
            int mode = reg_values[6];
            reg_values[4] = open(mapMem(reg_values[4]), flag, mode);
            break;
        }
    }
    case 14:
        {
            int fd = reg_values[4];
            int address_of_input_buffer = reg_values[5];
            int len = reg_values[6];
            reg_values[4] = read(fd, mapMem(address_of_input_buffer), len);
            break;
        }

    case 15:
        {
            int fd = reg_values[4];
            int address_of_input_buffer = reg_values[5];
            int len = reg_values[6];
            reg_values[4] = write(fd, mapMem(address_of_input_buffer), len);

            break;
        }
    case 16:
        {
            int fd = reg_values[4];
            close(fd);
            break;
        }
    case 17:
        {
            int result = reg_values[4];
            exit(result);
            break;
        }
    default:
        break;
    }
}
/*
* initialize the registers
*/
void init()
{
    reg_values[29] = 0xA00000; //sp
}

char* mapMem(u_int32_t virtual_mem)
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
            *reinterpret_cast<int32_t *>(mapMem(dynamic_data_begin)) = stoi(i); // 32-bit quantities
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
            *reinterpret_cast<int16_t *>(mapMem(dynamic_data_begin)) = (int16_t)stoi(i); // 16-bit quantities
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
        if (line.find(".text") != string::npos)
            break; // text segment
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

void putText(vector<int> instruction_machine_code)
{
    u_int32_t text_addr = start_addr;
    for (int32_t i : instruction_machine_code)
    {
        *reinterpret_cast<int32_t *>(mapMem(text_addr)) = i;
        // cout << i << endl;
        text_addr += 4;
    }
}

void readInstruction()
{

    u_int32_t pc = 0x400000;
    while (*mapMem(pc))
    {
        int32_t mc = *mapMem(pc);

        int op = mc >> 26;
        int func = mc & 0x111111; // last 6 bits
        int rs = (mc >> 21) & 0x11111;
        int rt = (mc >> 16) & 0x11111;
        int rd = (mc >> 11) & 0x11111;
        int shamt = (mc >> 6) & 0x11111;

        if (op == 0)
        {
            if (func == 0x20)
                add(rs, rt, rd);
            else if (func == 0x21)
                addu(rs, rt, rd);
            else if (func == 0x24)
                and_(rs, rt, rd);
            else if (func == 0x1a)
                div_(rs, rt);
            else if (func == 0x1b)
                divu(rs, rt);
            else if (func == 0x18)
                mult(rs, rt);
            else if (func == 0x19)
                multu(rs, rt);
            else if (func == 0x27)
                nor(rs, rt, rd);
            else if (func == 0x25)
                or_(rs, rt, rd);
            else if (func == 0)
                sll(rt, rd, (mc >> 6) & 0x11111);
            else if (func == 2)
                srl(rt, rd, (mc >> 6) & 0x11111);
            else if (func == 3)
                sra(rt, rd, (mc >> 6) & 0x11111);
            else if (func == 4)
                sllv(rs, rt, rd);
            else if (func == 6)
                srlv(rs, rt, rd);
            else if (func == 7)
                srav(rs, rt, rd);
            else if (func == 8)
                jr(rs);
            else if (func == 9)
                jalr(rs, rd);
            else if (func == 0xc)
                syscall();
            else if (func == 0x10)
                mfhi(rd);
            else if (func == 0x11)
                mthi(rs);
            else if (func == 0x12)
                mflo(rd);
            else if (func == 0x13)
                mtlo(rs);

            else if (func == 0x22)
                sub(rs, rt, rd);
            else if (func == 0x23)
                subu(rs, rt, rd);
            else if (func == 0x26)
                xor_(rs, rt, rd);
            else if (func == 0x2a)
                slt(rs, rt, rd);
            else if (func == 0x2b)
                sltu(rs, rt, rd);
            else if (func == 0x30)
                tge(rs, rt);
            else if (func == 0x31)
                tgeu(rs, rt);
            else if (func == 0x32)
                tlt(rs, rt);
            else if (func == 0x33)
                tltiu(rs, mc & ((1 << 16) - 1));
            else if (func == 0x34)
                teq(rs, rt);
            else if (func == 0x36)
                tne(rs, rt);
        }

        else if (op == 1)
        {
            if (rt == 1)
                bgez(rs, mc & ((1 << 16) - 1));
            else if (rt == 0x11)
                bgezal(rs, mc & ((1 << 16) - 1));
            else if (rt == 0x10)
                bltzal(rs, mc & ((1 << 16) - 1));
            else if (rt == 0)
                bltz(rs, mc & ((1 << 16) - 1));
            else if (rt == 8)
                tgei(rs, mc & ((1 << 16) - 1));
            else if (rt == 9)
                tgeiu(rs, mc & ((1 << 16) - 1));
            else if (rt == 0xb)
                tltiu(rs, mc & ((1 << 16) - 1));
            else if (rt == 0xc)
                teqi(rs, mc & ((1 << 16) - 1));
            else if (rt == 0xe)
                tnei(rs, mc & ((1 << 16) - 1));
        }

        else if (op == 2)
            j((mc << 6) >> 6);
        else if (op == 3)
            jal((mc << 6) >> 6);

        else if (op == 4)
            beq(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 5)
            bne(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 6)
            blez(rs, mc & ((1 << 16) - 1));
        else if (op == 7)
            bgtz(rs, mc & ((1 << 16) - 1));
        else if (op == 8)
            addi(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 9)
            addiu(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0xc)
            andi(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x1c)
        {
            if (func == 0x21)
                clo(rs, rd);
            else if (func == 0x20)
                clz(rs, rd);
            else if (func == 2)
                mul(rs, rt, rd);
            else if (func == 0)
                madd(rs, rt);
            else if (func == 1)
                maddu(rs, rt);
            else if (func == 4)
                msub(rs, rt);
            else if (func == 5)
                msubu(rs, rt);
        }
        else if (op == 0xa)
            slti(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0xb)
            sltiu(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0xd)
            ori(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0xe)
            xori(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0xf)
            lui(rt, mc & ((1 << 16) - 1));

        else if (op == 0x20)
            lb(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x21)
            lh(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x22)
            lwl(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x23)
            lw(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x24)
            lbu(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x25)
            lhu(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x26)
            lwr(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x28)
            sb(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x29)
            sh(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x2a)
            swl(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x2b)
            sw(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x2e)
            swr(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x30)
            ll(rs, rt, mc & ((1 << 16) - 1));
        else if (op == 0x38)
            sc(rs, rt, mc & ((1 << 16) - 1));

        pc += 4;
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
    string filename = "/Users/jiaqishao/Downloads/simulator-samples/a-plus-b.asm";
    init();             // set register
    readData(filename); // get addresss where the dynamic data begins;
    Assembler assembler;
    assembler.readFile(filename);
    for (auto i : assembler.instructions)
    {
        assembler.translateInstruc(i);
    }

    putText(assembler.instruction_machine_code);

    readInstruction();
    // u_int32_t static_data_addr = static_data_begin;

    // string line = ".half 23, 255, 120";
    // putData("half", line);

    // int32_t v = 0x500000;
    // auto res = mapMem(v);
    // cout << hex <<  res;

    return 0;
}