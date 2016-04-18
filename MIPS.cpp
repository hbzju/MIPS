#define _CRT_SECURE_NO_DEPRECATE
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

void int_to_str_8(int instruction, char inst_str[]);
int Find_op(char* str, unsigned int& opc, unsigned int&opf, int &index);
int Find_reg_index(const string& code, int& i);
unsigned int Find_num(const string& code, int& i);
void R_type(int& index, unsigned int& instruction, const string& code, int i);
void I_type(int& index, unsigned int& instruction, const string& code, int i);
void J_type(int& index, unsigned int& instruction, const string& code, int i);
void Multi_type(int& index, unsigned int& instruction, const string& code, int i);

unsigned int inst_char_to_int(const string& code);
void Find_op_name(unsigned int instruction);
void Re_R_type(unsigned int& instruction,int index);
void Re_JorI_type(unsigned int& instruction,int index);
void Add_reg(int reg_index);
void Add_num(int num);
void Add_Branch(int branch);

unsigned int instruction_Ast;
//辅助寄存器用来处理li伪指令
char re_instruction[30];
char *reg_name[32] = {
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0",
    "t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2",
    "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp",
    "sp", "fp", "ra"
};
char *op_name[59] = {
    "lw", "lb", "lbu", "lh", "lhu", "sw", "sb", "sh", "add", "addu",
    "sub", "subu", "slt", "sltu", "and", "or", "xor", "nor", "sll", "srl",
    "sra", "mult", "multu", "div", "divu", "addi", "addiu", "andi", "ori", "xori",
    "lui", "slti", "sltiu", "beq", "bne", "blez", "bgtz", "bltz", "bgez", "j",
    "jal", "jalr", "jr", "mfhi", "mflo", "mthi", "mtlo", "eret", "mfco", "mtco",
    "break", "syscall", "la", "move", "li"
};
static unsigned int opvalue[52] = {
    0x23, 0x20, 0x24, 0x21, 0x25, 0x2B, 0x28, 0x29, 0x20, 0x21,
    0x22, 0x23, 0x2A, 0x2B, 0x24, 0x25, 0x26, 0x27, 0x00, 0x02,
    0x03, 0x18, 0x19, 0x01, 0x1B, 0x08, 0x09, 0x0C, 0x0D, 0x0E,
    0x0F, 0x0A, 0x0B, 0x04, 0x05, 0x06, 0x07, 0xFF, 0xFF, 0x02,
    0x03, 0x09, 0x08, 0x10, 0x12, 0x11, 0x13, 0x18, 0x10, 0x10,
    0x0D, 0x0C
};

void int_to_str_8(int instruction, char inst_str[])
{
    sprintf(inst_str, "%8.x", instruction);
    for (int i = 0; i < 9; i++){
        if (inst_str[i] == ' ')inst_str[i] = '0';
    }
}
unsigned int Assemble(const string& code)
{
    int i = 0, j = 0, index;
    char str[7];
    unsigned int opcode, opfunc;
    unsigned int instruction = 0;
    
    while (code[i] == '\t' || code[i] == ' ')i++;
    while (code[i] != ' '){
        str[j++] = code[i++];
    }
    str[j] = NULL;
    int type = Find_op(str, opcode, opfunc, index);
    if(type==0){//li instruction
        instruction |= opcode << 26;
        instruction_Ast |= opvalue[28];
        Multi_type(index, instruction, code, i);
    }
    else if (type == 1){//R type
        instruction |= opfunc;//增加opfuction，opcode为0，不用改变
        R_type(index, instruction, code, i);
    }
    else if (type == 2){
        instruction |= opcode << 26;
        I_type(index, instruction, code, i);
    }
    else{
        instruction |= opcode << 26;
        instruction_Ast |= opvalue[28];
        J_type(index, instruction, code, i);
    }
    return instruction;
}
int Find_reg_index(const string& code, int& i)
{
    char str[7];
    int j = 0;
    while (code[i] != '$'&&code[i] != ';'&&code[i] != '\n'&&code[i] != ')')i++;
    if (code[i] == ';' || code[i] == '\n' || code[i] == ')')return -1;
    else{
        i++;
        while ((code[i] >= '0'&&code[i] <= '9') || (code[i] >= 'a'&&code[i] <= 'z')){//寄存器名称只有数字和小写字母
            str[j++] = code[i++];
        }
    }
    str[j] = NULL;
    for (j = 0; j<32; j++){
        if (strcmp(str, reg_name[j]) == 0)return j;
    }
    return -1;
}
unsigned int Find_num(const string& code, int& i)
{
    unsigned int num = 0;
    while ((code[i] <'0' || code[i] >'9') && code[i] != ';'&&code[i] != '\n')i++;
    if (code[i] == ';' || code[i] == '\n')return -1;
    else{
        while (code[i] >= '0' && code[i] <= '9'){
            num = num * 10 + (code[i] - '0');
            i++;
        }
    }
    return num;
}
int Find_op(char str[], unsigned int& opc, unsigned int& opf, int &index)
{
    int i;
    for (i = 0; i<55; i++){
        if (strcmp(str, op_name[i]) == 0){
            index = i;
            break;
        }
    }
    if(index==54){
        opc = opvalue[30];
        return 0;//li type
    }
    else if(index==53){//move
        opc = 0;
        opf = 0x20;
        return 1;
    }
    else if(index==52){
        opc = 0x8;
        return 2;
        //la = addi $rd $zero 0
    }
    else if ((index >= 8 && index <= 24) || (index >= 41 && index <= 46) || index == 50 || index == 51){
        opc = 0;
        opf = opvalue[index];
        return 1;//R type
    }
    else if (index == 38 || index == 37){
        opc = 1;
        return 2;//I type
        //bltz,bgez
    }
    else if (index >= 47 && index <= 49){
        opc = 0x10;
        opf = opvalue[index];
        return 1;//R type
    }
    else {
        opc = opvalue[index];
        opf = -1;
        if (index == 39 || index == 40)return 3;//J type
        else return 2;//I type
    }
}

void R_type(int& index, unsigned int& instruction, const string& code, int i)
{
    if (index >= 8 && index <= 17){
        instruction |= Find_reg_index(code, ++i) << 11;
        instruction |= Find_reg_index(code, ++i) << 21;
        instruction |= Find_reg_index(code, ++i) << 16;
    }
    else if (index >= 18 && index <= 20){
        instruction |= Find_reg_index(code, i) << 11;
        instruction |= Find_reg_index(code, i) << 16;
        instruction |= Find_num(code, i) << 6;
    }
    else if (index >= 21 && index <= 24){
        instruction |= Find_reg_index(code, i) << 21;
        instruction |= Find_reg_index(code, i) << 16;
    }
    else if (index == 41 || index == 53 ){//jalr & move
        instruction |= Find_reg_index(code, i) << 11;
        instruction |= Find_reg_index(code, i) << 21;
    }
    else if (index == 42 || index == 45 || index == 46){//First: jr
        instruction |= Find_reg_index(code, i) << 21;
    }
    else if (index == 43 || index == 44){
        instruction |= Find_reg_index(code, i) << 11;
    }
    else if (index == 50){
        instruction |= Find_num(code, i)<<6;
    }
    else if (index == 51)return;
}
void J_type(int& index, unsigned int& instruction, const string& code, int i)
{
    instruction |= Find_num(code, i);
}
//28 I-type instructions
void I_type(int& index, unsigned int& instruction, const string& code, int i)
{
    if (index <= 7){//load and sw
        instruction |= Find_reg_index(code, i) << 16;
        instruction |= Find_num(code, i);
        //Find the number
        instruction |= Find_reg_index(code, i) << 21;
        //Find the source regs,')' is added to the fuction as a stop symbol,so such as 8($t1) can be found.
    }
    else if(index==30||index==52){//lui and la
        instruction |= Find_reg_index(code, i) << 16;
        instruction |= Find_num(code, i);
    }
    else if(index>=25&&index<=32){//立即数运算和小于立即数比较
        instruction |= Find_reg_index(code, i) << 16;
        instruction |= Find_reg_index(code, i) << 21;
        instruction |= Find_num(code, i);
    }
    else if(index>=33&&index<=34){//bne和beq
        instruction |= Find_reg_index(code, i) << 21;
        instruction |= Find_reg_index(code, i) << 16;
        instruction |= Find_num(code, i);
    }
    else if(index>=35&&index<=37){
        instruction |= Find_reg_index(code, i) << 21;
        instruction |= Find_num(code, i);
    }
    else if(index==38){
        instruction |= Find_reg_index(code, i) << 21;
        instruction |= 1<<16;
        instruction |= Find_num(code, i);
    }
    else if(index==47){
        instruction |= 1<<25;
    }
    else if(index==48){
        instruction |= Find_reg_index(code, i) << 16;
        instruction |= Find_reg_index(code, i) << 11;
    }
    else if(index==49){
        instruction |= Find_reg_index(code, i) << 16;
        instruction |= Find_reg_index(code, i) << 11;
        instruction |= 4<<21;
    }
}
void Multi_type(int& index, unsigned int& instruction, const string& code, int i)
{
    int rd = Find_reg_index(code, i);
    unsigned int num = Find_num(code, i);
    instruction |= num>>16;
    instruction |= 1<<16;
    instruction_Ast |= (num<<16)>>16;
    instruction_Ast |= 1<<21;
    instruction_Ast |= rd<<16;
}

void ReAssemble(const string& code)
{
    unsigned int instruction=inst_char_to_int(code);
    Find_op_name(instruction);
}

//add $t1, $t2, $t1;
unsigned int inst_char_to_int(const string& code)
{
    int i=0;
    unsigned int instruction=0;
    while(!((code[i]<='9'&&code[i]>='0')||(code[i]<='F'&&code[i]>='A')||(code[i]<='f'&&code[i]>='a')))i++;
    while(code[i]!=0&&code[i]!='\n'){
        if(code[i]<='9'&&code[i]>='0')instruction=instruction*16+code[i++]-'0';
        else if(code[i]<='F'&&code[i]>='A')instruction=instruction*16+code[i++]-'A'+10;
        else if(code[i]<='f'&&code[i]>='a')instruction=instruction*16+code[i++]-'a'+10;
    }
    return instruction;
}

void Find_op_name(unsigned int instruction)
{
    unsigned int op_value=instruction>>26;
    unsigned int opfunction;
    int space_index;
    int index,flag=0;
    if(op_value==0){//R type
        opfunction=(instruction<<26)>>26;
        //get instruction[0-5]
        for(index=8;index<=53&&flag==0;index++){
            switch(index){
                case 25:
                    index=40;
                    continue;
                case 46:
                    index=49;
                    continue;
                case 52:
                    continue;
                default:
                    if(opvalue[index]==opfunction){
                        strcpy(re_instruction,op_name[index]);
                        space_index=strlen(re_instruction);
                        flag=1;
                        break;
                    }
            }
        }
        Re_R_type(instruction,index);
    }
    else{
        for(index=0;index<=52&&flag==0;index++){
            switch(index){
                case 8:
                    index=24;
                    continue;
                case 41:
                    index=45;
                    continue;
                case 50:
                    index=51;
                    continue;
                default:
                    if(opvalue[index]==op_value){
                        strcpy(re_instruction,op_name[index]);
                        space_index=strlen(re_instruction);
                        flag=1;
                        break;
                    }
            }
        }
        Re_JorI_type(instruction,index);
    }
    re_instruction[space_index]=' ';
}

void Re_R_type(unsigned int& instruction,int index)
{
    int reg1,reg2,reg3,num1,num2;
    if (index >= 8 && index <= 17){
        reg1=(instruction<<16)>>27;//11-16
        reg2=(instruction<<6)>>27;//21-26
        reg3=(instruction<<11)>>27;//16-21
        Add_reg(reg1);
        Add_reg(reg2);
        Add_reg(reg3);
    }
    else if (index >= 18 && index <= 20){
        reg1=(instruction<<16)>>27;
        reg2=(instruction<<11)>>27;
        Add_reg(reg1);
        Add_reg(reg2);
        num1=(instruction<<21)>>27;//6-11
        Add_num(num1);
    }
    else if (index >= 21 && index <= 24){
        reg1=(instruction<<6)>>27;
        reg2=(instruction<<11)>>27;
        Add_reg(reg1);
        Add_reg(reg2);
    }
    else if (index == 41 || index == 53){//jalr
        reg1=(instruction<<16)>>27;
        reg2=(instruction<<6)>>27;
        Add_reg(reg1);
        Add_reg(reg2);
    }
    else if (index == 42 || index == 45 || index == 46){//First: jr
        reg1=(instruction<<6)>>27;
        Add_reg(reg1);
    }
    else if (index == 43 || index == 44){
        reg1=(instruction<<16)>>27;
        Add_reg(reg1);
    }
    else if (index == 50){//有问题，暂时这样写
        num1=(instruction<<6)>>12;
        Add_num(num1);
    }
    else if (index == 51);
    strcat(re_instruction,";");
}

void Re_JorI_type(unsigned int& instruction,int index)
{
    int reg1,reg2,reg3,num1,num2;
    if (index <= 7){//load and sw
        reg1=(instruction<<11)>>27;//16-21
        num1=(instruction<<16)>>16;
        Add_reg(reg1);
        Add_num(num1);
        strcat(re_instruction,"($");
        reg2=(instruction<<6)>>27;//21-26
        strcat(re_instruction,reg_name[reg2]);
        strcat(re_instruction,")");
        //Find the source regs,')' is added to the fuction as a stop symbol,so such as 8($t1) can be found.
    }
    else if(index==30||index==52){//lui and la
        reg1=(instruction<<11)>>27;
        num1=(instruction<<16)>>16;
        Add_reg(reg1);
        Add_num(num1);	
    }
    else if(index>=25&&index<=32){//立即数运算和小于立即数比较
        reg1=(instruction<<11)>>27;
        reg2=(instruction<<6)>>27;
        num1=(instruction<<16)>>16;
        Add_reg(reg1);
        Add_reg(reg2);
        Add_num(num1);
    }
    else if(index>=33&&index<=34){//bne和beq
        reg1=(instruction<<6)>>27;//21-26
        reg2=(instruction<<11)>>27;
        num1=(instruction<<16)>>16;
        Add_reg(reg1);
        Add_reg(reg2);
        Add_Branch(num1);
    }
    else if((index>=35&&index<=37)||index==38){
        reg1=(instruction<<6)>>27;//21-26
        num1=(instruction<<16)>>16;
        Add_reg(reg1);
        Add_Branch(num1);
    }
    else if(index==48||index==49){
        reg1=(instruction<<11)>>27;
        reg2=(instruction<<16)>>27;
        Add_reg(reg1);
        Add_reg(reg2);
    }
    else if(index==39||index==40){
        num1=(instruction<<6)>>6;
        Add_Branch(num1);
    }
    strcat(re_instruction,";");
}

int main()
{
    string code;
    unsigned int instruction;
    char inst_str[9];
    cout << "input code:";
    for (int i = 0; i < 10; i++){
        getline(cin, code);
        instruction = Assemble(code);
        int_to_str_8(instruction, inst_str);//8位
        cout <<"Assemble:"<< inst_str << endl;
        getline(cin, code);
        ReAssemble(code);//8位
        cout << re_instruction << endl;
    }
    system("pause");
    return 0;
}


void Add_Branch(int reg_index)
{
    char branch[10];
    strcpy(branch,"1010");
    //Check(table,branch);
    strcat(re_instruction,",");
    strcat(re_instruction,branch);
}

void Add_num(int num)
{
    char number[27];
    int i=0;
    while(num>0){
        number[i++]=num%10+'0';
        num/=10;
    }
    number[i]=0;
    strcat(re_instruction,",");
    strcat(re_instruction,number);
}

void Add_reg(int reg_index)
{
    strcat(re_instruction,",$");
    strcat(re_instruction,reg_name[reg_index]);
}