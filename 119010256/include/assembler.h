#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <unordered_map>
#include <vector>


using namespace std;
class Assembler {
    public:
        vector<vector<string> > instructions;
        vector<int> instruction_machine_code;
        unordered_map<string, int32_t> labels;
        unordered_map<string, int> regMap;

        
        void translate_R_type(int op, int rs, int rt, int rd, int shamt, int funct);
        void translate_I_type(int op, int rs, int rt, int addr);
        void translate_J_type(int op, int addr);
        void readFile(std::string filename);
        void translateInstruc(vector<string> instruction);
        void set_regMap();       
        int is_valid_label(string label);
        
};


vector<string> split(const string &str, const string &delim);



#endif