#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#define INT_MAX 2147483647

using namespace std;
//Variables que se usaran a lo largo del codigo 
int global_ident = 0;//Usado para calcular el indent
int identmax = 0;
int line = 1;//Llevar un recuento de la linea en la que se encuentra
bool ident = false;
stack<string> stack1;
// Tipos de tokens
enum TokenType {
    ID,
    LIT_NUM,
    LIT_STR,
    KW_DEF,
    KW_IF,
    KW_ELSE,
    KW_TRUE,
    KW_FALSE,
    KW_RETURN,
    KW_NONE,
    KW_GLOBAL,
    KW_IMPORT,
    KW_IN,
    KW_IS,
    KW_LAMBDA,
    KW_NONLOCAL,
    KW_NOT,
    KW_OR,
    KW_PASS,
    KW_RAISE,
    KW_TRY,
    KW_WHILE,
    KW_WITH,
    KW_YIELD,
    KW_AND,
    KW_AS,
    KW_ASSERT,
    KW_ASYNC,
    KW_AWAIT,
    KW_BREAK,
    KW_CLASS,
    KW_CONTINUE,
    KW_DEL,
    KW_ELIF,
    KW_EXCEPT,
    KW_FINALLY,
    KW_FOR,
    KW_FROM,
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV,
    OP_INTDIV,
    OP_MOD,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_EQ,
    OP_NE,
    OP_FLE,
    OP_ASSIGN,
    OP_LPAREN,
    OP_RPAREN,
    OP_LBRACKET,
    OP_RBRACKET,
    OP_COMMA,
    OP_COLON,
    OP_DOT,
    OP_ARROW,
    NEWLINE,
    INDENT,
    DEDENT,
    COMMENT,
    END_OF_FILE,
    ERROR,
    FUNC
};

// Valores de tokens
struct TokenValue {
    TokenType type;
    string str_val;
    long long int_val = 0;
    int linenum = 0;//numero de linea
    int columnum = 0;//numero de columna
};
struct nodo {//Estructura nodo que se usara para crear el Arbol
    string data;//El valor del nodo
    vector<nodo*> children;
    nodo() {
        data = "";
    }
    nodo(string value) : data(value) {

    }

    ~nodo() {

    }

    void insert(string value) {//Se crea el valor dentro del arbol
        nodo* newnodo = new nodo(value);
        children.push_back(newnodo);
    }
    void insert(nodo* child) {//Si se pide introducir un nodo se agregara el nodo ingresado a la funcion como hijo del nodo current
        children.push_back(child);
    }


};
struct Arbol {//El arbol AST
    nodo* root;
    Arbol() : root(nullptr) {}

    void printArbol(nodo* currentnodo, const string& prefix = "", bool isLastChild = true) {//Funcion para imprimir el arbol
        if (currentnodo == nullptr) {
            return;
        }

        cout << prefix;
        if (currentnodo != root) {
            if (isLastChild) {
                cout << " I--- ";
            }
            else {
                cout << "|--- ";
            }
        }
        cout << currentnodo->data << "\n";

        string espacio;
        if (isLastChild) {
            espacio = prefix + "    ";
        }
        else {
            espacio = prefix + "|   ";
        }
        string ultespacio;
        if (isLastChild) {
            ultespacio = prefix + "    ";
        }
        else {
            ultespacio = prefix + "|   ";
        }
        
        const size_t numChildren = currentnodo->children.size();
        if (numChildren == 0) {
            return; // No hay hijos, salir de la función
        }
        size_t childIndex = 0;
        for (nodo* child : currentnodo->children) {
            if (child == nullptr) {
                return;
            }
            const bool isLast = (++childIndex == numChildren);
            printArbol(child, isLast ? ultespacio : espacio, isLast);
        }
    }

    ~Arbol() {

    }
};
class ASTArbol {//Se crearon dos parseos en el codigo, uno busca errores dentro del codigo y en caso estos no existen efectua
    //el segundo parseo para crear el arbol
public:
    vector<TokenValue> vec2;
    Arbol ast;
    TokenValue current;
    int pos;
    nodo* parse();//Funciones que emulan la gramatica
    nodo* expr();
    nodo* def();

    nodo* term();
    nodo* termPrime(nodo* first);

    nodo* factor();
    nodo* literal();
    nodo* name();
    nodo* nameTail();

    nodo* list();
    nodo* exprList(string name);
    nodo* exprListTail(nodo* parent);
    void asignar(vector<TokenValue> vec1, TokenValue t1, int num) {//Al arbol se asignan los valores del scanner
        vec2 = vec1;
        current = t1;
        pos = num;
    };
};


// Palabras reservadas
unordered_map<string, TokenType> keywords = {

    {"def", KW_DEF},         {"if", KW_IF},         {"else", KW_ELSE},
    {"None", KW_NONE},       {"and", KW_AND},       {"as", KW_AS},
    {"assert", KW_ASSERT},   {"async", KW_ASYNC},   {"await", KW_AWAIT},
    {"break", KW_BREAK},     {"class", KW_CLASS},   {"continue", KW_CONTINUE},
    {"del", KW_DEL},         {"elif", KW_ELIF},     {"except", KW_EXCEPT},
    {"finally", KW_FINALLY}, {"for", KW_FOR},       {"True", KW_TRUE},
    {"False", KW_FALSE},     {"return", KW_RETURN}, {"from", KW_FROM},
    {"global", KW_GLOBAL},   {"import", KW_IMPORT}, {"in", KW_IN},
    {"is", KW_IS},           {"lambda", KW_LAMBDA}, {"nonlocal", KW_NONLOCAL},
    {"not", KW_NOT},         {"or", KW_OR},         {"pass", KW_PASS},
    {"raise", KW_RAISE},     {"try", KW_TRY},       {"while", KW_WHILE},
    {"with", KW_WITH},       {"yield", KW_YIELD} };

// Función auxiliar para verificar si un caracter es un dígito
bool is_digit(char c) { return c >= '0' && c <= '9'; }

// Función auxiliar para verificar si un caracter es una letra
bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Función para leer el input desde un archivo o un elemento de área de texto
string read_input(string filename) {
    string input;
    string line;
    ifstream file(filename);
    if (file.is_open()) {
        while (getline(file, line)) {
            input += line + '\n';
        }
        file.close();
    }
    return input;
}

// Función para calcular la indentación
int calc_indent(string line) {
    int indent = 0;
    for (char c : line) {
        if (c == ' ') {
            indent++;
        }
        else if (c == '\t') {
            indent += 4;
        }
        else {
            break;
        }
    }
    return indent;
}

// Función para calcular la dedentación
int calc_dedent(string line, int prevIndent) {
    int dedent = 0;
    for (char c : line) {
        if (c == ' ') {
            dedent++;
        }
        else if (c == '\t') {
            dedent += 4;
        }
        else {
            break;
        }
    }
    return prevIndent - dedent;
}

// Función para leer el siguiente caracter del input y mover el puntero del
// caracter al siguiente
char get_char(string& input, int& pos) {
    if (pos >= input.length()) {
        return '\0';
    }
    char c = input[pos];
    pos++;
    return c;
}

// Función para leer el siguiente caracter sin mover el puntero
char peek_char(string& input, int pos) {
    if (pos >= input.length()) {
        return '\0';
    }
    return input[pos];
}
//Funcion para leer cual es el siguiente token
TokenValue peek_token(vector<TokenValue> vec, int pos) {
    if (pos >= vec.size()) {
        return TokenValue();
    }
    return vec[pos + 1];
}
//Funcion para pasar de un token al siguiente 
TokenValue next_token(vector<TokenValue> vec, int& pos) {
    if (pos < vec.size() - 1) {
        pos += 1;
        return vec[pos]; // Return the next element in the vector
    }
    else {
        return TokenValue();
    }
};
//Funcion para que basandose en el string de texto puede generar el token que le corresponda 
TokenValue get_token(string& input, int& pos, int& line_num, int& col_num) {
    TokenValue token;
    string token_str;
    char c = get_char(input, pos);
    col_num++;

    // Ignorar espacios
    if (ident == true) {
        while (c == ' ') {
            c = get_char(input, pos);
            col_num++;
        }
    }

    // Nuevo línea
    if (c == '\n') {
        token.type = NEWLINE;
        token.str_val = "newline";
    }
    if (c == '\t') {
        token.type = INDENT;
        token.str_val = "indent";
        col_num = col_num + 4;
    }
    if (c == ' ') {
        token.type = INDENT;
        token.str_val = "indent";
        c = get_char(input, pos);
        col_num++;
    }
    // Indentación
    int indent = calc_indent(input.substr(pos - 1));
    if (indent > 0) {
        token.type = INDENT;
        if (indent > identmax) {
            global_ident = global_ident + 1;
        }
        if (indent > identmax) {
            identmax = indent;
        }

        line = line_num;
    }

    // Dindentación
    int dedent = 0;
    while (indent < 0 && peek_char(input, pos + dedent) == ' ') {
        dedent++;
        indent += 1;
    }
    if (line < line_num) {
        if (global_ident > indent) {
            dedent++;
            global_ident = global_ident - 1;
            line = line_num;
        }
    }
    if (dedent > 0) {
        token.type = DEDENT;
        token.str_val = "dedent";
        dedent--;
        token.linenum = line_num;
        token.columnum = col_num;
        identmax--;
        pos--;
        return token;
    }
    if (c == '\n') {
        line_num++;
        col_num = 0;
        ident = false;
    }
    // Identificador
    if (is_alpha(c) || c == '_') {
        ident = true;
        token_str += c;
        c = peek_char(input, pos);
        while (is_alpha(c) || is_digit(c) || c == '_') {
            token_str += get_char(input, pos);
            c = peek_char(input, pos);
        }
        if (keywords.count(token_str)) {
            token.type = keywords[token_str];
        }
        else {
            token.type = ID;
        }
        token.str_val = token_str;
        token.int_val = 0;
        token.linenum = line_num;
        token.columnum = col_num;
        return token;
    }

    // Número
    if (is_digit(c)) {
        stringstream t;
        t << c;
        long long temp = 0;
        t >> temp;
        if (temp < INT_MAX) {
            ident = true;
            token_str += c;
            c = peek_char(input, pos);
            while (is_digit(c)) {
                token_str += get_char(input, pos);
                c = peek_char(input, pos);
            }
            token.type = LIT_NUM;
            token.str_val = "lit_num";
            // token.str_val = token_str;
            if (stoll(token_str) >= 2147483648) {
                token.str_val = "this number surpass the int limit";
                token.int_val = 0;
            }
            else {
                token.int_val = stoi(token_str);
            }
            token.linenum = line_num;
            token.columnum = col_num;
            return token;
        }
    }

    // String
    if (c == '"') {
        ident = true;
        token_str += c;
        c = peek_char(input, pos);
        while (c != '\0' && c != '"') {
            if (c == '\\') {
                token_str += get_char(input, pos);
            }
            token_str += get_char(input, pos);
            c = peek_char(input, pos);
        }
        if (c == '\0') {
            token.type = ERROR;
            token.str_val = "Unexpected end of input in string literal";
            token.int_val = 0;
            token.linenum = line_num;
            token.columnum = col_num;
            return token;
        }
        token_str += get_char(input, pos);
        token.type = LIT_STR;
        token.str_val = token_str;
        token.int_val = 0;
        token.linenum = line_num;
        token.columnum = col_num;
        return token;
    }
    //otros valores existentes
    if (c == '#') {
        ident = true;
        c = peek_char(input, pos);
        while (c != '\0' && c != '\n') {
            if (c == '\\') {
                continue;
            }
            token_str += get_char(input, pos);
            c = peek_char(input, pos);
        }
        token_str = "comment";
        token.type = COMMENT;
        token.str_val = token_str;
        token.int_val = 0;
        token.linenum = line_num;
        token.columnum = col_num;
        return token;
    }

    // Operadores
    switch (c) {
    case '+':
        token.type = OP_PLUS;
        token.str_val = "+";
        break;
    case '-':
        if (peek_char(input, pos) == '>') {
            get_char(input, pos);
            token.type = OP_FLE;
            token.str_val = "->";
            break;
        }
        token.type = OP_MINUS;
        token.str_val = "-";
        break;
    case '*':
        token.type = OP_MULT;
        token.str_val = "*";
        break;
    case '/':
        if (peek_char(input, pos) == '/') {
            get_char(input, pos);
            token.type = OP_INTDIV;
            token.str_val = "//";
        }
        else {
            token.type = OP_DIV;
            token.str_val = "/";
        }
        break;
    case '%':
        token.type = OP_MOD;
        token.str_val = "%";
        break;
    case '<':
        if (peek_char(input, pos) == '=') {
            get_char(input, pos);
            token.type = OP_LE;
            token.str_val = "<=";
        }
        else {
            token.type = OP_LT;
            token.str_val = "<";
        }
        break;
    case '>':
        if (peek_char(input, pos) == '=') {
            get_char(input, pos);
            token.type = OP_GE;
            token.str_val = ">=";
        }
        else {
            token.type = OP_GT;
            token.str_val = ">";
        }
        break;
    case '=':
        if (peek_char(input, pos) == '=') {
            get_char(input, pos);
            token.type = OP_EQ;
            token.str_val = "==";
        }
        else {
            token.type = OP_ASSIGN;
            token.str_val = "=";
        }
        break;
    case '!':
        if (peek_char(input, pos) == '=') {
            get_char(input, pos);
            token.type = OP_NE;
            token.str_val = "!=";
        }
        else {
            token.type = ERROR;
            token.str_val = "Unexpected character '!'";
            token.int_val = 0;
            token.linenum = line_num;
            token.columnum = col_num;
            return token;
        }
        break;
    case '(':
        ident = true;
        token.type = OP_LPAREN;
        token.str_val = "(";
        break;
    case ')':
        token.type = OP_RPAREN;
        token.str_val = ")";
        break;
    case '[':
        ident = true;
        token.type = OP_LBRACKET;
        token.str_val = "[";
        break;
    case ']':
        token.type = OP_RBRACKET;
        token.str_val = "]";
        break;
    case ',':
        token.type = OP_COMMA;
        token.str_val = ",";
        break;
    case ':':
        token.type = OP_COLON;
        token.str_val = ":";
        break;
    case '\n':
        break;
    case '\0':
        token.type = END_OF_FILE;
        token.str_val = "\0";
        break;
    case '\t':
        break;
    case ' ':

        break;
    default:
        token.type = ERROR;
        token.str_val = "Unexpected character '" + string(1, c) + "'";
        token.int_val = 0;
        break;
    }
    token.linenum = line_num;
    token.columnum = col_num;
    return token;
}
// Parser
void StackError(string error, int linea, int colum) {//Ests funcion sirve para guardar los errores encontrados en el codigo en un 
    //en un stack
    cout << "Error:" << error << " ,linea: " << linea << " columna: " << colum << endl;
    stack1.push(error);
}
//Todas estas funciones buscan emular la gramatica, cada funcion devuelve un booleano para poder confirmar donde se encontre el error
bool Expr(vector<TokenValue> vec1, TokenValue& t1, int& pos);
bool SSTail(vector<TokenValue> vec1, TokenValue& t1, int& pos);
bool Block(vector<TokenValue> vec1, TokenValue& t1, int& pos);
bool Else(vector<TokenValue> vec1, TokenValue& t1, int& pos);
bool SimpleStatement(vector<TokenValue> vec1, TokenValue& t1, int& pos);
bool ElifList(vector<TokenValue> vec1, TokenValue& t1, int& pos);
bool Statement(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "if") {
        t1 = next_token(vec1, pos);
        if (Expr(vec1, t1, pos)) {
            if (t1.str_val == ":") {
                t1 = next_token(vec1, pos);
                if (Block(vec1, t1, pos) and ElifList(vec1, t1, pos) and Else(vec1, t1, pos)) {
                    return true;
                }
                else {
                    StackError("Se esperaba block/eliflist/else", t1.linenum, t1.columnum);
                    return false;
                }
            }
            StackError("Se esperaba ':' despues de la expresion", t1.linenum, t1.columnum);
            return false;
        }
        StackError("Se esperaba expresion", t1.linenum, t1.columnum);
        return false;
    }
    else if (t1.str_val == "while") {
        t1 = next_token(vec1, pos);
        if (Expr(vec1, t1, pos)) {//Cada vez que se devuelve un true el valor de pos avanza en uno para seguir verificando el vector
            if (t1.str_val == ":") {
                t1 = next_token(vec1, pos);
                if (Block(vec1, t1, pos)) {
                    return true;
                }
                else {
                    StackError("Se esperaba Block", t1.linenum, t1.columnum);
                    return false;
                }
            }
            StackError("Se esperaba':'", t1.linenum, t1.columnum);
            return false;
        }
        StackError("Se esperaba expresion", t1.linenum, t1.columnum);
        return false;
    }
    else if (t1.str_val == "for") {
        t1 = next_token(vec1, pos);
        if (t1.type == ID) {
            t1 = next_token(vec1, pos);
            if (t1.str_val == "in") {
                t1 = next_token(vec1, pos);
                if (Expr(vec1, t1, pos)) {
                    if (t1.str_val == ":") {
                        t1 = next_token(vec1, pos);
                        return Block(vec1, t1, pos);
                    }
                    StackError("Se esperaba op", t1.linenum, t1.columnum);
                    return false;
                }
                StackError("Se esperaba expresion", t1.linenum, t1.columnum);
                return false;
            }
            StackError("Se esperaba 'in'", t1.linenum, t1.columnum);
            return false;
        }
        StackError("Se esperaba ID", t1.linenum, t1.columnum);
        return false;
    }
    else if (SimpleStatement(vec1, t1, pos)) {
        if (t1.type == NEWLINE) {
            t1 = next_token(vec1, pos);
            return true;
        }
        StackError("Se esperaba expresion", t1.linenum, t1.columnum);
        return false;
    }
    //Los vectores follow existen para verificar los follow de los no terminales de la gramatica
    vector<string> follow = { "if",  "while",  "pass", "return", "for", "dedent", "True", "False","No","lit_num" };
    if (t1.str_val == "->" || t1.str_val == "(" ||
        t1.str_val == "[") {
        return true;
    }
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i])
            return true;
    }
    if (t1.type == ID || t1.type == LIT_STR) {
        return true;
    }
    return false;
}
bool StatementList(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.type == END_OF_FILE or t1.type == DEDENT) {
        return true;
    }
    if (Statement(vec1, t1, pos) and StatementList(vec1, t1, pos)) {
        return true;
    }
    return false;
    //return Statement(vec1, t1, pos) and StatementList(vec1, t1, pos);
}
bool Elif(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "elif") {
        t1 = next_token(vec1, pos);
        if (Expr(vec1, t1, pos)) {
            if (t1.str_val == ":") {
                t1 = next_token(vec1, pos);
                return Block(vec1, t1, pos);
            }
            StackError("Se esperaba ':' despues de la expresion", t1.linenum, t1.columnum);
            return false;
        }
        return false;
    }

    return false;
}
bool ElifList(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (Elif(vec1, t1, pos) and ElifList(vec1, t1, pos)) {
        return true;
    }
    vector<string> follow = { "if","while","for","pass","return","not","->","(","None","True","False","[","else" };
    if (t1.type == LIT_NUM or t1.type == LIT_STR or t1.type == ID or t1.type == END_OF_FILE or t1.type == NEWLINE) {
        return true;
    }
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i]) {
            return true;
        }
    }
    return false;
}

bool Block(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.type == NEWLINE) {
        t1 = next_token(vec1, pos);
        if (t1.type == INDENT) {
            t1 = next_token(vec1, pos);
            if (Statement(vec1, t1, pos)) {
                if (StatementList(vec1, t1, pos)) {
                    if (t1.str_val == "dedent") {
                        t1 = next_token(vec1, pos);
                        return true;
                    }
                    StackError("Se esperaba dedent", t1.linenum, t1.columnum);
                    return false;
                }
            }
            StackError("Se esperaba expresion", t1.linenum, t1.columnum);
            return false;
        }
        StackError("Se esperaba identacion", t1.linenum, t1.columnum);
        return false;
    }
    vector<string> follow = { "def", "while", "pass", "return",
                             "for", "elif",  "True", "False" ,"if","None","lit_num" };
    if (t1.str_val == "-" || t1.str_val == "(" || t1.str_val == "[") {
        return true;
    }
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i])
            return true;
    }
    if (t1.type == ID || t1.type == LIT_STR) {
        return true;
    }
    return false;
}

bool Type(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "int" || t1.str_val == "str") {
        return true;
    }
    else {
        if (t1.str_val == "[") {
            t1 = next_token(vec1, pos);
            if (Type(vec1, t1, pos)) {
                if (t1.str_val == "]")
                    return true;
            }
            else {
                StackError("Se esperaba ']'", t1.linenum, t1.columnum);
                return false;
            }
        }
        else {
            StackError("Se especifico un tipo no valido", t1.linenum, t1.columnum);
            return false;
        }
    }
    vector<string> follow = { "]", ":", ",", ")" };
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i])
            return true;
    }
    return false;
}

bool TypedVar(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.type == ID) {
        t1 = next_token(vec1, pos);
        if (t1.str_val == ":") {
            t1 = next_token(vec1, pos);
            if (Type(vec1, t1, pos)) {
                t1 = next_token(vec1, pos);
                return true;
            }
            else {
                StackError("Tipo no reconocido", t1.linenum, t1.columnum);
                return false;
            }
        }
        else {
            StackError("Se esperaba declaracion de tipo", t1.linenum, t1.columnum);
            return false;
        }
    }
    if (t1.type == OP_RPAREN) {
        return true;
    }
    if (t1.str_val == ",") {
        return true;
    }
    return false;
}
bool TypedVarListTail(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == ",") {
        t1 = next_token(vec1, pos);
        if (TypedVar(vec1, t1, pos)) {
            if (TypedVarListTail(vec1, t1, pos)) {
                return true;
            }
            else {
                StackError("Lista de Variables tipadas no puede acabar en coma", t1.linenum, t1.columnum);
                return false;
            }
        }
        else {
            StackError("Tipo incorrecto", t1.linenum, t1.columnum);
            return false;
        }
    }
    if (t1.type == OP_RPAREN) {
        return true;
    }
    return false;
}

bool TypedVarList(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    return TypedVar(vec1, t1, pos) && TypedVarListTail(vec1, t1, pos);
}

bool Return(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "->") {
        t1 = next_token(vec1, pos);
        if (Type(vec1, t1, pos)) {
            return true;
        }
        else {
            StackError("Se esperaba tipo", t1.linenum, t1.columnum);
            return false;
        }
    }
    if (t1.str_val == ":")
        return true;
    return false;
}

bool Def(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "def") {
        t1 = next_token(vec1, pos);
        if (t1.type == ID) {
            t1 = next_token(vec1, pos);
            if (t1.str_val == "(") {
                t1 = next_token(vec1, pos);
                if (TypedVarList(vec1, t1, pos)) {
                    if (t1.str_val == ")") {
                        t1 = next_token(vec1, pos);
                        if (Return(vec1, t1, pos)) {
                            if (t1.str_val == ":") {
                                t1 = next_token(vec1, pos);
                                if (Block(vec1, t1, pos)) {
                                    return true;
                                }
                                else {
                                    StackError("Se esperaba bloque", t1.linenum, t1.columnum);
                                    return false;
                                }
                            }
                            else {
                                StackError("Se esperaba ':'", t1.linenum, t1.columnum);
                                return false;
                            }
                        }
                        else {
                            StackError("Se esperaba expresion Return", t1.linenum, t1.columnum);
                            return false;
                        }
                    }
                    else {
                        StackError("Se esperaba ')'", t1.linenum, t1.columnum);
                        return false;
                    }
                }
                else {
                    StackError("Declaracion Inesperada", t1.linenum, t1.columnum);
                    return false;
                }
            }
            else {
                StackError("Se esperaba '('", t1.linenum, t1.columnum);
                return false;
            }
        }
        else {
            StackError("Se esperaba ID", t1.linenum, t1.columnum);
            return false;
        }
    }
    return false;
}

bool DefList(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (Def(vec1, t1, pos)) {
        if (t1.type == END_OF_FILE) {
            return true;
        }
        //t1 = next_token(vec1,pos);
        if (DefList(vec1, t1, pos)) {
            return true;
        }
        else {
            StackError("Listado incorrecto de definiciones", t1.linenum, t1.columnum);
            return false;
        }
    }
    vector<string> follow = { "if", "while", "for",  "pass",  "return",  "-",
                             "(",  "ID",    "true", "false", "lit_num", "[" };
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i]) {
            return true;
        }
    }
    if (t1.type == LIT_STR || t1.type == ID) {
        return true;
    }
    return false;
}

bool parse(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    return DefList(vec1, t1, pos) && StatementList(vec1, t1, pos);
}

bool Else(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "else") {
        t1 = next_token(vec1, pos);
        if (t1.str_val == ":") {
            return Block(vec1, t1, pos);
        }
        else {
            StackError("Se esperaba ':' despues de else", t1.linenum, t1.columnum);
            return false;
        }
    }
    vector<string> follow = { "if",  "while",  "pass", "return", "for",
                             "def", "dedent", "True", "False" };
    if (t1.type == LIT_NUM or t1.type == LIT_STR or t1.type == ID or t1.type == END_OF_FILE or t1.type == NEWLINE) {
        return true;
    }
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i])
            return true;
    }
    return false;
}
bool ReturnExpr(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (Expr(vec1, t1, pos)) {
        return true;
    }
    if (t1.type == NEWLINE) {
        return true;
    }
    return false;
}
bool SimpleStatement(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "return") {
        if (t1.str_val == "return") {
            t1 = next_token(vec1, pos);
            if (ReturnExpr(vec1, t1, pos)) {
                return true;
            }
            else {
                StackError("Expresion no valida", t1.linenum, t1.columnum);
                return false;
            }
        }
    }
    if (t1.str_val == "pass") {
        t1 = next_token(vec1, pos);
        return true;
    }
    else if (Expr(vec1, t1, pos)) {
        // t1 = next_token(vec1, pos);
        if (SSTail(vec1, t1, pos)) {
            return true;
        }
        else {
            StackError("Declaracion no valida", t1.linenum, t1.columnum);
            return false;
        }
    }
    return false;
}
bool SSTail(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "=") {
        t1 = next_token(vec1, pos);
        if (Expr(vec1, t1, pos)) {
            return true;
        }
        StackError("Expresion no valida", t1.linenum, t1.columnum);
        return false;
    }
    if (t1.type == NEWLINE) {
        return true;
    }
    return false;
}
bool CompOp(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    bool comp_op = false;
    comp_op |= t1.str_val == "==";
    comp_op |= t1.str_val == "!=";
    comp_op |= t1.str_val == "<";
    comp_op |= t1.str_val == ">";
    comp_op |= t1.str_val == "<=";
    comp_op |= t1.str_val == ">=";
    comp_op |= t1.str_val == "is";

    if (comp_op) {
        t1 = next_token(vec1, pos);
    }
    return comp_op;
}
bool literal(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    bool literal = false;
    literal |= t1.str_val == "True";
    literal |= t1.str_val == "False";
    literal |= t1.str_val == "None";
    literal |= t1.str_val == "lit_num";
    literal |= t1.type == LIT_STR;

    if (literal) {
        t1 = next_token(vec1, pos);
    }
    return literal;
}
bool exprListTail(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.type == OP_COMMA) {
        t1 = next_token(vec1, pos);
        if (Expr(vec1, t1, pos)) {
            return exprListTail(vec1, t1, pos);
        }
        else {
            StackError("Una lista no puede acabar en coma", t1.linenum, t1.columnum);
            return false;
        }
    }
    else if (t1.type == OP_RBRACKET) {
        return true;
    }
    return false;
}
bool exprList(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (Expr(vec1, t1, pos) && exprListTail(vec1, t1, pos)) {
        return true;
    }
    else if (t1.type == OP_RBRACKET) {
        return true;
    }
    StackError("Lista de Expresiones invalida", t1.linenum, t1.columnum);
    return false;
}
bool List(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.type == OP_LBRACKET) {
        t1 = next_token(vec1, pos);
        if (exprList(vec1, t1, pos)) {
            if (t1.type == OP_RBRACKET) {
                t1 = next_token(vec1, pos);
                return true;
            }
            StackError("Lista no cerrada", t1.linenum, t1.columnum);
            return false;
        }
    }
    return false;
}
bool nameTail(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (List(vec1, t1, pos)) {
        return true;
    }
    if (t1.type == OP_LPAREN) {
        t1 = next_token(vec1, pos);
        if (exprList(vec1, t1, pos)) {
            if (t1.type == OP_RPAREN) {
                t1 = next_token(vec1, pos);
                return true;
            }
        }
        StackError("Se esperaba el operador ')'", t1.linenum, t1.columnum);
        return false;
    }
    vector<string> follow = { "if", "else", "or", "and", "==", "!=", ">=", "<=",
                             "<",  ">",    "is", "+",   "-",  "*",  "//", "%" };
    if (t1.str_val == "newline" || t1.str_val == "=" || t1.str_val == ":" ||
        t1.str_val == ")" || t1.str_val == "dedent") {
        return true;
    }
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i])
            return true;
    }
    return false;
}
bool name(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.type == ID) {
        t1 = next_token(vec1, pos);
        return nameTail(vec1, t1, pos);
    }
    return false;
}
bool factor(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.type == OP_MINUS) {
        t1 = next_token(vec1, pos);
        return factor(vec1, t1, pos);
    }
    if (name(vec1, t1, pos)) {
        return true;
    }
    if (literal(vec1, t1, pos)) {
        return true;
    }
    if (List(vec1, t1, pos)) {
        return true;
    }
    if (t1.type == OP_LPAREN) {
        t1 = next_token(vec1, pos);
        if (Expr(vec1, t1, pos)) {
            if (t1.type == OP_RBRACKET) {
                t1 = next_token(vec1, pos);
                return true;
            }
        }
        StackError("Se esperaba el operador ')'", t1.linenum, t1.columnum);
        return false;
    }
    if (t1.type == DEDENT) {
        return true;
    }
    return false;
}
bool termPrime(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "*" || t1.str_val == "//" || t1.str_val == "%") {
        t1 = next_token(vec1, pos);
        if (factor(vec1, t1, pos)) {
            return termPrime(vec1, t1, pos);
        }
        StackError("Se esperaba expresion despues de operacion", t1.linenum, t1.columnum);
        return false;
    }
    vector<string> follow = { "if", "else", "or", "and", "==", "!=", ">=",
                             "<=", "<",    ">",  "is",  "+",  "-" };
    if (t1.str_val == "newline" || t1.str_val == "=" || t1.str_val == ":" ||
        t1.str_val == ")") {
        return true;
    }
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i])
            return true;
    }
    return false;
}
bool term(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    return factor(vec1, t1, pos) and termPrime(vec1, t1, pos);
}
bool intExprPrime(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.type == OP_PLUS || t1.type == OP_MINUS) {
        t1 = next_token(vec1, pos);
        if (term(vec1, t1, pos)) {
            return intExprPrime(vec1, t1, pos);
        }
        StackError("Se esperaba expresion despues del operador", t1.linenum, t1.columnum);
        return false;
    }
    vector<string> follow = {
        "if", "else", "or", "and", "==", "!=", ">=", "<=", "<", ">", "is" };
    if (t1.str_val == "newline" || t1.str_val == "=" || t1.str_val == ":" ||
        t1.str_val == ")") {
        return true;
    }
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i])
            return true;
    }
    return false;
}
bool intExpr(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    return term(vec1, t1, pos) and intExprPrime(vec1, t1, pos);
}
bool CompExprPrime(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (CompOp(vec1, t1, pos)) {
        if (intExpr(vec1, t1, pos)) {
            return CompExprPrime(vec1, t1, pos);
        }
        StackError("Se esperaba expresion despues del operador compuesto", t1.linenum, t1.columnum);
        return false;
    }
    vector<string> follow = { "if", "else", "or", "and","," };
    if (t1.str_val == "newline" || t1.str_val == "=" || t1.str_val == ":" ||
        t1.str_val == ")" || t1.str_val == "]") {
        return true;
    }
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i])
            return true;
    }
    return false;
}
bool CompExpr(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    return intExpr(vec1, t1, pos) and CompExprPrime(vec1, t1, pos);
}
bool notExprPrime(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "not") {
        t1 = next_token(vec1, pos);
        if (CompExpr(vec1, t1, pos)) {
            return notExprPrime(vec1, t1, pos);
        }
        StackError("Se esperaba expresion despues de 'not'", t1.linenum, t1.columnum);
        return false;
    }
    vector<string> follow = { "if", "else", "or", "and" };
    if (t1.str_val == "newline" || t1.str_val == "=" || t1.str_val == ":" ||
        t1.str_val == ")") {
        return true;
    }
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i]) {
            return true;
        }
    }
    return false;
}
bool notExpr(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    return CompExpr(vec1, t1, pos) and notExprPrime(vec1, t1, pos);
    /*if (t1.str_val == "not") {
        t1 = next_token(vec1,pos);
        return notExpr(vec1, t1, pos);
    }
    return CompExpr(vec1, t1,  pos);*/
}
bool andExprPrime(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "and") {
        t1 = next_token(vec1, pos);
        if (notExpr(vec1, t1, pos)) {
            return andExprPrime(vec1, t1, pos);
        }
        StackError("Se esperaba expresion despues de 'and'", t1.linenum, t1.columnum);
        return false;
    }
    vector<string> follow = { "if", "else", "or","," };
    if (t1.str_val == "newline" || t1.str_val == "=" || t1.str_val == ":" ||
        t1.str_val == ")") {
        return true;
    }
    for (int i = 0; i < follow.size(); i++) {
        if (t1.str_val == follow[i]) {
            return true;
        }
    }
    return false;
}

bool andExpr(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    return notExpr(vec1, t1, pos) and andExprPrime(vec1, t1, pos);
}
bool orExprPrime(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "or") {
        t1 = next_token(vec1, pos);
        if (andExpr(vec1, t1, pos)) {
            return orExprPrime(vec1, t1, pos);
        }
        StackError("Se esperaba expresion despues de 'or'", t1.linenum, t1.columnum);
        return false;
    }
    vector<string> follow = { "if", "else" };
    if (t1.str_val == "newline" || t1.str_val == "=" || t1.str_val == ":" ||
        t1.str_val == ")") {
        return true;
    }
    for (int i = 0; i < 2; i++) {
        if (t1.str_val == follow[i]) {
            return true;
        }
    }
    return false;
}
bool orExpr(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    return andExpr(vec1, t1, pos) and orExprPrime(vec1, t1, pos);
}

bool ExprPrime(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    if (t1.str_val == "if") {
        t1 = next_token(vec1, pos);
        if (andExpr(vec1, t1, pos)) {
            t1 = next_token(vec1, pos);
            if (t1.str_val == "else") {
                if (andExpr(vec1, t1, pos)) {
                    return ExprPrime(vec1, t1, pos);
                }
                StackError("Se esperaba expresion despues de 'else'", t1.linenum, t1.columnum);
                return false;
            }
        }
        else {
            StackError("Se esperaba expresion", t1.linenum, t1.columnum);
            return false;
        }
    }
    if (t1.str_val == "newline" || t1.str_val == "=" || t1.str_val == ":" ||
        t1.str_val == ")") {
        return true;
    }
    return false;
}
bool Expr(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    return orExpr(vec1, t1, pos) and ExprPrime(vec1, t1, pos);
}
//Esta es la funcion principal del parser
bool parser(vector<TokenValue> vec1, TokenValue& t1, int& pos) {
    bool env = false;
    if (parse(vec1, t1, pos) and t1.type == END_OF_FILE) {
        return true;
    }//En caso el parser devuelva false verfica que no es final del codigo, en caso no lo sea se volvera a correr para 
    //buscar mas errores
    else if (!parse(vec1, t1, pos) and t1.type != END_OF_FILE) {
        if (t1.type == NEWLINE) {
            t1 = next_token(vec1, pos);
            if (t1.type == DEDENT) {
                t1 = next_token(vec1, pos);
            }
            parser(vec1, t1, pos);
        }
    }
    return env;
}
// Funciónes AST, no contempla errores porque solo se llama a la creacion del arbol en caso no existan errores
nodo* ASTArbol::literal()
{
    nodo* lit = nullptr;

    if (current.str_val == "True" || current.str_val == "None" || current.str_val == "False") {
        lit = new nodo(current.str_val);
        current = next_token(vec2, pos);
    }
    else if (current.type == LIT_NUM || current.type == LIT_STR) {
        lit = new nodo(current.str_val);
        current = next_token(vec2, pos);
    }

    return lit;
}

nodo* ASTArbol::exprListTail(nodo* parent)
{
    vector<string> error_follow = { ")","]","newline" };

    if (current.type == OP_COMMA) {
        current = next_token(vec2, pos);
        nodo* expr_nodo;
        if ((expr_nodo = expr())) {
            parent->insert(expr_nodo);
            return exprListTail(parent);
        }
    }

    if (current.type == OP_RBRACKET || current.type == OP_RPAREN)
        return parent;
}

nodo* ASTArbol::exprList(string name)
{
    nodo* exprList_nodo = new nodo(name);

    nodo* expr_nodo;
    if ((expr_nodo = expr())) {
        exprList_nodo->insert(expr_nodo);
        nodo* exprListTail_nodo = exprListTail(exprList_nodo);

        return exprListTail_nodo;
    }

    if (current.type == OP_RBRACKET || current.type == OP_RPAREN)
        return exprList_nodo;


}

nodo* ASTArbol::list()
{
    if (current.type == OP_LBRACKET) {
        current = next_token(vec2, pos);
        nodo* exprList_nodo = exprList("LIST");

        vector<string> follow = { "newline",")","]","," };


        current = next_token(vec2, pos);
        return exprList_nodo;
    }

    return nullptr;
}

nodo* ASTArbol::nameTail()
{
    nodo* nameTail_nodo = nullptr;

    vector<string> follow = {
        "newline",")","]",",","*","+"
    };

    if (current.str_val == "(") {
        current = next_token(vec2, pos);
        nodo* exprList_nodo = exprList("ARGS");



        current = next_token(vec2, pos);
        return exprList_nodo;
    }
    if ((nameTail_nodo = list())) {
        return nameTail_nodo;
    }

    if (find(follow.begin(), follow.end(), current.str_val) != follow.end()) {
        return nameTail_nodo;
    }


}

nodo* ASTArbol::name()
{
    if (current.type != ID)
        return nullptr;

    nodo* name_nodo = new nodo("ID");
    name_nodo->insert(current.str_val);
    current = next_token(vec2, pos);

    nodo* nameTail_nodo;
    if ((nameTail_nodo = nameTail()))
        name_nodo->insert(nameTail_nodo);
    return name_nodo;
}

nodo* ASTArbol::factor()
{
    nodo* factor_nodo = nullptr;

    if (current.str_val == "-") {
        current = next_token(vec2, pos);
        nodo* neg_nodo = new nodo("-");
        neg_nodo->insert(factor());
        return neg_nodo;
    }
    if ((factor_nodo = name())) {
        return factor_nodo;
    }

    if ((factor_nodo = literal())) {
        return factor_nodo;
    }
    if ((factor_nodo = list())) {
        return factor_nodo;
    }

    vector<string> follow = {
        "newline",")","]",",","*"
    };


    current = next_token(vec2, pos);
    nodo* expr_nodo = expr();


    current = next_token(vec2, pos);
    return expr_nodo;
}

nodo* ASTArbol::termPrime(nodo* first)
{
    vector<string> start = { "*", "//", "%","+" ,"-" };
    for (auto a : start) {
        if (current.str_val == a) {
            nodo* op = new nodo(current.str_val);
            op->insert(first);
            current = next_token(vec2, pos);
            nodo* second = factor();
            op->insert(second);

            nodo* termPrime_nodo = termPrime(op);
            return termPrime_nodo;
        }
    }

    vector<string> follow1 = { "if","else" ,"or" ,"and" ,"==" ,"!=" ,">=" ,"<=" ,"<" ,">" ,"is" ,"+" ,"-" };
    for (auto a : follow1) {

        if (current.str_val == a) {
            nodo* op = new nodo(current.str_val);
            op->insert(first);
            current = next_token(vec2, pos);
            nodo* second = factor();
            op->insert(second);

            nodo* termPrime_nodo = termPrime(op);
            return termPrime_nodo;
        }
    }
    vector<string> follow = { "newline",")","]","," ,"+" ,"-" };
    if (find(follow.begin(), follow.end(), current.str_val) != follow.end()) {
        return first;
    }

}
nodo* ASTArbol::term()
{
    nodo* factor_nodo = factor();
    nodo* term_prime_nodo = termPrime(factor_nodo);
    return term_prime_nodo;
}

nodo* ASTArbol::expr()
{
    return term();
}

nodo* ASTArbol::def() {
    vector<string> follow = { "if", "while", "for",  "pass",  "return",  "-",
                            "(",  "ID",    "true", "false", "lit_num", "[" ,"def" };
    for (int i = 0; i < follow.size(); i++) {
        if (current.str_val == follow[i]) {
            nodo* block_node = new nodo("Bloque");
            while (current.type != INDENT) {
                current = next_token(vec2, pos);
            }
            current = next_token(vec2, pos);
            block_node->insert(expr());
            return block_node;
        }
    }
    return expr();
}


nodo* ASTArbol::parse()
{
    nodo* parse_nodo = new nodo("PROGRAM");
    parse_nodo->insert(def());
    if (current.type == NEWLINE) {
        current = next_token(vec2, pos);
        nodo* parse_dedent = new nodo("dedent");
        parse_nodo->insert(parse_dedent);
    }
    return parse_nodo;
}
int main() {
    // string input = "def add(a, b):\n"
    //                "\treturn a + b\n"
    //               "\n"
    //                "print(add(1, 2))\n";
    string input = read_input("input.txt");
    int pos = 0;
    int line_num = 1;
    int col_num = 0;
    TokenValue token = get_token(input, pos, line_num, col_num);
    vector<TokenValue> vec1;
    vector<TokenValue> vec2;
    vector<vector<TokenValue>> lineas;
    vec1.push_back(token);
    vec2.push_back(token);
    cout << "--------------------Scanner--------------------" << endl;
    while (token.type != END_OF_FILE) {
        string tabs = "\t";
        if (token.str_val.length() < 4) {
            tabs += "\t";
        }
        int spcng = 0;
        if (token.type != NEWLINE && token.type != INDENT && token.type != DEDENT &&
            token.type != ERROR) {
            spcng += token.str_val.length() - 1;
        }
        cout << "Found: " << token.str_val << tabs + "at line:" << line_num
            << ", column: " << col_num << "\t(Token Type: " << token.type << ")"
            << endl;
        col_num += spcng;
        token = get_token(input, pos, line_num, col_num);
        if (token.type == NEWLINE) {
            token.linenum = vec2[vec2.size() - 1].linenum;
            token.columnum = vec2[vec2.size() - 1].columnum + 1;

        }
        vec1.push_back(token);
        vec2.push_back(token);

        if (token.type == NEWLINE or token.type == END_OF_FILE) {
            lineas.push_back(vec1);
            vec1.clear();
        }
    }
    cout << endl;
    cout << "--------------------Parser--------------------" << endl;
    pos = 0;
    bool indent = 0;
    bool dedent = 0;
    for (int i = 0; i < vec2.size() - 1; i++) {
        if (vec2[i].type == INDENT and !indent) {
            indent = true;
            //TokenValue c=vec2[i];
            //vec2[i] = vec2[i + 1];
            //vec2[i + 1] = c;
        }
        else if (vec2[i].type == INDENT and indent == true) {
            vec2.erase(vec2.begin() + i);
        }
        else if (vec2[i].type == DEDENT) {
            indent = false;
        }
    }
    for (int i = 0; i < vec2.size() - 1; i++) {
        if (vec2[i].type == DEDENT and vec2[i + 1].type == NEWLINE) {
            vec2.erase(vec2.begin() + i + 1);
        }
    }
    TokenValue t2 = vec2[0];

    bool factible = parser(vec2, t2, pos);
    if (stack1.size() > 0) {
        factible = false;
    }
    cout << factible << endl;
    while (!stack1.empty()) {
        string element = stack1.top();
        stack1.pop();
        //cout << element << endl;
    }
    t2 = vec2[0];
    pos = 0;
    ASTArbol arb1;
    arb1.asignar(vec2, t2, pos);
   if (factible == true) {
        arb1.ast.root = arb1.parse();
        cout << "Creacion Arbol"<<endl;
        arb1.ast.printArbol(arb1.ast.root);
    }
    cout << endl;
}