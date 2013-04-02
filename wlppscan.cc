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
using namespace std;

//======================================================================
//========= Declarations for the scan() function =======================
//======================================================================

// Each token has one of the following kinds.

enum Kind {
  ID,                 // Opcode or identifier (use of a label)
  NUM,                // Decimal integer
  LPAREN,             // (
  RPAREN,             // )
  LBRACE,             // {
  RBRACE,             // }
  RETURN,             // return
  IF,                 // if
  ELSE,               // else
  WHILE,              // while
  PRINTLN,            // println
  WAIN,               // wain
  BECOMES,            // =
  INT,                // int
  EQ,                 // ==
  NE,                 // !=
  LT,                 // <
  GT,                 // >
  LE,                 // <=
  GE,                 // >=
  PLUS,               // +
  MINUS,              // -
  STAR,               // *
  SLASH,              // /
  PCT,                // %
  COMMA,              // Comma
  SEMI,               // ;
  NEW,                // new
  DELETE,             // delete
  LBRACK,             // [
  RBRACK,             // ]
  AMP,                // &
  NULL1,               // NULL
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
   * of kind NUM (decimal integer constant) and HEXNUM (hexadecimal integer
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
  ST_ZERO,
  ST_NUM,
  ST_STRING,
  ST_SYMBOL_SINGLE,
  ST_BECOMES,
  ST_EQUAL,
  ST_NOT,
  ST_LT,
  ST_GT,
  ST_NE,
  ST_EQ,
  ST_LE,
  ST_GE,
  ST_MINUS,
  ST_SLASH1,
  ST_COMMENT,
  ST_WHITESPACE
};

// The *kind* of token (see previous enum declaration)
// represented by each state; states that don't represent
// a token have stateKinds == NUL.

Kind stateKinds[] = {
  NUL,            // ST_NUL
  NUL,            // ST_START
  NUM,            //ST_ZERO
  NUM,            //ST_NUM
  ID,             //ST_STRING
  STAR,           //ST_SYMBOL_SINGLE
  BECOMES,         //ST_BECOMES
  EQ,             //ST_EQUAL
  NUL,            //ST_NOT
  LT,             //ST_LT
  GT,             //ST_GT
  NE,             //ST_NE
  EQ,             //ST_EQ
  LE,             //ST_LE
  GE,             //ST_GE
  MINUS,          //ST_MINUS
  SLASH,          //ST_SLASH1
  WHITESPACE,     //ST_COMMENT
  WHITESPACE      //ST_WHITESPACE
};

State delta[ST_WHITESPACE+1][256];

    #define whitespace "\t\n\r "
    #define letters    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    #define digits     "0123456789"
    #define oneToNine  "123456789"
    #define symbols "+-/*%&,;[]{}><="

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
  setT( ST_START,      letters,        ST_STRING         );
  setT( ST_STRING,     letters digits, ST_STRING         );
  setT( ST_START,      oneToNine,      ST_NUM        );
  setT( ST_NUM,        digits,         ST_NUM        );
  setT( ST_START,      "-",            ST_MINUS      );
  setT( ST_MINUS,      digits,       ST_NUM        );
  setT( ST_START,      symbols,        ST_SYMBOL_SINGLE);
  setT( ST_START,     "/",     ST_SLASH1 );
  setT( ST_SLASH1,     "//",    ST_COMMENT  );
  setT( ST_START,      ",",            ST_SYMBOL_SINGLE      );
  setT( ST_START,      "(",            ST_SYMBOL_SINGLE     );
  setT( ST_START,      ")",            ST_SYMBOL_SINGLE     );
  setT( ST_START,      "0",            ST_ZERO       );
  setT( ST_START,      "!",            ST_NOT        );
  setT( ST_START,      "=",            ST_BECOMES    );
  setT( ST_START,      ">",            ST_GT         );
  setT( ST_START,      "<",            ST_LT         );
  setT( ST_BECOMES,    "=",            ST_EQ         );
  setT( ST_GT,         "=",            ST_GE         );
  setT( ST_LT,         "=",            ST_LE         );
  setT( ST_NOT,        "=",            ST_NE         );

  for ( j=0; j<256; j++ ) delta[ST_COMMENT][j] = ST_COMMENT;
}

static int initT_done = 0;
int tokenType(Kind a){
  if(a==ID||a==NUM||a==RETURN||a==IF||a==ELSE||a==WHILE||a==PRINTLN||a==WAIN||a==INT||a==NEW||a==NULL1||a==DELETE){
    return 1;
  }else if(a==EQ||a==NE||a==LT||a==GT||a==GE||a==BECOMES){
    return 2;
  }else{
    return -1;
  }
}

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
    Kind previousStateKind = NUL;
    bool whitespace1 = 0;
    while(true) {
      State nextState = ST_NUL;
      // Kind previousStateKind = NU0L;
      // bool whitespace1 = 0;
      if(i < input.length())
	nextState = delta[state][(unsigned char) input[i]];
      if(nextState == ST_NUL) {
	// no more transitions possible
	if(stateKinds[state] == NUL) {
	  throw("ERROR in lexing after reading " + input.substr(0, i)); 
	}
	if(stateKinds[state] == WHITESPACE){
	  whitespace1 = 1;
	}
	if(stateKinds[state] != WHITESPACE) {
	  Token token;
	  token.kind = stateKinds[state];
	  token.lexeme = input.substr(startIndex, i-startIndex);
	  ret.push_back(token);
	  if(!whitespace1&&tokenType(previousStateKind)==tokenType(token.kind)&&tokenType(token.kind)!=-1){
	    throw("ERROR must have space between those tokens" + input.substr(0, i));
	  }
	  previousStateKind = token.kind;
	  whitespace1 = 0;
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
  if(kind == NUM) {
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
  } 
  throw("ERROR: attempt to convert non-integer token "+lexeme+" to Int");
}

// kindString maps each kind to a string for use in error messages.

string kS[] = {
  "ID",                 // Opcode or identifier (use of a label)
  "NUM",                // Decimal integer
  "LPAREN",             // (
  "RPAREN",             // )
  "LBRACE",             // {
  "RBRACE",             // }
  "RETURN",             // return
  "IF",                 // if
  "ELSE",               // else
  "WHILE",              // while
  "PRINTLN",            // println
  "WAIN",               // wain
  "BECOMES",            // =
  "INT",                // int
  "EQ",                 // ==
  "NE",                 // !=
  "LT",                 // <
  "GT",                 // >
  "LE",                 // <=
  "GE",                 // >=
  "PLUS",               // +
  "MINUS",              // -
  "STAR",               // *
  "SLASH",              // /
  "PCT",                // %
  "COMMA",              // Comma
  "SEMI",               // ;
  "NEW",                // new
  "DELETE",             // delete
  "LBRACK",             // [
  "RBRACK",             // ]
  "AMP",                // &
  "NULL",               // NULL
  "WHITESPACE",         // Whitespace
  "NUL"                 // Bad/invalid token
};

string kindString( Kind k ){
  if ( k < ID || k > NUL ) return "INVALID";
  return kS[k];
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

    // Now we process the tokens.
    // Sample usage: print the tokens of each line.
    for(int line=0; line < tokLines.size(); line++ ) {
      for(int j=0; j < tokLines[line].size(); j++ ) {
	Token token = tokLines[line][j];
	if(token.lexeme == "wain"){
	  token.kind = WAIN;
	}else if(token.lexeme == "int"){
	  token.kind = INT;
	}else if(token.lexeme == "("){
	  token.kind = LPAREN;
	}else if(token.lexeme == ")"){
	  token.kind = RPAREN;
	}else if(token.lexeme == "{"){
	  token.kind = LBRACE;
	}else if(token.lexeme == "}"){
	  token.kind = RBRACE;
	}else if(token.lexeme == "return"){
	  token.kind = RETURN;
	}else if(token.lexeme == "if"){
	  token.kind = IF;
	}else if(token.lexeme == "else"){
	  token.kind = ELSE;
	}else if(token.lexeme == "while"){
	  token.kind = WHILE;
	}else if(token.lexeme == "println"){
	  token.kind = PRINTLN;
	}else if(token.lexeme == "+"){
	  token.kind = PLUS;
	}else if(token.lexeme == "*"){
	  token.kind = STAR;
	}else if(token.lexeme == "%"){
	  token.kind = PCT;
	}else if(token.lexeme == ","){
	  token.kind = COMMA;
	}else if(token.lexeme == ";"){
	  token.kind = SEMI;
	}else if(token.lexeme == "["){
	  token.kind = LBRACK;
	}else if(token.lexeme == "]"){
	  token.kind = RBRACK;
	}else if(token.lexeme == "&"){
	  token.kind = AMP;
	}else if(token.lexeme == "NULL"){
	  token.kind = NULL1;
	}else if(token.lexeme == "new"){
	  token.kind = NEW;
	}else if(token.lexeme == "delete"){
	  token.kind = DELETE;
	}else if(token.lexeme == "-"){
	  token.kind = MINUS;
	}

	cout <<kindString(token.kind) 
	     << " " << token.lexeme<<endl;
      }
    }
  } catch(string msg) {
    cerr << msg << endl;
  }

  return 0;
}
