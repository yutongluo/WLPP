// Starter code for CS241 assignments 9-11
//
// C++ translation by Simon Parent (Winter 2011),
// based on Java code by Ondrej Lhotak,
// which was based on Scheme code by Gord Cormack.
// Modified July 3, 2012 by Gareth Davies
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

// The set of terminal symbols in the WLPP grammar.
const char *terminals[] = {
  "BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE", "GT", "ID",
  "IF", "INT", "LBRACE", "LE", "LPAREN", "LT", "MINUS", "NE", "NUM",
  "PCT", "PLUS", "PRINTLN", "RBRACE", "RETURN", "RPAREN", "SEMI",
  "SLASH", "STAR", "WAIN", "WHILE", "AMP", "LBRACK", "RBRACK", "NEW",
  "DELETE", "NULL"
};
int isTerminal(const string &sym) {
  int idx;
  for(idx=0; idx<sizeof(terminals)/sizeof(char*); idx++)
    if(terminals[idx] == sym) return 1;
  return 0;
}

// Data structure for storing the parse tree.
class tree {
public:
  string rule;
  vector<string> tokens;
  vector<tree*> children;
  ~tree() { for(int i=0; i<children.size(); i++) delete children[i]; }
};

// Call this to display an error message and exit the program.
void bail(const string &msg) {
  // You can also simply throw a string instead of using this function.
  throw string(msg);
}

// Read and return wlppi parse tree.
tree *readParse(const string &lhs) {
  // Read a line from standard input.
  string line;
  getline(cin, line);
  if(cin.fail())
    bail("ERROR: Unexpected end of file.");
  tree *ret = new tree();
  // Tokenize the line.
  stringstream ss;
  ss << line;
  while(!ss.eof()) {
    string token;
    ss >> token;
    if(token == "") continue;
    ret->tokens.push_back(token);
  }
  // Ensure that the rule is separated by single spaces.
  for(int idx=0; idx<ret->tokens.size(); idx++) {
    if(idx>0) ret->rule += " ";
    ret->rule += ret->tokens[idx];
  }
  // Recurse if lhs is a nonterminal.
  if(!isTerminal(lhs)) {
    for(int idx=1/*skip the lhs*/; idx<ret->tokens.size(); idx++) {
      ret->children.push_back(readParse(ret->tokens[idx]));
    }
  }
  return ret;
}

tree *parseTree;
vector<string> symbleTable;
// Compute symbols defined in t.


string findType(tree *t){
  string id,type;
  if(t->rule == "dcl type ID"){
    id = t->children[1]->tokens[1];
    for(int i = 0; i < symbleTable.size(); i++){
      if(id == symbleTable[i]){
	type = symbleTable[i+1];
	break;
      }
    }
  }
  else if(t->rule == "lvalue STAR factor"){
    if(findType(t->children[1])=="int*"){
      type = "int";
    }else
      bail("ERROR: "+ t->rule); 
  }
  else if(t->rule == "lvalue ID"){
    id = t->children[0]->tokens[1];
    for(int i = 0; i < symbleTable.size(); i++){
      if(id==symbleTable[i]){
	type=symbleTable[i+1];
	break;
      }
    }
  }
  else if(t->rule == "lvalue LPAREN lvalue RPAREN"){
    type = findType(t->children[1]);
  }
  else if(t->rule == "factor ID"){
    id = t->children[0]->tokens[1];
    for(int i = 0; i < symbleTable.size(); i++){
      if(id==symbleTable[i]){
	type = symbleTable[i+1];
	break;
      }
    }
  }
  else if(t->rule == "factor NUM"){
    type = "int";
  }
  else if(t->rule == "factor NULL"){
    type = "int*";
  }
  else if(t->rule == "factor LPAREN expr RPAREN"){
    type = findType(t->children[1]);
  }
  else if(t->rule == "factor AMP lvalue"){
    if(findType(t->children[1])=="int") type = "int*";
    else 
      bail("ERROR: wrong &lvalue type");
  }
  else if(t->rule == "factor STAR factor"){
    //cerr<<findType(t->children[1])<<endl;
    if(findType(t->children[1])=="int*") type = "int";
    else{
      //cerr<<"NOPE"<<endl;
      bail("ERROR: wrong *factor type");
    }
  }
  else if(t->rule == "factor NEW INT LBRACK expr RBRACK"){
    if(findType(t->children[3])=="int") type = "int*";
    else bail("ERROR: wrong new [expr] type");
  }
  else if(t->rule == "term factor"){
    type = findType(t->children[0]);
  }
  else if(t->rule == "term term STAR factor"){
    string temp1=findType(t->children[0]), temp2=findType(t->children[2]);
    if(temp1=="int"&&temp2=="int") type = "int";
    else bail("ERROR: wrong term * factor type");
  }
  else if(t->rule == "term term PCT factor"){
    string temp1=findType(t->children[0]), temp2=findType(t->children[2]);
    if(temp1=="int"&&temp2=="int") type = "int";
    else bail("ERROR: wrong term % factor type");
  }
  else if(t->rule == "term term SLASH factor"){
    string temp1=findType(t->children[0]), temp2=findType(t->children[2]);
    if(temp1=="int"&&temp2=="int") type = "int";
    else bail("ERROR: wrong term / factor type");
  }
  else if(t->rule == "expr expr PLUS term"){
    string temp1=findType(t->children[0]), temp2=findType(t->children[2]);
    if(temp1=="int"&&temp2=="int") type = "int";
    else if(temp1=="int*" && temp2=="int*") bail("ERROR: wrong expr PLUS term type");
    else{
      type = "int*";
      //cerr<<"AYYYYY"<<endl;
    }
  }
  else if(t->rule == "expr expr MINUS term"){
    string temp1=findType(t->children[0]), temp2=findType(t->children[2]);
    if(temp1==temp2) type = "int";
    else if(temp1=="int*" && temp2=="int") type = "int*";
    else bail("ERROR: wrong expr MINUS term type");
  }
  else if(t->rule == "expr term"){
    type = findType(t->children[0]);
  }
  return type;
}

bool testType(tree* t){
  if(t->rule == "test expr EQ expr" ||
    t->rule == "test expr NE expr" ||
    t->rule == "test expr LT expr" ||
    t->rule == "test expr LE expr" ||
    t->rule == "test expr GE expr" ||
      t->rule == "test expr GT expr" ){
      if (findType(t->children[0]) ==  findType(t->children[2])) 
	return true;
      else return false;
    }
}
void typeCheck(tree* t){
  if (t->rule =="procedure INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE"){
    if (findType(t->children[5])=="int*") bail("ERROR: second parameter must be integer");
    if (findType(t->children[11])!="int") bail("ERROR: return expression must be integer");
  }
  if (t->rule =="dcls dcls dcl BECOMES NUM SEMI"){
    if (findType (t->children[1])!= "int") bail("ERROR: wrong declaration type");
  }
  if (t->rule =="dcls dcls dcl BECOMES NUM SEMI"){
    if (findType (t->children[1])!= "int") bail("ERROR: wrong declaration type");
  }
  if ( t->rule =="statement lvalue BECOMES expr SEMI"){
    if (findType(t->children[0])!=findType(t->children[2])) bail("ERROR: wrong BECOMES type");
  }
  else if (t->rule =="statement PRINTLN LPAREN expr RPAREN SEMI"){
    if (findType(t->children[2])!="int") bail("ERROR: expr type wrong");
  }
  else if (t->rule =="statement DELETE LBRACK RBRACK expr SEMI"){
    if (findType(t->children[3]) != "int*") bail("ERROR: expr type wrong");
  }
  else if (t->rule =="expr term"){
    if (findType(t->children[0])!= "int") bail("ERROR: the return type has to be int");
  }
  else if (t->rule =="expr expr PLUS term"){
    if (findType(t->children[0]) !="int" || findType(t->children[2])!="int") 
      bail("ERROR: wrong type for PLUS");
  }
  else if (t->rule =="expr expr MINUS term"){
    if (findType(t->children[0]) != findType(t->children[2]))
      bail("ERROR: wrong type for minus");
  }
  else if (t->rule =="statement IF LPAREN test RPAREN LBRACE statements RBRACE" ||
	   t->rule =="statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE"){
    if (testType(t->children[2]) == false ) bail("ERROR: comparison of different types");
  }
  else if(t->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE"){
    if (testType(t->children[2]) == false ) bail("ERROR: comparison of different types");
  }
  else if(t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI"){
    if (findType(t->children[2])!="int") bail("ERROR: wrong type for minus");                        
  }
  else if(t->rule == "statement DELETE LBRACK RBRACK expr SEMI"){
    if (findType(t->children[3])!= "int*") bail("ERROR: Attempting to deallocate an integer address");                          
  }
  else {
    for (int i = 0 ; i < t->children.size();i++)
      typeCheck(t->children[i]);
  }
}
int addr = 1;
string NumberToString ( int Number )
{
  stringstream ss;
  ss << Number;
  return ss.str();
}
int StringToNumber ( const string &Text ) 
{                               
  stringstream ss(Text);
  int result;
  return ss >> result ? result : 0;
}
void genSymbols(tree *t) {
  vector<string>::iterator it;
  stringstream convert1;
  if(t->tokens[0]=="ID"){
    it = find(symbleTable.begin(), symbleTable.end(),t->tokens[1]);
    if(it == symbleTable.end())
      bail("ERROR: " + t->tokens[1] + " has not been declared");
  }
  if(t->rule == "dcl type ID"){
    string id = (t->children[1])->tokens[1];
    tree* type = t->children[0];
    it = find(symbleTable.begin(), symbleTable.end(), id);
    if(it != symbleTable.end()&&symbleTable.size()!=0)
      bail("ERROR: " + id + " has been declared before");
    symbleTable.push_back(id);
    if(type->tokens.size()==3){
      if(type->tokens[1]== "INT" && type->tokens[2]== "STAR"){
	cerr<<id<<" "<<"int*"<<endl;
	symbleTable.push_back("int*");
      }
    }
    else if(type->tokens[1]== "INT"){
      cerr<<id<<" "<<"int"<<endl;
      symbleTable.push_back("int");
    }else 
      bail("ERROR: no such type");
    symbleTable.push_back(NumberToString(addr));
    addr++;
  }

  if(isTerminal(t->tokens[0])==0){
    for(int i = 0; i< t->children.size(); i++){
      genSymbols(t->children[i]);
    }
  }
}

int findAddr(string name){
  string ret;
  for(int i = 0; i < symbleTable.size(); i++){
    if(name==symbleTable[i]){
      ret=symbleTable[i+2];
      break;
    }
  }
  return StringToNumber(ret);
}
void print(string str){
  cout<<str<<endl;
}
void lw(string reg1, int off, string reg2){
  print("lw $"+reg1 + ","+NumberToString(off)+"($"+reg2+")");
}
void sw(string reg1, int off, string reg2){
  print("sw $"+reg1 + ","+NumberToString(off)+"($"+reg2+")");
}
void push(string reg){
  print("sw $" + reg + ", -4($30)");
  print("sub $30,$30,$4"); 
}
void pop(string reg){
  print("add $30,$30,$4");
  print("lw $"+ reg + ",-4($30)");
}
void dclCode(tree *t, string num){
  string ret;
  if(t->children[0]->rule == "type INT"||t->children[0]->rule == "type INT STAR"){
    sw(num,findAddr(t->children[1]->tokens[1])*4,"29");
  }
}
void dclsCode(tree *t){
  if (t->rule =="dcls") return;
  else if(t->rule =="dcls dcls dcl BECOMES NUM SEMI"){
    dclsCode(t->children[0]);
    print("lis $1");
    print(".word "+t->children[3]->tokens[1]);
    sw("1",findAddr(t->children[1]->children[1]->tokens[1]) * 4,"29");
  }
}
void exprCode(tree *t);
int lValue(tree *t){
  if(t->rule == "lvalue ID"){
    return findAddr(t->children[0]->tokens[1])*4;
  }else if(t->rule =="lvalue LPAREN lvalue RPAREN"){
    return lValue(t->children[1]);
  }
}
int whileCounter = 0;
int ifCounter = 0;
void testCode(tree *t, string label);
void statementsCode(tree *t){
  string whileNum = NumberToString(whileCounter);
  string ifNum = NumberToString(ifCounter);
  if (t->rule =="statements") return;
  else if(t->rule =="statements statements statement"){
    statementsCode(t->children[0]);
    statementsCode(t->children[1]);
  }
  else if(t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI"){
    exprCode(t->children[2]);
    push("1");
    print("add $1,$3,$0");
    print("lis $28");
    print(".word print");
    print("jalr $28");
    pop("1");
  }
  else if (t->rule =="statement lvalue BECOMES expr SEMI"){
    exprCode(t->children[2]);
    sw("3",lValue(t->children[0]),"29");
  }
  else if(t->rule =="statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE"){
    ifCounter++;
    testCode(t->children[2],"else"+ifNum);
    statementsCode(t->children[5]);
    print("beq $0,$0,Ldone"+ifNum);
    print("Lelse"+ifNum+":");
    statementsCode(t->children[9]);
    print("Ldone"+ifNum+":");
  }
  else if(t->rule =="statement WHILE LPAREN test RPAREN LBRACE statements RBRACE"){
    whileCounter++;
    //    cerr<<"whileCounter:"<<whileCounter<<endl;
    print("Lwhile"+whileNum+":");
    testCode(t->children[2],"endwhile"+whileNum);
    statementsCode(t->children[5]);
    print("beq $0,$0,Lwhile"+whileNum);
    print("Lendwhile"+whileNum+":");
    }
}

void testCode(tree *t,string label){
  if(t->rule == "test expr LT expr"){
    exprCode(t->children[0]);
    push("3");
    exprCode(t->children[2]);
    pop("1");
    print("slt $1,$1,$3");
    print("beq $1,$0,L" + label);
  }
  else if (t->rule =="test expr GT expr"){
    exprCode(t->children[0]);
    push("3");
    exprCode(t->children[2]); 
    pop("1");
    print("slt $1,$3,$1"); 
    print("beq $1,$0,L" + label);
  }
  else if (t->rule =="test expr EQ expr"){
    exprCode(t->children[0]);
    push("3");
    exprCode(t->children[2]);
    pop("1");
    print("bne $1,$3,L" + label);
  }
  else if (t->rule =="test expr NE expr"){
    exprCode(t->children[0]);
    push("3");
    exprCode(t->children[2]);
    pop("1");
    print("beq $1,$3,L" + label);
  }
  else if (t->rule =="test expr LE expr"){
    exprCode(t->children[0]);
    push("3");
    exprCode(t->children[2]);
    pop("1");
    print("slt $1,$3,$1");
    print("bne $1,$0,L"+label);
  }
  else if (t->rule =="test expr GE expr"){
    exprCode(t->children[0]);
    push("3");
    exprCode(t->children[2]);
    pop("1");
    print("slt $1,$1,$3");
    print("bne $1,$0,L"+label);
  }
}

void factorCode(tree *t){
  if(t->rule == "factor ID"){
    lw("3",findAddr(t->children[0]->tokens[1])*4,"29");
  }
  else if(t->rule == "factor LPAREN expr RPAREN"){
    exprCode(t->children[1]);
  }
  else if(t->rule == "factor NUM"){
    print("lis $3");
    print(".word "+t->children[0]->tokens[1]);
  }
}
void termCode(tree *t){
  if(t->rule == "term factor"){
    factorCode(t->children[0]);
  }else if (t->rule == "term term STAR factor"){
    termCode(t->children[0]);
    push("3");
    factorCode(t->children[2]);
    pop("5");
    print("mult $3,$5");
    print("mflo $3");
  }
  else if(t->rule =="term term SLASH factor"){
    termCode(t->children[0]);
    push("3");
    factorCode(t->children[2]);
    pop("5");
    print("div $5,$3");
    print("mflo $3");  
  }
  else if (t->rule =="term term PCT factor"){
    termCode(t->children[0]);
    push("3");
    factorCode(t->children[2]);
    pop("5");
    print("div $5,$3");
    print("mfhi $3");   
  }
}
void exprCode(tree *t){
  if(t->rule == "expr term"){
    termCode(t->children[0]);
  }
  if(t->rule == "expr expr PLUS term"){
    if(findType(t->children[0])=="int" && findType(t->children[2])=="int"){
      exprCode(t->children[0]);
      push("3");
      termCode(t->children[2]);
      pop("5");
      print("add $3, $5, $3");
    }
    /*else if(findType(t->children[0])=="int" && findType(t->children[2])=="int*"){
      exprCode(t->children[0]);
      push("3");
      termCode(t->children[2]);
      pop("5");
      print("add $5, $5, $5");
      print("add $5, $5, $5");
      print("add $3, $5, $3");
    }else if(findType(t->children[0])=="int*" && findType(t->children[2])=="int"){
      exprCode(t->children[0]);
      push("3");
      termCode(t->children[2]);
      pop("5");
      print("add $3, $3, $3");
      print("add $3, $3, $3");
      print("add $3, $5, $3");
      }*/
  }
  if(t->rule == "expr expr MINUS term"){
    if(findType(t->children[0])=="int" && findType(t->children[2])=="int"){
      exprCode(t->children[0]);
      push("3");
      termCode(t->children[2]);
      pop("5");
      print("sub $3, $5, $3");
    }/*else if(findType(t->children[0])=="int*" && findType(t->children[2])=="int"){
      exprCode(t->children[0]);
      push("3");
      termCode(t->children[2]);
      pop("5");
      print("add $3, $3, $3");
      print("add $3, $3, $3");
      print("sub $3, $5, $3");
      }*/
  }
}

// Generate the code for the parse tree t.
void genCode(tree *t) {
  if(t->rule == "procedure INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE"){
    print("lis $4");
    print(".word 4");
    print("lis $11");
    print(".word 1");
    print("lis $5");
    print(".word "+NumberToString(symbleTable.size() * 4));
    print("sub $30, $30, $5");
    print("add $29, $30, $0");
    push("31");
    dclCode(t->children[3],"1");
    dclCode(t->children[5],"2");
    dclsCode(t->children[8]);
    statementsCode(t->children[9]);
    exprCode(t->children[11]);
    pop("31");
    print("jr $31");
  }
  else {
    for (int i=0 ; i < t->children.size(); i++){
      genCode(t->children[i]);
    }
  }
}

int main() {
  // Main program.
  try {
    parseTree = readParse("S");
    genSymbols(parseTree);
    typeCheck(parseTree);
    genCode(parseTree);
    cout<<"print:\nsw $1, -4($30)\nsw $2, -8($30)\nsw $3, -12($30)\nsw $4, -16($30)\nsw $5, -20($30)\nsw $6, -24($30)\nsw $7, -28($30)\nsw $8, -32($30)\nsw $9, -36($30)\nsw $10, -40($30)\nlis $3\n.word -40\nadd $30, $30, $3\nlis $3\n.word 0xffff000c\nlis $4\n.word 10\nlis $5\n.word 4\nadd $6, $1, $0\nslt $7, $1, $0\nbeq $7, $0, IfDone\nlis $8\n.word 0x0000002d\nsw $8, 0($3)\nsub $6, $0, $6\nIfDone:\nadd $9, $30, $0\nLoop:\ndivu $6, $4\nmfhi $10\nsw $10, -4($9)\nmflo $6\nsub $9, $9, $5          ; $9 = $9 - 4\nslt $10, $0, $6         ; $10 = 1 iff ($6 > 0) otherwise $10 = 0\nbne $10, $0, Loop       ; continue the loop until done\n; use second Loop to print the digits in the right order\nlis $7\n.word 48                ; load character 0 (ascii 48) into $7\nLoop2:\nlw $8, 0($9)            ; $8 = mem[$9]\nadd $8, $8, $7         ; calculate the ascii value of the digit\nsw $8, 0($3)            ; print the character in $8\nadd $9, $9, $5          ; $9 = $9 + 4\nbne $9, $30, Loop2      ; jump the loop\nsw $4, 0($3)            ; print character '\\n' (ascii 10)\n; restore saved registers\nlis $3\n.word 40\nadd $30, $30, $3        ; restore the stack pointer\nlw $1, -4($30)\nlw $2, -8($30)\nlw $3, -12($30)\nlw $4, -16($30)\nlw $5, -20($30)\nlw $6, -24($30)\nlw $7, -28($30)\nlw $8, -32($30)\nlw $9, -36($30)\nlw $10, -40($30)\njr $31"<<endl;
  } catch(string msg) {
    cerr << msg << endl;
  }
  if (parseTree) delete parseTree;
  return 0;
}
