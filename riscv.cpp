// Author: Ece Dilara ASLAN
//         Salih Bedirhan EKER
// Date: 17.02.2021
// Compiling status: Compiling
// Working status: Working
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>

using namespace std;

// IF/ID register it is implemented as struct data type
struct IF_ID{

    string inst[6];       // keeps data of one instruction
    long int PC;           // keep Program Counter

    void copy(struct IF_ID newValue){           // copy() is used for copying one IF/ID register to another one
        this->inst[0] = newValue.inst[0];       // name of the instruction
        this->inst[1] = newValue.inst[1];       // source register 1
        this->inst[2] = newValue.inst[2];       // source register 2
        this->inst[3] = newValue.inst[3];       // destination register
        this->inst[4] = newValue.inst[4];       // offset
        this->inst[5] = newValue.inst[5];       // label name
        this->PC = newValue.PC;                 // PC
    }

};

// ID/EX register
struct ID_EX{

    long int PC;        // Program Counter
    long int src1;      // value of source register 1
    long int src2;      // value of source register 2
    long int imm;       // immediate value
    int src1_reg;       // source register 1
    int src2_reg;       // source register 2
    int dest;           // destination register
    int ALUop;          // control value of ALOop
    bool ALUsrc;        // control value of ALUsrc
    bool mem_write;     // control value of MemWrite
    bool mem_read;      // control value of MemRead
    bool branch;        // control value of Branch
    bool reg_write;     // control value of RegWrite
    bool mem_to_reg;    // control value of MemtoReg

    void copy(struct ID_EX newValue){                     // copy() is used for copying one ID/EX register to another one
        this->PC = newValue.PC;
        this->src1 = newValue.src1;
        this->src2 = newValue.src2;
        this->imm = newValue.imm;
        this->src1_reg = newValue.src1_reg;
        this->src2_reg = newValue.src2_reg;
        this->dest = newValue.dest;
        this->ALUop = newValue.ALUop;
        this->ALUsrc = newValue.ALUsrc;
        this->mem_write = newValue.mem_write;
        this->mem_read = newValue.mem_read;
        this->branch = newValue.branch;
        this->reg_write = newValue.reg_write;
        this->mem_to_reg = newValue.mem_to_reg;
    }

};

// EX/MEM register
struct EX_MEM{

    long int PC;                // Program Counter
    long int src2;              // value of source register 2
    int dest;                   // destination register
    long int ALUresult;         // result of the ALU
    long int branch_target;     // address of target of branch
    bool zero;                  // control value of zero
    bool mem_write;             // control value of MemWrite
    bool mem_read;              // control value of MemRead
    bool branch;                // control value of Branch
    bool reg_write;             // control value of RegWrite
    bool mem_to_reg;            // control value of MemtoReg

    void copy(struct EX_MEM newValue){              // copy() is used for copying one EX/MEM register to another one
        this->PC = newValue.PC;
        this->src2 = newValue.src2;
        this->dest = newValue.dest;
        this->ALUresult = newValue.ALUresult;
        this->branch_target = newValue.branch_target;
        this->zero = newValue.zero;
        this->mem_write = newValue.mem_write;
        this->mem_read = newValue.mem_read;
        this->branch = newValue.branch;
        this->reg_write = newValue.reg_write;
        this->mem_to_reg = newValue.mem_to_reg;
    }

};


// MEM/WB register
struct MEM_WB{

    int dest;               // destination register
    long int ALUresult;     // result of ALU
    long int mem_value;     // value of the address in memory
    bool reg_write;         // control value of RegWrite
    bool mem_to_reg;        // control value of MemtoReg

    void copy(struct MEM_WB newValue){              // copy() is used for copying one MEM/WB register to another one
        this->dest = newValue.dest;
        this->ALUresult = newValue.ALUresult;
        this->mem_value = newValue.mem_value;
        this->reg_write = newValue.reg_write;
        this->mem_to_reg = newValue.mem_to_reg;
    }

};

string instruction_memory[1024][6];                      // instruction memory each entry holds an instruction token by token
vector<string> instructions;                            // stores instructions
map<string,long int> labels;                            // stores addresses of labels
long int PC = 0;                                        // Program Counter
long int registers[32] = {0};                           // registers
long int data_memory[4096];                             // data memory
bool IFID_write = true;                                 // control value of write to IF/ID register
int cycle = 0;                                          // counts the number of cycles
int stall = 0;                                          // counts the number of stalls
vector<string> stall_inst;                              // keeps the instructions that cause stalls

void line_handler(vector<string> tokens, int count){        // used when taking input. It takes a line and stores in instruction memory in the proper form

    if(tokens[0] == "add" || tokens[0] == "sub" || tokens[0] == "and" || tokens[0] == "or"){    // if the instruction is R-Type program needs to store in 0,1,2,3. indexes
        instruction_memory[count][0] = tokens[0];
        instruction_memory[count][3] = tokens[1].substr(1,tokens[1].length());
        instruction_memory[count][1] = tokens[2].substr(1,tokens[2].length());
        instruction_memory[count][2] = tokens[3].substr(1,tokens[3].length());
    }
    else if(tokens[0] == "ld"){                                                                 // if the instruction is I-Type program needs to store 0,1,3,4. indexes 
        instruction_memory[count][0] = tokens[0];
        instruction_memory[count][3] = tokens[1].substr(1,tokens[1].length());
        instruction_memory[count][4] = tokens[2];
        instruction_memory[count][1] = tokens[3].substr(1,tokens[3].length());
    }
    else if(tokens[0] == "sd"){                                                                 // if the instruction is S-Type program needs to store 0,1,2,4. indexes
        instruction_memory[count][0] = tokens[0];
        instruction_memory[count][2] = tokens[1].substr(1,tokens[1].length());
        instruction_memory[count][4] = tokens[2];
        instruction_memory[count][1] = tokens[3].substr(1,tokens[3].length());
    }
    else if(tokens[0] == "beq"){                                                                // if the instruction is SB-Type program needs to store 0,1,2,5. indexes
        instruction_memory[count][0] = tokens[0];
        instruction_memory[count][1] = tokens[1].substr(1,tokens[1].length());
        instruction_memory[count][2] = tokens[2].substr(1,tokens[2].length());
        instruction_memory[count][5] = tokens[3];
    }
    else{                                                 // if the line includes a label                                                
        labels[tokens[0]] = count;                          
        if(tokens.size() != 1){                          // if the line includes a label and an instruction call line handler for the instruction one more
            tokens.erase(tokens.begin());
            line_handler(tokens, count);
        }
    }

}

void control(string inst_name, struct ID_EX* new_IDEX){         // sets all the control flags according to type of the instruction
    if(inst_name == "add"){                 // sets necessary flags for add instruction
        new_IDEX->ALUop = 1;
        new_IDEX->ALUsrc = false;
        new_IDEX->mem_write = false;
        new_IDEX->mem_read = false;
        new_IDEX->branch = false;
        new_IDEX->reg_write = true;
        new_IDEX->mem_to_reg = false;
    }
    else if(inst_name == "sub"){            // sets necessary flags for sub instruction
        new_IDEX->ALUop = 2;
        new_IDEX->ALUsrc = false;
        new_IDEX->mem_write = false;
        new_IDEX->mem_read = false;
        new_IDEX->branch = false;
        new_IDEX->reg_write = true;
        new_IDEX->mem_to_reg = false;
    }
    else if(inst_name == "and"){            // sets necessary flags for and instruction
        new_IDEX->ALUop = 3;
        new_IDEX->ALUsrc = false;
        new_IDEX->mem_write = false;
        new_IDEX->mem_read = false;
        new_IDEX->branch = false;
        new_IDEX->reg_write = true;
        new_IDEX->mem_to_reg = false;
    }
    else if(inst_name == "or"){             // sets necessary flags for or instruction
        new_IDEX->ALUop = 4;
        new_IDEX->ALUsrc = false;
        new_IDEX->mem_write = false;
        new_IDEX->mem_read = false;
        new_IDEX->branch = false;
        new_IDEX->reg_write = true;
        new_IDEX->mem_to_reg = false;
    }
    else if(inst_name == "ld"){             // sets necessary flags for ld instruction
        new_IDEX->ALUop = 1;
        new_IDEX->ALUsrc = true;
        new_IDEX->mem_write = false;
        new_IDEX->mem_read = true;
        new_IDEX->branch = false;
        new_IDEX->reg_write = true;
        new_IDEX->mem_to_reg = true;
    }
    else if(inst_name == "sd"){             // sets necessary flags for sd instruction
        new_IDEX->ALUop = 1;
        new_IDEX->ALUsrc = true;
        new_IDEX->mem_write = true;
        new_IDEX->mem_read = false;
        new_IDEX->branch = false;
        new_IDEX->reg_write = false;
        new_IDEX->mem_to_reg = false;
    }
    else if(inst_name == "beq"){            // sets necessary flags for beq instruction
        new_IDEX->ALUop = 2;
        new_IDEX->ALUsrc = false;
        new_IDEX->mem_write = false;
        new_IDEX->mem_read = false;
        new_IDEX->branch = true;
        new_IDEX->reg_write = false;
        new_IDEX->mem_to_reg = false;
    }
    else{                                   // sets necessary flags for stall (all flags must be zero)
        new_IDEX->ALUop = 0;
        new_IDEX->ALUsrc = false;
        new_IDEX->mem_write = false;
        new_IDEX->mem_read = false;
        new_IDEX->branch = false;
        new_IDEX->reg_write = false;
        new_IDEX->mem_to_reg = false;
    }
}

int main(int argc, char* argv[]){

    ifstream file;
    file.open(argv[1], ios::in);                    // file stream to get the input file

    string instruction_temp;
    int inst_count = 0;                             // instruction count is zero initially
    
    while(getline(file, instruction_temp)){         // loop that gets line by line the input file
        vector<string> go_handler;
        string const delims = " ,:()" ;
        size_t beg, pos = 0;
        while ((beg = instruction_temp.find_first_not_of(delims, pos)) != string::npos)     // tokenizer
        {
            pos = instruction_temp.find_first_of(delims, beg + 1);
            go_handler.push_back(instruction_temp.substr(beg, pos - beg));
        }
        line_handler(go_handler, inst_count);                               // it stores the information into instruction_memory
        if(go_handler.size() == 4 || go_handler.size() == 5){               // if the line includes an instruction
            inst_count++;
            size_t found = instruction_temp.find(':');
            if(found != string::npos){                                      // if the line includes label and an instruction                  
                instructions.push_back(instruction_temp.substr(found+1,instruction_temp.length()));
            }
            else{                                                           // the line has only an instruction
                instructions.push_back(instruction_temp);
            }
        }
    }

    struct IF_ID old_IFID;                      // old values of the IF/ID register
    struct IF_ID new_IFID;                      // new values of the IF/ID register
    new_IFID.inst[0] = "";                      // initialization of the IF/ID register
    new_IFID.inst[1] = "";                      // initially it does not keep any instruction
    new_IFID.inst[2] = "";
    new_IFID.inst[3] = "";
    new_IFID.inst[4] = "";
    new_IFID.inst[5] = "";
    new_IFID.PC = 0;                            // PC is initially zero
    struct ID_EX old_IDEX;                                                                        // old values of the ID/EX register
    struct ID_EX new_IDEX = {0, 0, 0, 0, 0, 0, 0, 0, false, false, false, false, false, false};   // new values of the ID/EX register
    struct EX_MEM old_EXMEM;                                                                      // old values of the EX/MEM register
    struct EX_MEM new_EXMEM = {0, 0, 0, 0, 0, false, false, false, false, false, false};          // new values of the EX/MEM register
    struct MEM_WB old_MEMWB;                                                                      // old values of the MEM/WB register
    struct MEM_WB new_MEMWB = {0, 0, 0, false, false};                                            // new values of the MEM/WB register

    while(PC != inst_count+4){                         //*****************      OPERATIONS DONE IN ONE CYCLE        ***************//
        if(IFID_write){                         // if there is no stall, program copy the new IF/ID register to old IF/ID register in order not to lose any information needed            
            old_IFID.copy(new_IFID);
        }
        else{                                   // if there is a stall program must not copy the values and make IFID_write true for next cycle
            IFID_write = true;
        }
        old_IDEX.copy(new_IDEX);                                // copy the new ID/EX register to old ID/EX register
        old_EXMEM.copy(new_EXMEM);                              // copy the new EX/MEM register to old EX/MEM register
        old_MEMWB.copy(new_MEMWB);                              // copy the new MEM/WB register to old MEM/WB register
        //******************************************// WB
        if(old_MEMWB.reg_write && old_MEMWB.dest != 0){                 // if dest register is not x0 and RegWrite is set, then write to register
            if(old_MEMWB.mem_to_reg){
                registers[old_MEMWB.dest] = old_MEMWB.mem_value;        // write a memory value to register
            }
            else{
                registers[old_MEMWB.dest] = old_MEMWB.ALUresult;        // write an ALU result to register 
            }
        }
        //******************************************// IF
        new_IFID.inst[0] = instruction_memory[PC][0];                   // gets the instruction name from instruction memory
        new_IFID.inst[1] = instruction_memory[PC][1];                   // gets the source register 1 from instruction memory
        new_IFID.inst[2] = instruction_memory[PC][2];                   // gets the source register 2 from instruction memory
        new_IFID.inst[3] = instruction_memory[PC][3];                   // gets the destination register from instruction memory
        new_IFID.inst[4] = instruction_memory[PC][4];                   // gets the offset from instruction memory
        new_IFID.inst[5] = instruction_memory[PC][5];                   // gets the label name from instruction memory
        new_IFID.PC = PC;                                               // gets the current value to IF/ID register of PC
        //******************************************// ID
        new_IDEX.PC = old_IFID.PC;                                      // carry the PC value to ID/EX register
        if(old_IFID.inst[1] != ""){                                     // if source register 1 exist
            new_IDEX.src1 = registers[stoi(old_IFID.inst[1])];          // store value of source register 1 to ID/EX register
        }
        if(old_IFID.inst[2] != ""){                                     // if source register 2 exist
            new_IDEX.src2 = registers[stoi(old_IFID.inst[2])];          // store value of source register 2 to ID/EX register
        }
        if(old_IFID.inst[1] != ""){                                     // if source register 1 exist
            new_IDEX.src1_reg = stoi(old_IFID.inst[1]);                 // store source register 1 to ID/EX register
        }
        if(old_IFID.inst[2] != ""){                                     // if source register 2 exist
            new_IDEX.src2_reg = stoi(old_IFID.inst[2]);                 // store source register 2 to ID/EX register
        }
        if(old_IFID.inst[3] != ""){                                     // if destination register exist
            new_IDEX.dest = stoi(old_IFID.inst[3]);                     // store destination register to ID/EX register
        }
        if(old_IFID.inst[4] != "" && old_IFID.inst[0] != "beq"){        // if the instruction is ld or sd
            new_IDEX.imm = stol(old_IFID.inst[4]);                      // store offset to ID/EX register
        }
        else if(old_IFID.inst[0] == "beq"){                             // if the instruction is beq
            new_IDEX.imm = labels[old_IFID.inst[5]];                    // store address of branch target to ID/EX register
        }
        control(old_IFID.inst[0], &new_IDEX);                           // set the control values of the instruction
        //******************************************// EX
        new_EXMEM.PC = old_IDEX.PC;                                     // carry the PC value to EX/MEM register
        new_EXMEM.dest = old_IDEX.dest;                                 // carry the destination register to EX/MEM register
        new_EXMEM.branch_target = old_IDEX.imm;                         // carry the branch target to EX/MEM register
                                                                                                    //*********** FORWARDING UNIT
        if(old_EXMEM.reg_write && old_EXMEM.dest != 0 && old_EXMEM.dest == old_IDEX.src1_reg){      // if EX Hazard of source register 1
            old_IDEX.src1 = old_EXMEM.ALUresult;                                                    // forward the value of source register 1
        }
        else if(old_MEMWB.reg_write && old_MEMWB.dest != 0 && old_MEMWB.dest == old_IDEX.src1_reg){ // if MEM Hazard of source register 1
            old_IDEX.src1 = registers[old_MEMWB.dest];                                              // forward the value of source register 1
        }
        if(old_EXMEM.reg_write && old_EXMEM.dest != 0 && old_EXMEM.dest == old_IDEX.src2_reg){      // if EX Hazard of source register 2
            old_IDEX.src2 = old_EXMEM.ALUresult;                                                    // forward the value of source register 2
        }
        else if(old_MEMWB.reg_write && old_MEMWB.dest != 0 && old_MEMWB.dest == old_IDEX.src2_reg){ // if MEM Hazard of source register 1
            old_IDEX.src2 = registers[old_MEMWB.dest];                                              // forward the value of source register 2
        }                                                                                           //************* END OF FORWARDING UNIT
        new_EXMEM.src2 = old_IDEX.src2;                                 // carry the value of source register 2 to EX/MEM register

                                                                    // ALU operations
        if(old_IDEX.ALUop == 1){                                    // if ALUop = 1 ==> addition
            if(old_IDEX.ALUsrc){                                        // if ALUsrc = 1 ==> use immediate value as second operand
                new_EXMEM.ALUresult = old_IDEX.src1 + old_IDEX.imm;
                new_EXMEM.zero = false;
            }
            else{                                                       // if ALUsrc = 0 ==> use value of the source register 2 as second operand
                new_EXMEM.ALUresult = old_IDEX.src1 + old_IDEX.src2;
                new_EXMEM.zero = false;
            }
        }
        else if(old_IDEX.ALUop == 2){                               // if ALUop = 2 ==> subtraction
            new_EXMEM.ALUresult = old_IDEX.src1 - old_IDEX.src2;    // executes subtraction
            if(new_EXMEM.ALUresult == 0){
                new_EXMEM.zero = true;
            }
            else{
                new_EXMEM.zero = false;
            }
        }
        else if(old_IDEX.ALUop == 3){                               // if ALUop = 3 ==> bitwise and
            new_EXMEM.ALUresult = old_IDEX.src1 & old_IDEX.src2;
            new_EXMEM.zero = false;
        }
        else if(old_IDEX.ALUop == 4){                               // if ALUop = 4 ==> bitwise or
            new_EXMEM.ALUresult = old_IDEX.src1 | old_IDEX.src2;
            new_EXMEM.zero = false;
        }
        new_EXMEM.mem_write = old_IDEX.mem_write;                   // carry the MemWrite to EX/MEM register
        new_EXMEM.mem_read = old_IDEX.mem_read;                     // carry the MemRead to EX/MEM register
        new_EXMEM.branch = old_IDEX.branch;                         // carry the Branch to EX/MEM register
        new_EXMEM.reg_write = old_IDEX.reg_write;                   // carry the RegWrite to EX/MEM register
        new_EXMEM.mem_to_reg = old_IDEX.mem_to_reg;                 // carry the MemtoReg to EX/MEM register
        //******************************************// MEM
        new_MEMWB.dest = old_EXMEM.dest;                            // carry the destination register to  MEM/WB register
        new_MEMWB.ALUresult = old_EXMEM.ALUresult;                  // carry the result of the ALU to  MEM/WB register
        if(old_EXMEM.mem_write){                                        // if MemWrite = 1, write value of source register 2 to target memory location (sd)
            data_memory[old_EXMEM.ALUresult] = old_EXMEM.src2;
        }
        if(old_EXMEM.mem_read){                                         // if MemRead = 1, read value of target memory address into the mem_value (ld)
            new_MEMWB.mem_value = data_memory[old_EXMEM.ALUresult];
        }
                                                                    //*********** THE BRANCH PREDICTION UNIT (PREDICT AS NOT TAKEN)
        if(old_EXMEM.branch && old_EXMEM.zero){                     // if the prediction is wrong, flush 3 consecutive instructions and set PC to branch target
            PC = old_EXMEM.branch_target - 1;                       // update the PC to branch target (we used -1 because end of the loop it increments by 1)  
            new_EXMEM.mem_write = false;                            // *** set control values of EX/MEM register to zero
            new_EXMEM.mem_read = false;
            new_EXMEM.branch = false;
            new_EXMEM.reg_write = false;
            new_EXMEM.mem_to_reg = false;
            new_IDEX.ALUop = 0;                                     // *** set control values of ID/EX register to zero
            new_IDEX.ALUsrc = false;
            new_IDEX.mem_write = false;
            new_IDEX.mem_read = false;
            new_IDEX.branch = false;
            new_IDEX.reg_write = false;
            new_IDEX.mem_to_reg = false;
            new_IFID.inst[0] = "";                                  // *** set data of IF/ID to zero, so that next cycle all control values can be zero
            stall = stall + 3;                                      // increase number of stalls by 3
            string tmp;
            tmp = instructions.at(old_EXMEM.PC) + " --> 3 stalls";  // store the instruction that causes the stall
            stall_inst.push_back(tmp);
        }                                                       //*********** END OF THE BRANCH PREDICTION UNIT
        new_MEMWB.reg_write = old_EXMEM.reg_write;              // carry the value of RegWrite to MEM/WB register
        new_MEMWB.mem_to_reg = old_EXMEM.mem_to_reg;            // carry the value of MemtoReg to MEM/WB register
                                                                //***************** THE HAZARD DETECTION UNIT (LOAD USE HAZARD) *************************//   
        // if there is no branch hazard and there is a load/use hazard add one stall
        if(!(old_EXMEM.branch && old_EXMEM.zero) && old_IDEX.mem_read && (old_IDEX.dest == stoi(old_IFID.inst[1]) || (old_IFID.inst[2] != "" && old_IDEX.dest == stoi(old_IFID.inst[2])))){
            new_IDEX.ALUop = 0;                             // set control values of ID/EX register to zero
            new_IDEX.ALUsrc = false;
            new_IDEX.mem_write = false;
            new_IDEX.mem_read = false;
            new_IDEX.branch = false;
            new_IDEX.reg_write = false;
            new_IDEX.mem_to_reg = false;
            IFID_write = false;                             // in next cycle program must keep the same values of IF/ID register
            PC--;                                           // prevent Program Counter to be updated (we used -1 because end of the loop it increments by 1) 
            stall++;                                        // increase the number of stalls by 1
            string tmp;
            tmp = instructions.at(old_IDEX.PC) + "      AND     " + instructions.at(old_IDEX.PC+1) + " --> 1 stall";
            stall_inst.push_back(tmp);                      // stores the instructions that causes the stall
        }                                                       //***************** END OF THE HAZARD DETECTION UNIT (LOAD USE HAZARD) *************************// 
        PC++;           // update the PC
        cycle++;        // increment the number of cycles
    }

    // printing the output values
    cout << "CPI: " << (double)cycle/(double)inst_count << endl;
    cout << "Number of cycles: " << cycle << endl;
    cout << "Number of stalls: " << stall << endl;
    cout << "Instructions causing stalls: " << endl;
    if(stall_inst.size() == 0){
        cout << "None" << endl;
    }
    for(int i=0; i<stall_inst.size(); i++){
        cout << stall_inst.at(i) << endl;
    }

    return 0;

}