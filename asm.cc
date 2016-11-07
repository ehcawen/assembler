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
    
    std::map<std::string,int>symTab;
    // Nested vector representing lines of Tokens
    // Needs to be used here to cleanup in the case
    // of an exception
    int addr = 0;
    const int mult= 4;
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
        
        // Iterate over the lines of tokens and print them
        // to standard error
        vector<vector<Token*> >::iterator it;
        for(it = tokLines.begin(); it != tokLines.end(); ++it){
            vector<Token*>::iterator it2;
            if(it->size()<1){
                continue;
            }else{
                for(it2 = it->begin(); it2 != it->end(); ++it2){
                    if((*it2)->getKind() == ASM::DOTWORD){
                        ++addr;
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
                        }
                    }
                }
            }
        }
        
        addr=0;
        
        for(it = tokLines.begin(); it != tokLines.end(); ++it){
            
            vector<Token*>::iterator it2;
            it2 = it->begin();
            if(it->size()<1){
                continue;
            }else{
                int dotWord = 0;//0: no .word yet; 1 1 .word ; 2 .word num
                for(it2 = it->begin(); it2 != it->end(); ++it2){
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
                            std::string errMSG = "ERROR: Illegal opcode "+(*it2)->getLexeme();
                            throw std::string(errMSG);
                            
                        }else{
                            std::string s = (*it2)->getLexeme();
                            std::map<std::string,int>::iterator sym;
                            sym = symTab.find(s);
                            if(sym==symTab.end()){
                                std::string errMSG = "ERROR: No such label: "+(*it2)->getLexeme();
                                throw std::string(errMSG);
                            }else{
                                
                                int symAddr = sym->second;
                                putchar(symAddr>>24);
                                putchar(symAddr>>16);
                                putchar(symAddr>>8);
                                putchar(symAddr);
                                dotWord=2;
                            }
                            
                        }
                    }else if(((*it2)->getKind()== ASM::INT)||((*it2)->getKind()== ASM::HEXINT)){
                        std::string errMSG = "ERROR";
                        if(dotWord!=1)throw std::string(errMSG);
                        
                        int n =(*it2)->toInt();
                        putchar(n>>24);
                        putchar(n>>16);
                        putchar(n>>8);
                        putchar(n);
                        dotWord=2;
                    }else if((*it2)->getKind() == ASM::DOTWORD){
                        dotWord = 1;
                        ++addr;
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

