/*  CS241 Scanner

    Starter code for the CS 241 assembler (assignments 3 and 4).
    Code contained here may be included in submissions for CS241
    assignments at the University of Waterloo.

    ---------------------------------------------------------------

    To compile on a CSCF linux machine, use:

            g++ -g asm.cc -o asm

    To run:
            ./asm           < source.asm > program.mips
            valgrind ./asm  < source.asm > program.mips
*/

#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include <map>
            using namespace std;

//======================================================================
//========= Declarations for the scan() function =======================
//======================================================================

// Each token has one of the following kinds.

            enum Kind {
  ID,                 // Opcode or identifier (use of a label)
  INT,                // Decimal integer
  HEXINT,             // Hexadecimal integer
  REGISTER,           // Register number
  COMMA,              // Comma
  LPAREN,             // (
  RPAREN,             // )
  LABEL,              // Declaration of a label (with a colon)
  DOTWORD,            // .word directive
  WHITESPACE,         // Whitespace
  NUL                 // Bad/invalid token
};

// kindString(k) returns string a representation of kind k
// that is useful for error and debugging messages.
string kindString(Kind k);

// Each token is described by its kind and its lexeme.

struct Token {
  Kind      kind;
  string    lexeme;
  /* toInt() returns an integer representation of the token. For tokens
   * of kind INT (decimal integer constant) and HEXINT (hexadecimal integer
   * constant), returns the integer constant. For tokens of kind
   * REGISTER, returns the register number.
   */
   int       toInt();
 };

// scan() separates an input line into a vector of Tokens.
 vector<Token> scan(string input);

// =====================================================================
// The implementation of scan() and associated type definitions.
// If you just want to use the scanner, skip to the next ==== separator.

// States for the finite-state automaton that comprises the scanner.

 enum State {
  ST_NUL,
  ST_START,
  ST_DOLLAR,
  ST_MINUS,
  ST_REGISTER,
  ST_INT,
  ST_ID,
  ST_LABEL,
  ST_COMMA,
  ST_LPAREN,
  ST_RPAREN,
  ST_ZERO,
  ST_ZEROX,
  ST_HEXINT,
  ST_COMMENT,
  ST_DOT,
  ST_DOTW,
  ST_DOTWO,
  ST_DOTWOR,
  ST_DOTWORD,
  ST_WHITESPACE
};

// The *kind* of token (see previous enum declaration)
// represented by each state; states that don't represent
// a token have stateKinds == NUL.

Kind stateKinds[] = {
  NUL,            // ST_NUL
  NUL,            // ST_START
  NUL,            // ST_DOLLAR
  NUL,            // ST_MINUS
  REGISTER,       // ST_REGISTER
  INT,            // ST_INT
  ID,             // ST_ID
  LABEL,          // ST_LABEL
  COMMA,          // ST_COMMA
  LPAREN,         // ST_LPAREN
  RPAREN,         // ST_RPAREN
  INT,            // ST_ZERO
  NUL,            // ST_ZEROX
  HEXINT,         // ST_HEXINT
  WHITESPACE,     // ST_COMMENT
  NUL,            // ST_DOT
  NUL,            // ST_DOTW
  NUL,            // ST_DOTWO
  NUL,            // ST_DOTWOR
  DOTWORD,        // ST_DOTWORD
  WHITESPACE      // ST_WHITESPACE
};

State delta[ST_WHITESPACE+1][256];

#define whitespace "\t\n\r "
#define letters    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define digits     "0123456789"
#define hexDigits  "0123456789ABCDEFabcdef"
#define oneToNine  "123456789"

void setT(State from, string chars, State to) {
  for(int i = 0; i < chars.length(); i++ ) delta[from][chars[i]] = to;
}

void initT(){
  int i, j;

  // The default transition is ST_NUL (i.e., no transition
  // defined for this char).
  for ( i=0; i<=ST_WHITESPACE; i++ ) {
    for ( j=0; j<256; j++ ) {
      delta[i][j] = ST_NUL;
    }
  }
  // Non-null transitions of the finite state machine.
  // NB: in the third line below, letters digits are macros
  // that are replaced by string literals, which the compiler
  // will concatenate into a single string literal.
  setT( ST_START,      whitespace,     ST_WHITESPACE );
  setT( ST_WHITESPACE, whitespace,     ST_WHITESPACE );
  setT( ST_START,      letters,        ST_ID         );
  setT( ST_ID,         letters digits, ST_ID         );
  setT( ST_START,      oneToNine,      ST_INT        );
  setT( ST_INT,        digits,         ST_INT        );
  setT( ST_START,      "-",            ST_MINUS      );
  setT( ST_MINUS,      digits,       ST_INT        );
  setT( ST_START,      ",",            ST_COMMA      );
  setT( ST_START,      "(",            ST_LPAREN     );
    setT( ST_START,      ")",            ST_RPAREN     );
    setT( ST_START,      "$",            ST_DOLLAR     );
    setT( ST_DOLLAR,     digits,         ST_REGISTER   );
    setT( ST_REGISTER,   digits,         ST_REGISTER   );
    setT( ST_START,      "0",            ST_ZERO       );
    setT( ST_ZERO,       "x",            ST_ZEROX      );
    setT( ST_ZERO,       digits,       ST_INT        );
    setT( ST_ZEROX,      hexDigits,      ST_HEXINT     );
    setT( ST_HEXINT,     hexDigits,      ST_HEXINT     );
    setT( ST_ID,         ":",            ST_LABEL      );
    setT( ST_START,      ";",            ST_COMMENT    );
    setT( ST_START,      ".",            ST_DOT        );
    setT( ST_DOT,        "w",            ST_DOTW       );
    setT( ST_DOTW,       "o",            ST_DOTWO      );
    setT( ST_DOTWO,      "r",            ST_DOTWOR     );
    setT( ST_DOTWOR,     "d",            ST_DOTWORD    );

    for ( j=0; j<256; j++ ) delta[ST_COMMENT][j] = ST_COMMENT;
  }

static int initT_done = 0;

vector<Token> scan(string input){
  // Initialize the transition table when called for the first time.
  if(!initT_done) {
    initT();
    initT_done = 1;
  }

  vector<Token> ret;

  int i = 0;
  int startIndex = 0;
  State state = ST_START;

  if(input.length() > 0) {
    while(true) {
      State nextState = ST_NUL;
      if(i < input.length())
       nextState = delta[state][(unsigned char) input[i]];
     if(nextState == ST_NUL) {
	// no more transitions possible
       if(stateKinds[state] == NUL) {
         throw("ERROR in lexing after reading " + input.substr(0, i));
       }
       if(stateKinds[state] != WHITESPACE) {
         Token token;
         token.kind = stateKinds[state];
         token.lexeme = input.substr(startIndex, i-startIndex);
         ret.push_back(token);
       }
       startIndex = i;
       state = ST_START;
       if(i >= input.length()) break;
     } else {
       state = nextState;
       i++;
     }
   }
 }

 return ret;
}

int Token::toInt() {
  if(kind == INT) {
    long long l;
    sscanf( lexeme.c_str(), "%lld", &l );
    if (lexeme.substr(0,1) == "-") {
      if(l < -2147483648LL)
       throw("ERROR: constant out of range: "+lexeme);
   } else {
    unsigned long long ul = l;
    if(ul > 4294967295LL)
     throw("ERROR: constant out of range: "+lexeme);
 }
 return l;
} else if(kind == HEXINT) {
  long long l;
  sscanf( lexeme.c_str(), "%llx", &l );
  unsigned long long ul = l;
  if(ul > 0xffffffffLL)
    throw("ERROR: constant out of range: "+lexeme);
  return l;
} else if(kind == REGISTER) {
  long long l;
  sscanf( lexeme.c_str()+1, "%lld", &l );
  unsigned long long ul = l;
  if(ul > 31)
    throw("ERROR: constant out of range: "+lexeme);
  return l;
}
throw("ERROR: attempt to convert non-integer token "+lexeme+" to Int");
}

// kindString maps each kind to a string for use in error messages.

string kS[] = {
  "ID",           // ID
  "INT",          // INT
  "HEXINT",       // HEXINT
  "REGISTER",     // REGISTER
  "COMMA",        // COMMA
  "LPAREN",       // LPAREN
  "RPAREN",       // RPAREN
  "LABEL",        // LABEL
  "DOTWORD",      // DOTWORD
  "WHITESPACE",   // WHITESPACE
  "NUL"           // NUL
};

string kindString( Kind k ){
  if ( k < ID || k > NUL ) return "INVALID";
  return kS[k];
}
void outbyte(int i){
  putchar(i>>24);
  putchar(i>>16);
  putchar(i>>8);
  putchar(i);
}
//======================================================================
//======= A sample program demonstrating the use of the scanner. =======
//======================================================================

int main() {

  try {
    vector<string> srcLines;

    // Read the entire input file, storing each line as a
    // single string in the array srcLines.
    while(true) {
      string line;
      getline(cin, line);
      if(cin.fail()) break;
      srcLines.push_back(line);
    }

    // Tokenize each line, storing the results in tokLines.
    vector<vector<Token> > tokLines;

    for(int line = 0; line < srcLines.size(); line++) {
      tokLines.push_back(scan(srcLines[line]));
    }

    int lineNumber = 0;
    map<string, int> label_map;
    //fills the label_map
    for(int line=0; line < tokLines.size(); line++){
      for(int j=0; j < tokLines[line].size(); j++ ) {
        Token token = tokLines[line][j];
        if(token.kind == LABEL){
          string labelName = token.lexeme.substr(0, token.lexeme.size()-1);
          if(label_map.count(labelName)==0){
            label_map.insert(label_map.begin(),pair<string, int>(labelName,lineNumber));
          }else{
            cerr << "ERROR: There already exist a label witht the same name" <<endl;
          }
        }else if(token.kind == ID || token.kind == DOTWORD || token.kind == INT || token.kind == HEXINT){ 
         lineNumber+=4;
         break;
       }
       else
         break;
     }
   }
   lineNumber = 0;
   for(int line=0; line < tokLines.size(); line++){
    for(int j=0; j < tokLines[line].size(); j++) {
     Token token = tokLines[line][j];
     if(token.kind == LABEL){
	     continue; //label is already entered into the label_map
	   }
     else if(token.kind == DOTWORD && tokLines[line].size()==j+2 && (tokLines[line][j+1].kind == INT || tokLines[line][j+1].kind == HEXINT || tokLines[line][j+1].kind==ID)){
      lineNumber+=4;
       if(tokLines[line][j+1].kind == INT || tokLines[line][j+1].kind == HEXINT){
         outbyte(tokLines[line][j+1].toInt());
       }else if(tokLines[line][j+1].kind == ID){
         string labelUse = tokLines[line][j+1].lexeme;
         if(label_map.count(labelUse)>0){
          std::map<string, int>::iterator it;
          it=label_map.find(labelUse);
          outbyte(it->second);
        }else{
          cerr <<"ERROR"<<endl;
          break;
        }
      }
      break;
    }
    else if(token.kind == ID && token.lexeme == "jr" && tokLines[line].size()==j+2 && tokLines[line][j+1].kind == REGISTER){
     int jrTemplate = 0x00000008;
     int s = tokLines[line][j+1].toInt();
     int i = jrTemplate|(s<<21);
     outbyte(i);
     lineNumber+=4;
     break;
   }
   else if(token.kind == ID && token.lexeme == "jalr" && tokLines[line].size()==j+2 && tokLines[line][j+1].kind == REGISTER){
     int jalrTemplate = 0x00000009;
     int s = tokLines[line][j+1].toInt();
     int i = jalrTemplate|(s<<21);
     outbyte(i);
     lineNumber+=4;
     break;
   }
   else if(token.kind == ID && (token.lexeme == "lis"||token.lexeme == "mflo"||token.lexeme == "mfhi") && tokLines[line].size()==j+2 && tokLines[line][j+1].kind == REGISTER){
     int jalrTemplate;
     if(token.lexeme=="lis"){
      jalrTemplate = 0x00000014;
     }else if(token.lexeme=="mflo"){
      jalrTemplate = 0x00000012;
     }else{
      jalrTemplate = 0x00000010;
     }
     int d = tokLines[line][j+1].toInt();
     int i = jalrTemplate|(d<<11);
     outbyte(i);
     lineNumber+=4;
     break;
   }
   else if(token.kind == ID && (token.lexeme == "add" ||token.lexeme == "sub"||token.lexeme == "slt" || token.lexeme == "sltu") && tokLines[line].size()==j+6){
     int rtemplate;
     int d;
     int s;
     int t;
     for(int i = 2; i < 6 ; i +=2){
       if(tokLines[line][j+i].kind != COMMA){
         cerr <<"ERROR: must be seperated by comma" <<endl;
         return 0;
       }
     }
     if(tokLines[line][j+1].kind == REGISTER){
       d=tokLines[line][j+1].toInt();
     }else{
       cerr <<"ERROR: d must be register" <<endl;
       return 0;
     }
     if(tokLines[line][j+3].kind == REGISTER){
      s=tokLines[line][j+3].toInt();
    }else{
      cerr <<"ERROR: s must be register" <<endl;
      return 0;
    }
    if(tokLines[line][j+5].kind == REGISTER){
      t=tokLines[line][j+5].toInt();
    }else{
      cerr <<"ERROR: t must be register" <<endl;
      return 0;
    }
    if(token.lexeme == "add"){
      rtemplate = 0x00000020;
    }
    else if(token.lexeme == "sub"){
      rtemplate = 0x00000022;
    }
    else if(token.lexeme == "slt"){
      rtemplate = 0x0000002a;
    }else{
      rtemplate = 0x0000002b;
    }
    int i = rtemplate|s<<21|t<<16|d<<11;
    outbyte(i);
    lineNumber+=4;
    break;
  } 
  else if(token.kind == ID && (token.lexeme == "mult" ||token.lexeme == "multu"||token.lexeme == "div" || token.lexeme == "divu") && tokLines[line].size()==j+4){
     int rtemplate;
     int s;
     int t;
     if(tokLines[line][j+2].kind != COMMA){
       cerr <<"ERROR: must be seperated by comma" <<endl;
       return 0;
     }
     if(tokLines[line][j+1].kind == REGISTER){
       s=tokLines[line][j+1].toInt();
     }else{
       cerr <<"ERROR: d must be register" <<endl;
       return 0;
     }
     if(tokLines[line][j+3].kind == REGISTER){
      t=tokLines[line][j+3].toInt();
    }else{
      cerr <<"ERROR: s must be register" <<endl;
      return 0;
    }
    if(token.lexeme == "mult"){
      rtemplate = 0x00000018;
    }
    else if(token.lexeme == "multu"){
      rtemplate = 0x00000019;
    }
    else if(token.lexeme == "div"){
      rtemplate = 0x0000001a;
    }else{
      rtemplate = 0x0000001b;
    }
    int i = rtemplate|s<<21|t<<16;
    outbyte(i);
    lineNumber+=4;
    break;
  } 
  else if(token.kind == ID && (token.lexeme == "beq" ||token.lexeme == "bne") && tokLines[line].size()==j+6){
    int rtemplate;
    int s;
    int t;
    int I;
    for(int i = 2; i < 6 ; i +=2){
      if(tokLines[line][j+i].kind != COMMA){
        cerr <<"ERROR: must be seperated by comma" <<endl;
        return 0;
      }
    }
    if(tokLines[line][j+1].kind == REGISTER){
      s=tokLines[line][j+1].toInt();
    }else{
      cerr <<"ERROR: s must be register" <<endl;
      return 0;
    }
    if(tokLines[line][j+3].kind == REGISTER){
      t=tokLines[line][j+3].toInt();
    }else{
      cerr <<"ERROR: t must be register" <<endl;
      return 0;
    }
    if(tokLines[line][j+5].kind == INT || tokLines[line][j+5].kind == HEXINT|| tokLines[line][j+5].kind == ID){
      if(tokLines[line][j+5].kind == INT){
        I=tokLines[line][j+5].toInt();
        if(I>32767 || I < -32768){
          cerr <<"ERROR: branch offset out of range" <<endl;
          return 0;
        }
      }else if(tokLines[line][j+5].kind == HEXINT){
        unsigned int rangeTest= tokLines[line][j+5].toInt();
        if(rangeTest>0xffff){
          cerr <<"ERROR: branch offset out of range" <<endl;
          return 0;
        }
        I = tokLines[line][j+5].toInt();
      }else{
        string labelUse = tokLines[line][j+5].lexeme;
        if(label_map.count(labelUse)>0){
          std::map<string, int>::iterator it;
          it=label_map.find(labelUse);
          int labelAddress = it->second;
          I=(labelAddress - (lineNumber + 4))/4;
          if(I>32767 || I < -32768){
            cerr <<"ERROR: branch offset out of range" <<endl;
            return 0;
          }
        }else{
          cerr <<"ERROR: label not found" <<endl;
          return 0;
        }
      }
    }else{
      cerr <<"ERROR: t must be hexadecimal, integer, or a label" <<endl;
      return 0;
    }
    if(token.lexeme == "beq"){
      rtemplate = 0x10000000;
    } 
    else{
      rtemplate = 0x14000000;
    }

    int i = rtemplate|s<<21|t<<16|(I & 0xffff);
    outbyte(i);
    lineNumber+=4;
    break;
  }
  else if(token.kind == ID && (token.lexeme == "sw"||token.lexeme == "lw") 
	  && tokLines[line].size()==j+7
){
    int rtemplate;
    int s;
    int t;
    int I;
    if(tokLines[line][j+2].kind != COMMA){
      cerr <<"ERROR: must be seperated by comma" <<endl;
      return 0;
    }
    if(tokLines[line][j+4].kind != LPAREN){
      cerr <<"ERROR: must be seperated by parenthesis" <<endl;
      return 0;
    }
    if(tokLines[line][j+6].kind != RPAREN){
      cerr <<"ERROR: must be seperated by parenthesis" <<endl;
      return 0;
    }
    if(tokLines[line][j+1].kind == REGISTER){
      t=tokLines[line][j+1].toInt();
    }else{
      cerr <<"ERROR: t must be register" <<endl;
      return 0;
    }
    if(tokLines[line][j+5].kind == REGISTER){
      s=tokLines[line][j+5].toInt();
    }else{
      cerr <<"ERROR: s must be register" <<endl;
      return 0;
    }
    if(tokLines[line][j+3].kind == INT || tokLines[line][j+3].kind == HEXINT|| tokLines[line][j+3].kind == ID){
      if(tokLines[line][j+3].kind == INT){
        I=tokLines[line][j+3].toInt();
        if(I>32767 || I < -32768){
          cerr <<"ERROR: branch offset out of range" <<endl;
          return 0;
        }
      }else if(tokLines[line][j+3].kind == HEXINT){
        unsigned int rangeTest= tokLines[line][j+3].toInt();
        if(rangeTest>0xffff){
          cerr <<"ERROR: branch offset out of range" <<endl;
          return 0;
        }
        I = tokLines[line][j+3].toInt();
      }else{
        string labelUse = tokLines[line][j+3].lexeme;
        if(label_map.count(labelUse)>0){
          std::map<string, int>::iterator it;
          it=label_map.find(labelUse);
          int labelAddress = it->second;
          I=(labelAddress - (lineNumber + 4))/4;
          if(I>32767 || I < -32768){
            cerr <<"ERROR: branch offset out of range" <<endl;
            return 0;
          }
        }else{
          cerr <<"ERROR: label not found" <<endl;
          return 0;
        }
      }
    }
    if(token.lexeme == "lw"){
      rtemplate = 0x8c000000;
    }else{
      rtemplate = 0xac000000;
    }
    int i = rtemplate|s<<21|t<<16|(I & 0xffff);
    outbyte(i);
    lineNumber+=4;
    break;
  }
  else{
    cerr <<"ERROR: Invalid Command"<<token.lexeme <<endl;
   break;
 }
}
}
for(map<string,int>::const_iterator it = label_map.begin(); it != label_map.end();++it){
  cerr<< it->first <<" "<< it->second <<endl;
}
} catch(string msg) {
  cerr << msg << endl;
}

return 0;
}
