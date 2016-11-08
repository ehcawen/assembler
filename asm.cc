#include "kind.h"
#include "lexer.h"
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <map>
// Use only the neeeded aspects of each namespace
using std::string;
using std::vector;
using std::endl;
using std::cerr;
using std::cin;
using std::getline;
using ASM::Token;
using ASM::Lexer;


int main(int argc, char* argv[]){
    int addr = 3;
    int pc = 0xc;
    const int mult= 4;
    const int func = 26;
    const int s = 21;
    const int t = 16;
    const int d = 11;
    const std::string JR = "jr";
    const std::string JALR = "jalr";
    const std::string ADD = "add";
    const std::string SUB = "sub";
    const std::string SLT = "slt";
    const std::string SLTU = "sltu";
    const std::string BEQ = "beq";
    const std::string BNE = "bne";
    const std::string MFHI = "mfhi";
    const std::string MFLO = "mflo";
    const std::string LIS = "lis";
    const std::string MULT = "mult";
    const std::string MULTU = "multu";
    const std::string DIV = "div";
    const std::string DIVU = "divu";
    const std::string LW = "lw";
    const std::string SW = "sw";
    std::map<std::string,int>symTab;
    vector<int>rep;
    vector<int>relc;
    // Nested vector representing lines of Tokens
    // Needs to be used here to cleanup in the case
    // of an exception
    
    vector< vector<Token*> > tokLines;
    //vector<Symbol*> symTab;
    try{
        // Create a MIPS recognizer to tokenize
        // the input lines
        Lexer lexer;
        // Tokenize each line of the input
        string line;
        while(getline(cin,line)){
            tokLines.push_back(lexer.scan(line));
        }
        //HEADER COOKIE
        
        // Iterate over the lines of tokens and print them
        // to standard ERROR
        vector<vector<Token*> >::iterator it;
        for(it = tokLines.begin(); it != tokLines.end(); ++it){
            
            vector<Token*>::iterator it2;
            if(it->size()<1){
                continue;
            }else{
                for(it2 = it->begin(); it2 != it->end(); ++it2){
                    if(((*it2)->getKind() == ASM::DOTWORD)||((*it2)->getKind() == ASM::ID)){
                        ++addr;
                        break;
                    }else if((*it2)->getKind() == ASM::LABEL){
                        std::map<std::string,int>::iterator sym;
                        std::string lb = (*it2)->getLexeme();
                        lb = lb.substr(0,(*it2)->getLexeme().length()-1);
                        sym = symTab.find(lb);
                        if(sym!=symTab.end()){
                            std::string errMSG = "ERROR: Duplicate symbol:"+lb;
                            throw std::string(errMSG);
                        }else{
                            symTab.insert (std::pair<std::string,int>(lb,addr*mult));
                            //std::cout<<lb<<": "<<addr<<endl;
                        }
                    }
                }
            }
        }
        
        
        for(it = tokLines.begin(); it != tokLines.end(); ++it){
            int endLine = 0;
            vector<Token*>::iterator it2;
            it2 = it->begin();
            if(it->size()<1){
                continue;
            }else{
                if(((*it2)->getKind()!= ASM::ID)&&((*it2)->getKind()!= ASM::LABEL)&&((*it2)->getKind()!= ASM::DOTWORD)){
                    std::string errMSG = "ERROR: Expecting opcode, label, or directive, but got something else.";
                    
                    throw std::string(errMSG);
                }
                int dotWord = 0;//0: no .word yet; 1 1 .word ; 2 .word num
                for(it2 = it->begin(); it2 != it->end(); ++it2){
                    if(endLine){
                        std::string errMSG = "ERROR: Expecting end of line, but there's more stuff.";
                        throw std::string(errMSG);
                    }
                    if(dotWord==1){
                        if(!((((*it2)->getKind()== ASM::ID))||((*it2)->getKind()== ASM::INT)||((*it2)->getKind()== ASM::HEXINT))){
                            std::string errMSG = "ERROR: Need an integer after directive .word";
                            throw std::string(errMSG);
                        }
                    }else if (dotWord == 2){
                        std::string errMSG = "ERROR: Expecting end of line, but there's more stuff.";
                        throw std::string(errMSG);
                    }
                    
                    if(((*it2)->getKind()== ASM::ID)){
                        if(dotWord==0){
                            /////////////////////////////jrjalr///////
                            if(((*it2)->getLexeme() == JALR)||((*it2)->getLexeme() == JR)){
                                pc = pc + mult;
                                std::string inst = (*it2)->getLexeme();
                                ++it2;
                                if(it2!=it->end()){
                                    if((*it2)->getKind()== ASM::REGISTER){
                                        int reg = (*it2)->toInt();
                                        int n =0;
                                        if(inst==JR){
                                            int r = 0;
                                            int op = 8;
                                            r <<= func;
                                            reg <<= s;
                                            n = r+reg+op;
                                        }else if(inst==JALR){
                                            int r = 0;
                                            int op = 9;
                                            r <<= func;
                                            reg <<= s;
                                            n = r+reg+op;
                                        }
                                        ++it2;
                                        if(it2!=it->end()){
                                            std::string errMSG = "ERROR";
                                            throw std::string(errMSG);
                                        }
                                        rep.push_back(n);
                                        endLine=1;
                                        
                                        break;
                                    }else{
                                        std::string errMSG = "ERROR: Expecting register but got something else.";
                                        throw std::string(errMSG);
                                    }
                                }else{
                                    std::string errMSG = "ERROR: Expecting register but got end of line.";
                                    throw std::string(errMSG);
                                }
                            }
                            //////////////////////////////ADD,SUB,SLT,SLTU
                            else if(((*it2)->getLexeme() == ADD)||((*it2)->getLexeme() == SUB)||((*it2)->getLexeme() == SLT)||((*it2)->getLexeme() == SLTU)){
                                pc = pc + mult;
                                std::string inst = (*it2)->getLexeme();
                                int i = 0;
                                int regD = 0;
                                int regS = 0;
                                int regT = 0;
                                for(;it2 != it->end(); ++it2){
                                    if((i==0)&&((*it2)->getKind() == ASM::ID)&&(((*it2)->getLexeme() == ADD)||((*it2)->getLexeme() == SUB)||((*it2)->getLexeme() == SLT)||((*it2)->getLexeme() == SLTU))){
                                        ++i;
                                        continue;
                                    }
                                    if((*it2)->getKind() == ASM::WHITESPACE){
                                        continue;
                                    }
                                    
                                    if((i==1)||(i==3)||(i==5)){
                                        if(((*it2)->getKind()!= ASM::REGISTER)){
                                            std::string errMSG = "ERROR: Expecting register but got something else.";
                                            throw std::string(errMSG);
                                        }
                                        if(i==1){
                                            regD =(*it2)->toInt();
                                        }else if(i==3){
                                            regS =(*it2)->toInt();
                                        }else{
                                            regT =(*it2)->toInt();
                                        }
                                        
                                    }else if((i==2)||(i==4)){
                                        if(((*it2)->getKind()!= ASM::COMMA)){
                                            std::string errMSG = "ERROR: Expecting a COMMA but got something else.";
                                            throw std::string(errMSG);
                                        }
                                    }else {
                                        //cerr << "  Token: "  << *(*it2) << endl;
                                        std::string errMSG = "ERROR: Expecting end of line but got something else.";
                                        throw std::string(errMSG);
                                    }
                                    ++i;
                                }
                                if(i<5){
                                    std::string errMSG = "ERROR: Expecting a register or a comma but got end of line.";
                                    throw std::string(errMSG);
                                }
                                int n =0;
                                int op = 0;
                                if(inst==ADD){
                                    op = 32;
                                }else if(inst==SUB){
                                    op = 34;
                                }else if(inst==SLT){
                                    op = 42;
                                }else{
                                    op = 43;
                                }
                                int r = 0;
                                
                                r <<= func;
                                regS <<= s;
                                regT <<=t;
                                regD<<=d;
                                n = r+regS+regD+regT+op;
                                rep.push_back(n);
                                endLine=1;
                                break;
                            }
                            //////////////////////////////////beq,bne
                            else if(((*it2)->getLexeme() == BEQ)||((*it2)->getLexeme() == BNE)){
                                pc = pc + mult;
                                bool hex = false;
                                std::string inst = (*it2)->getLexeme();
                                int i = 0;
                                int regD = 0;
                                int regS = 0;
                                int regT = 0;
                                int jump = 0;
                                for(;it2 != it->end(); ++it2){
                                    if((i==0)&&((*it2)->getKind() == ASM::ID)&&(((*it2)->getLexeme() == BEQ)||((*it2)->getLexeme() == BNE))){
                                        ++i;
                                        continue;
                                    }
                                    if((*it2)->getKind() == ASM::WHITESPACE){
                                        continue;
                                    }
                                    
                                    if((i==1)||(i==3)){
                                        if(((*it2)->getKind()!= ASM::REGISTER)){
                                            std::string errMSG = "ERROR: Expecting register but got something else.";
                                            throw std::string(errMSG);
                                        }
                                        if(i==1){
                                            regS =(*it2)->toInt();
                                        }else if(i==3){
                                            regT =(*it2)->toInt();
                                        }
                                        
                                    }else if((i==2)||(i==4)){
                                        if(((*it2)->getKind()!= ASM::COMMA)){
                                            std::string errMSG = "ERROR: Expecting a COMMA but got something else.";
                                            throw std::string(errMSG);
                                        }
                                    }else if(i==5){
                                        if(((*it2)->getKind()== ASM::INT)||((*it2)->getKind()== ASM::HEXINT)){
                                            //jump i
                                            if((*it2)->getKind()== ASM::HEXINT)hex=true;
                                            jump =(*it2)->toInt();
                                            
                                        }else if((*it2)->getKind()== ASM::ID){
                                            //jump label
                                            std::map<std::string,int>::iterator sym;
                                            std::string lb = (*it2)->getLexeme();
                                            sym = symTab.find(lb);
                                            if(sym==symTab.end()){
                                                std::string errMSG = "ERROR: No such label:"+lb;
                                                throw std::string(errMSG);
                                            }
                                            jump = (sym->second-pc)/mult;
                                        }else{
                                            std::string errMSG = "ERROR: Expecting a number or a label but got something else.";
                                            throw std::string(errMSG);
                                        }
                                    }else {
                                        std::string errMSG = "ERROR: Expecting end of line but got something else.";
                                        throw std::string(errMSG);
                                    }
                                    ++i;
                                }
                                if(i<5){
                                    std::string errMSG = "ERROR: Expecting a register or a comma or a number but got end of line.";
                                    throw std::string(errMSG);
                                }
                                int n = 0;
                                int op =0;
                                int r = 0;
                                if(inst==BEQ){
                                    r = 4;
                                }else{
                                    r = 5;
                                }
                                r <<=func;
                                regS <<= s;
                                regT <<= t;
                                op = jump;
                                if(!hex){
                                    if(op>32767){
                                        std::string errMSG = "ERROR: OVERFLOW";
                                        throw std::string(errMSG);
                                    }else if(op<-32768){
                                        std::string errMSG = "ERROR: OVERFLOW";
                                        throw std::string(errMSG);
                                    }else if (op<0){
                                        op = 65536+op;
                                    }
                                }else{
                                    if(op>0xffff){
                                        std::string errMSG = "ERROR: OVERFLOW";
                                        throw std::string(errMSG);
                                    }else if(op<0){
                                        std::string errMSG = "ERROR: OVERFLOW";
                                        throw std::string(errMSG);
                                    }
                                }
                                
                                n = r+regS+regT+op;
                                rep.push_back(n);
                                endLine=1;
                                break;
                            }
                            ///////mfhi,mflo,lis
                            else if (((*it2)->getLexeme() == MFHI)||((*it2)->getLexeme() == MFLO)||((*it2)->getLexeme() == LIS)){
                                pc = pc + mult;
                                std::string inst = (*it2)->getLexeme();
                                ++it2;
                                if(it2!=it->end()){
                                    if((*it2)->getKind()== ASM::REGISTER){
                                        int reg = (*it2)->toInt();
                                        int n =0;
                                        int op = 0;
                                        if(inst==MFHI){
                                            op = 16;
                                        }else if(inst==MFLO){
                                            op = 18;
                                        }else{
                                            op = 20;
                                        }
                                        ++it2;
                                        if(it2!=it->end()){
                                            std::string errMSG = "ERROR";
                                            throw std::string(errMSG);
                                        }
                                        reg <<= d;
                                        n = reg+op;
                                        rep.push_back(n);
                                        endLine=1;
                                        
                                        break;
                                    }else{
                                        std::string errMSG = "ERROR: Expecting register but got something else.";
                                        throw std::string(errMSG);
                                    }
                                }else{
                                    std::string errMSG = "ERROR: Expecting register but got end of line.";
                                    throw std::string(errMSG);
                                }
                            }
                            //mult,multu,div,divu
                            else if(((*it2)->getLexeme() == MULT)||((*it2)->getLexeme() == MULTU)||((*it2)->getLexeme() == DIV)||((*it2)->getLexeme() == DIVU)){
                                pc = pc + mult;
                                std::string inst = (*it2)->getLexeme();
                                int i = 0;
                                int regD = 0;
                                int regS = 0;
                                int regT = 0;
                                for(;it2 != it->end(); ++it2){
                                    if((i==0)&&((*it2)->getKind() == ASM::ID)&&(((*it2)->getLexeme() == MULT)||((*it2)->getLexeme() == MULTU)||((*it2)->getLexeme() == DIV)||((*it2)->getLexeme() == DIVU))){
                                        ++i;
                                        continue;
                                    }
                                    if((*it2)->getKind() == ASM::WHITESPACE){
                                        continue;
                                    }
                                    
                                    if((i==1)||(i==3)){
                                        if(((*it2)->getKind()!= ASM::REGISTER)){
                                            std::string errMSG = "ERROR: Expecting register but got something else.";
                                            throw std::string(errMSG);
                                        }
                                        if(i==1){
                                            regS =(*it2)->toInt();
                                        }else if(i==3){
                                            regT =(*it2)->toInt();
                                        }
                                    }else if(i==2){
                                        if(((*it2)->getKind()!= ASM::COMMA)){
                                            std::string errMSG = "ERROR: Expecting a COMMA but got something else.";
                                            throw std::string(errMSG);
                                        }
                                    }else {
                                        std::string errMSG = "ERROR: Expecting end of line but got something else.";
                                        throw std::string(errMSG);
                                    }
                                    ++i;
                                }
                                if(i<3){
                                    std::string errMSG = "ERROR: Expecting a register or a comma but got end of line.";
                                    throw std::string(errMSG);
                                }
                                int n =0;
                                int op = 0;
                                if(inst==MULT){
                                    op = 24;
                                }else if(inst==MULTU){
                                    op = 25;
                                }else if(inst==DIV){
                                    op = 26;
                                }else{
                                    op = 27;
                                }
                                int r = 0;
                                
                                r <<= func;
                                regS <<= s;
                                regT <<=t;
                                regD<<=d;
                                n = r+regS+regD+regT+op;
                                rep.push_back(n);
                                endLine=1;
                                break;
                            }
                            //////lw,sw
                            else if(((*it2)->getLexeme() == LW)||((*it2)->getLexeme() == SW)){
                                pc = pc + mult;
                                bool hex = 0;
                                int jump = 0;
                                std::string inst = (*it2)->getLexeme();
                                int i = 0;
                                int regD = 0;
                                int regS = 0;
                                int regT = 0;
                                for(;it2 != it->end(); ++it2){
                                    if((i==0)&&((*it2)->getKind() == ASM::ID)&&(((*it2)->getLexeme() == LW)||((*it2)->getLexeme() == SW))){
                                        ++i;
                                        continue;
                                    }
                                    if((*it2)->getKind() == ASM::WHITESPACE){
                                        continue;
                                    }
                                    
                                    if((i==1)||(i==5)){
                                        if(((*it2)->getKind()!= ASM::REGISTER)){
                                            std::string errMSG = "ERROR: Expecting register but got something else.";
                                            throw std::string(errMSG);
                                        }
                                        if(i==1){
                                            regT =(*it2)->toInt();
                                        }else if(i==5){
                                            regS =(*it2)->toInt();
                                        }
                                    }else if(i==2){
                                        if(((*it2)->getKind()!= ASM::COMMA)){
                                            std::string errMSG = "ERROR: Expecting a COMMA but got something else.";
                                            throw std::string(errMSG);
                                        }
                                    }else if(i==3){
                                        //imm
                                        if(((*it2)->getKind()== ASM::INT)||((*it2)->getKind()== ASM::HEXINT)){
                                            //jump i
                                            if((*it2)->getKind()== ASM::HEXINT)hex=true;
                                            jump =(*it2)->toInt();
                                            
                                        }else{
                                            std::string errMSG = "ERROR: Expecting a number but got something else.";
                                            throw std::string(errMSG);
                                        }
                                    }else if(i==4){
                                        //leftP
                                        if(((*it2)->getKind()!= ASM::LPAREN)){
                                            std::string errMSG = "ERROR: Expecting a ( but got something else.";
                                            throw std::string(errMSG);
                                        }
                                    }else if(i==6){
                                        //rightP
                                        if(((*it2)->getKind()!= ASM::RPAREN)){
                                            std::string errMSG = "ERROR: Expecting a ) but got something else.";
                                            throw std::string(errMSG);
                                        }
                                    }else {
                                        std::string errMSG = "ERROR: Expecting end of line but got something else.";
                                        throw std::string(errMSG);
                                    }
                                    ++i;
                                }
                                if(i<6){
                                    std::string errMSG = "ERROR: Expecting a register or a comma but got end of line.";
                                    throw std::string(errMSG);
                                }
                                int n =0;
                                int op = 0;
                                int r = 0;
                                if(inst==LW){
                                    r=35;
                                }else{
                                    r=43;
                                }
                                op=jump;
                                if(!hex){
                                    if(op>32767){
                                        std::string errMSG = "ERROR: OVERFLOW";
                                        throw std::string(errMSG);
                                    }else if(op<-32768){
                                        std::string errMSG = "ERROR: OVERFLOW";
                                        throw std::string(errMSG);
                                    }else if (op<0){
                                        op = 65536+op;
                                    }
                                }else{
                                    if(op>0xffff){
                                        std::string errMSG = "ERROR: OVERFLOW";
                                        throw std::string(errMSG);
                                    }else if(op<0){
                                        std::string errMSG = "ERROR: OVERFLOW";
                                        throw std::string(errMSG);
                                    }
                                }
                                r <<= func;
                                regS <<= s;
                                regT <<=t;
                                regD <<=d;
                                n = r+regS+regD+regT+op;
                                rep.push_back(n);
                                endLine=1;
                                break;
                            }
                            else{
                                std::string errMSG = "ERROR: Illegal opcode "+(*it2)->getLexeme();
                                throw std::string(errMSG);
                            }
                            
                            
                        }else{
                            std::string s = (*it2)->getLexeme();
                            std::map<std::string,int>::iterator sym;
                            sym = symTab.find(s);
                            if(sym==symTab.end()){
                                std::string errMSG = "ERROR: No such label: "+(*it2)->getLexeme();
                                throw std::string(errMSG);
                            }else{
                                int symAddr = sym->second;
                                rep.push_back(symAddr);
                                relc.push_back(1);
                                relc.push_back(pc-mult);
                                dotWord=2;
                            }
                            
                        }
                    }else if(((*it2)->getKind()== ASM::INT)||((*it2)->getKind()== ASM::HEXINT)){
                        std::string errMSG = "ERROR";
                        if(dotWord!=1)throw std::string(errMSG);
                        
                        int n =(*it2)->toInt();
                        rep.push_back(n);
                        dotWord=2;
                    }else if((*it2)->getKind() == ASM::DOTWORD){
                        pc = pc + mult;
                        dotWord = 1;
                    }
                }
                if(dotWord==1){
                    
                    std::string errMSG = "ERROR: Need an integer after directive .word";
                    throw std::string(errMSG);
                }
                
            }
            /*for(it2 = it->begin(); it2 != it->end(); ++it2){
             cerr << "  Token: "  << *(*it2) << endl;
             }*/
        }
        //cookie
        putchar(0x10000002>>24);
        putchar(0x10000002>>16);
        putchar(0x10000002>>8);
        putchar(0x10000002);
        int total = pc + relc.size()*mult;
        //merl size
        putchar(total>>24);
        putchar(total>>16);
        putchar(total>>8);
        putchar(total);
        //header+mips
        putchar(pc>>24);
        putchar(pc>>16);
        putchar(pc>>8);
        putchar(pc);
        //code
        vector<int>::iterator itrp;
        for(itrp=rep.begin();itrp!=rep.end();++itrp){
            putchar(*itrp>>24);
            putchar(*itrp>>16);
            putchar(*itrp>>8);
            putchar(*itrp);
        }
        //reloc
        for(itrp=relc.begin();itrp!=relc.end();++itrp){
            putchar(*itrp>>24);
            putchar(*itrp>>16);
            putchar(*itrp>>8);
            putchar(*itrp);
        }
        
    } catch(const string& msg){
        // If an exception occurs print the message and end the program
        cerr << msg << endl;
    }
    // Delete the Tokens that have been made
    vector<vector<Token*> >::iterator it;
    for(it = tokLines.begin(); it != tokLines.end(); ++it){
        vector<Token*>::iterator it2;
        for(it2 = it->begin(); it2 != it->end(); ++it2){
            delete *it2;
        }
    }
}

