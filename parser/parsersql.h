//
// Created by david on 18/05/2024.
//

#ifndef DB2_PROJECT1_PARSERSQL_H
#define DB2_PROJECT1_PARSERSQL_H

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <iomanip>
#include "data.h"

// #include "Sequential.h"
// #include "ISAM.h"
// #include "ExtendibleHashing.h"

using namespace std;


string KEY = "id";

class Token {
public:
    enum Type { ID, ERR, END, NUM, EOL, SELECT, FROM, WHERE, INSERT, DELETE_T, INTO, SET,
        VALUES, AND, OR, NOT, GT, LT, GTEQ, LTEQ, EQ, NEQ, LPAREN, RPAREN, COMILLAS,
        USING, INDEX, BETWEEN, CAMPO, CREATE, TABLE, FILE, SEMICOLON, COMMA, BOOL,FLOAT};
    static const char* token_names[36];
    Type type;
    string lexema;
    Token(Type);
    Token(Type, const string source);
};

const char* Token::token_names[36] = { "ID", "ERR", "END", "NUM", "EOL", "SELECT", "FROM",
                                       "WHERE", "INSERT", "DELETE", "INTO", "SET", "VALUES", "AND", "OR", "NOT", "GT",
                                       "LT", "GTEQ", "LE", "LTEQ", "NEQ", "LPAREN", "RPAREN", "COMILLAS", "USING", "INDEX",
                                       "BETWEEN", "CAMPO", "CREATE", "TABLE", "FILE", "SEMICOLON", "COMMA", "BOOL" , "FLOAT"};

Token::Token(Type type):type(type) { lexema = ""; }

Token::Token(Type type, const string source):type(type) {
    lexema = source;
}

std::ostream& operator << ( std::ostream& outs, const Token & tok )
{
    if (tok.lexema.empty())
        return outs << Token::token_names[tok.type];
    else
        return outs << Token::token_names[tok.type] << "(" << tok.lexema << ")";
}

std::ostream& operator << ( std::ostream& outs, const Token* tok ) {
    return outs << *tok;
}


class Scanner {
public:
    Scanner(const char* in_s);
    Token* nextToken();
    ~Scanner();
    void print_input() { cout << input << endl; }
private:
    string input;
    int first, current;
    unordered_map<string, Token::Type> reserved;
    char nextChar();
    void rollBack();
    void startLexema();
    string getLexema();
    Token::Type checkReserved(string);
};

Scanner::Scanner(const char* s):input(s),first(0),current(0) {
    reserved["SELECT"] = Token::SELECT;
    reserved["FROM"] = Token::FROM;
    reserved["WHERE"] = Token::WHERE;
    reserved["INSERT"] = Token::INSERT;
    reserved["DELETE"] = Token::DELETE_T;
    reserved["INTO"] = Token::INTO;
    reserved["SET"] = Token::SET;
    reserved["VALUES"] = Token::VALUES;
    reserved["AND"] = Token::AND;
    reserved["OR"] = Token::OR;
    reserved["NOT"] = Token::NOT;
    reserved["USING"] = Token::USING;
    reserved["INDEX"] = Token::INDEX;
    reserved["BETWEEN"] = Token::BETWEEN;
    reserved["CAMPO"] = Token::CAMPO;
    reserved["CREATE"] = Token::CREATE;
    reserved["TABLE"] = Token::TABLE;
    reserved["FILE"] = Token::FILE;
    reserved["select"] = Token::SELECT;
    reserved["from"] = Token::FROM;
    reserved["where"] = Token::WHERE;
    reserved["insert"] = Token::INSERT;
    reserved["delete"] = Token::DELETE_T;
    reserved["into"] = Token::INTO;
    reserved["set"] = Token::SET;
    reserved["values"] = Token::VALUES;
    reserved["and"] = Token::AND;
    reserved["or"] = Token::OR;
    reserved["not"] = Token::NOT;
    reserved["using"] = Token::USING;
    reserved["index"] = Token::INDEX;
    reserved["between"] = Token::BETWEEN;
    reserved["campo"] = Token::CAMPO;
    reserved["create"] = Token::CREATE;
    reserved["table"] = Token::TABLE;
    reserved["file"] = Token::FILE;
}

Token* Scanner::nextToken() {
    Token* token;
    char c;
    // consume whitespaces
    c = nextChar();
    while (c == ' ' || c == '\t' || c == '\n')
        c = nextChar();

    if(c=='#'){ // Comment in SQL, ignore all the next characters
        while (c != '\n')  // Be careful, maybe this is one error
            c = nextChar();
        c = nextChar();
    }
    if (c == '\0')
        return new Token(Token::END);

    startLexema();

    if (isdigit(c)) {
        c = nextChar();
        while (isdigit(c)) c = nextChar();

        if(c == '.'){
            c = nextChar();
            while (isdigit(c)) c = nextChar();
            rollBack();
            token = new Token(Token::FLOAT, getLexema());
        } else{
            rollBack();
            token = new Token(Token::NUM, getLexema());
        }
    } else if (isalpha(c)) {
        c = nextChar();
        while (isalpha(c) || isdigit(c)) c = nextChar();
        rollBack();

        string lex = getLexema();

        if(lex == "true" or lex == "false")
            token = new Token(Token::BOOL, getLexema());
        else{
            Token::Type ttype = checkReserved(lex); // Check if it is a reserved word.
            if (ttype != Token::ERR)
                token = new Token(ttype);
            else
                token = new Token(Token::CAMPO, getLexema());
        }

    } else if(c == '"'){
        c = nextChar();
        while (c != '"') {
            c = nextChar();
        }
        return new Token(Token::ID, getLexema());
    }
    else if (strchr("()*;,=<>", c)) {
        switch(c) {
            case '(': token = new Token(Token::LPAREN); break;
            case ')': token = new Token(Token::RPAREN); break;
            case '*': token = new Token(Token::CAMPO, getLexema()); break;
            case ';': token = new Token(Token::SEMICOLON); break;
            case ',': token = new Token(Token::COMMA); break;
            case '!': token = new Token(Token::NOT); break;
            case '=': token = new Token(Token::EQ); break;
            case '<':
                c = nextChar();
                if (c == '=') token = new Token(Token::LTEQ);
                else { rollBack(); token = new Token(Token::LT); }
                break;
            case '>':
                c = nextChar();
                if (c == '=') token = new Token(Token::GTEQ);
                else { rollBack(); token = new Token(Token::GT); }
                break;
            default: cout << "No deberia llegar aca" << endl;
        }
    } else {
        token = new Token(Token::ERR, getLexema());
    }
    return token;
}

Scanner::~Scanner() { }

char Scanner::nextChar() {
    int c = input[current];
    current++;
    return c;
}

void Scanner::rollBack() { // retract
    current--;
}

void Scanner::startLexema() {
    first = current-1;
    return;
}

string Scanner::getLexema() {
    string res = input.substr(first,current-first);
    size_t n = res.size();

    if (res.front() == '"' and res.back() == '"') {
        res = res.substr(1, res.length() - 2);
    }

    if (res[n-1] == ')'){
        res = res.substr(0, n-1);
        current--;
    }

    if (res[n-1] == '\n'){
        res = res.substr(0, n-1);
        current--;
    }

    return res;

}

Token::Type Scanner::checkReserved(string lexema) {
    std::unordered_map<std::string,Token::Type>::const_iterator it = reserved.find (lexema);
    if (it == reserved.end())
        return Token::ERR;
    else
        return it->second;
}

/* ***************** Parser ********************* */

class Parser {
private:
    Scanner* scanner;
    Token *currentToken, *previousToken;
    bool match(Token::Type ttype);
    bool check(Token::Type ttype);
    bool advance();
    bool isAtEnd();
    void parseStatementList(vector<Record>& records);
    void parseStatement(vector<Record>& records);
    void expect(Token::Type expectedTtype); // Check if the current token is the expected one by calling match and exit if it is not.
    vector<Record> parseSelectStatement();

    void parseCreateStatement();
    void parseInsertStatement();
    void parseDeleteStatement();
public:
    Parser(const string& input);
    void parse(vector<Record>& records);

    ~Parser();
};


Parser::Parser(const string& input) : currentToken(nullptr), previousToken(nullptr) {
    // Scanner scanner_in(input.c_str());

    scanner = new Scanner(input.c_str());
    scanner->print_input();

    // Token* tk = scanner->nextToken();
    // while (tk->type != Token::END) {
    //     cout << "next token " << tk << endl;
    //     delete tk;
    //     tk =  scanner->nextToken();
    // }
    // cout << "last token " << tk << endl;
    // delete tk;
}



Parser::~Parser(){
    if (currentToken){
        delete currentToken;
        currentToken = nullptr;
    }

    if (previousToken){
        delete previousToken;
        previousToken = nullptr;
    }

    if (scanner){
        delete scanner;
        scanner = nullptr;
    }
}


// match and consume next token
bool Parser::match(Token::Type ttype) {
    if (check(ttype)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(Token::Type ttype) {
    if (isAtEnd()) return false;
    return currentToken->type == ttype;
}

void Parser::expect(Token::Type expectedTtype) {
    Token temp = Token(expectedTtype);
    if (!match(expectedTtype)) {
        cout << "Error: unexpected token " << currentToken << " expected " << temp << endl;
        exit(0);
    }
}


bool Parser::advance() {
    if (!isAtEnd()) {
        Token* temp =currentToken;
        if (previousToken) delete previousToken;
        currentToken = scanner->nextToken();
        previousToken = temp;
        if (check(Token::ERR)) {
            cout << "Parse error, unrecognised character: " << currentToken->lexema << endl;
            exit(0);
        }
        return true;
    }
    return false;
}

bool Parser::isAtEnd() {
    return (currentToken->type == Token::END);
}


void Parser::parse(vector<Record>& records) {
    currentToken = scanner->nextToken();
    if (check(Token::ERR)) {
        cout << "Error en scanner - caracter invalido" << endl;
        exit(0);
    }

    parseStatementList(records);
    if (currentToken->type != Token::END)
        cout << "Esperaba fin-de-input, se encontro " << currentToken << endl;
    else
        cout << "Fin de ejecucion" << endl;

    return;
}





void Parser::parseStatementList(vector<Record>& records) {
    parseStatement(records);
    while(match(Token::SEMICOLON)) {
        parseStatement(records);
    }
}

void Parser::parseStatement(vector<Record>& records) {
    if (match(Token::SELECT)) {
        records = parseSelectStatement();
    } else if (match(Token::CREATE)) {
        parseCreateStatement();
    } else if (match(Token::INSERT)){
        parseInsertStatement();
    } else if (match(Token::DELETE_T)){
        parseDeleteStatement();
    } else {
        cout << "No se encontro ninguna sentencia" << endl;

    }
}

vector<Record> Parser::parseSelectStatement() {
    vector<Record> found_records;
    vector<string> campos;
    string tabla;

    expect(Token::CAMPO);

    campos.push_back(previousToken->lexema);

    while(match(Token::COMMA)) {
        expect(Token::CAMPO);
        campos.push_back(previousToken->lexema);
    }

    expect(Token::FROM);
    expect(Token::CAMPO);

    tabla = previousToken->lexema;
    // KEY = findKeyForTable(tabla);

    if(match(Token::USING)){
        expect(Token::CAMPO);
        if(currentToken->type == Token::END){
            if(previousToken->lexema == "SEQUENTIAL" or previousToken->lexema == "sequential"){
                // SequentialFile f(tabla);
                // found_records = f.inorder();
                cout << "SELECT exitoso en SEQUENTIAL." << endl;
            }else if(previousToken->lexema == "ISAM" or previousToken->lexema == "isam"){
                // AVLFile f(tabla);
                // found_records = f.inorder();
                cout << "SELECT exitoso en ISAM." << endl;
            }else if(previousToken->lexema == "HASH" or previousToken->lexema == "hash"){
                // DynamicHash f(tabla + ".bin", tabla + "index.bin", D, FB);
                // found_records = f.inorder();
                cout << "SELECT exitoso en HASH." << endl;
            } else {
                cout << "Indice no existente" << endl;
            }
        }
        return found_records;
    } else {
        // Consultas condicionadas
        expect(Token::WHERE);
        string campo;
        expect(Token::CAMPO);
        campo = previousToken->lexema;

        // expect({Token::GT, Token::GE, Token::LT, Token::LE, Token::EQ, Token::BETWEEN});
        if(match(Token::EQ) || match(Token::GT) || match(Token::GTEQ) || match(Token::LT) ||
           match(Token::LTEQ) || match(Token::EQ) || match(Token::EQ) )
        {
            string value;
            // expect({Token::NUM, Token::ID, Token::BOOL});
            expect(Token::NUM);

            value = previousToken->lexema;

            expect(Token::USING);
            expect(Token::CAMPO);

            if(currentToken->type == Token::END){
                if(previousToken->lexema == "SEQUENTIAL" || previousToken->lexema == "sequential"){
                    // SequentialFile f(tabla);
                    // found_records = f.search(stoi(value));
                    cout << "SELECT con WHERE en SEQUENTIAL exitoso" << endl;
                }else if(previousToken->lexema == "ISAM" || previousToken->lexema == "isam"){
                    // AVLFile f(tabla);
                    // found_records = f.search(stoi(value));
                    cout << "SELECT con WHERE en ISAM exitoso" << endl;
                }else if(previousToken->lexema == "HASH" || previousToken->lexema == "hash"){
                    // DynamicHash f(tabla + ".bin", tabla + "index.bin", D, FB);
                    // found_records.push_back(f.search(stoi(value)));
                    cout << "SELECT con WHERE en HASH exitoso" << endl;
                } else {
                    cout << "Indice no existente" << endl;
                }
            }
            return found_records;
        } else if(match(Token::BETWEEN)){
            string begin, end;
            expect(Token::LPAREN);
            expect(Token::NUM);
            begin = previousToken->lexema;
            expect(Token::COMMA);
            expect(Token::NUM);
            end = previousToken->lexema;
            expect(Token::RPAREN);
            expect(Token::USING);
            expect(Token::CAMPO);
            if(previousToken->lexema == "ISAM" or previousToken->lexema == "isam"){
                // AVLFile f(tabla);
                // found_records = f.rangeSearch(stoi(begin), stoi(end));
                cout << "SELECT con WHERE BETWEEN en ISAM exitoso" << endl;
            }else if(previousToken->lexema == "SEQUENTIAL" or previousToken->lexema == "sequential"){
                // SequentialFile f(tabla);
                // found_records = f.rangeSearch(stoi(begin), stoi(end));
                cout << "SELECT con WHERE BETWEEN en SEQUENTIAL exitoso" << endl;
            }

            return found_records;
        } else {
            cout << "Error: WHERE clause not found" << endl;
            return found_records;
        }


    }



}

void Parser::parseCreateStatement(){
    string tabla, filename, index, key;

    expect(Token::TABLE);
    expect(Token::CAMPO);

    tabla = previousToken->lexema;

    expect(Token::FROM);
    expect(Token::FILE);
    expect(Token::ID);

    filename = previousToken->lexema;
    expect(Token::USING);
    expect(Token::INDEX);
    expect(Token::CAMPO);


    index = previousToken->lexema;

    expect(Token::LPAREN);
    expect(Token::ID);

    key = previousToken->lexema;
    KEY = key;
    expect(Token::RPAREN);

    if(currentToken->type == Token::END){
        if(index == "ISAM" or index == "isam"){
            // savekey(key, tabla);
            // AVLFile indice(tabla);
            // readCSVFile<AVLFile>(filename, indice);
            // cout << "Tabla creada con nombre " << tabla << endl;
            cout << "CREATE TABLE USING ISAM exitoso" << endl;
        }else if(index == "SEQUENTIAL" or index == "sequential"){
            // savekey(key, tabla);
            // SequentialFile indice(tabla);
            // readCSVFile<SequentialFile>(filename, indice);
            // cout << "Tabla creada con nombre " << tabla << endl;
            cout << "CREATE TABLE USING SEQUENTIAL exitoso" << endl;
        }else if(index== "HASH" or index == "hash"){
            // savekey(key, tabla);
            // DynamicHash indice(tabla + ".bin", tabla + "index.bin", D, FB);
            // readCSVFile<DynamicHash>(filename, indice);
            // cout << "Tabla creada con nombre " << tabla << endl;
            cout << "CREATE TABLE USING HASH exitoso" << endl;
        }else{
            cout << "Indice no existente";
        }
    }
};

void Parser::parseInsertStatement(){
    Record record;
    string tabla;


    expect(Token::INTO);
    expect(Token::CAMPO);

    tabla = previousToken->lexema;

    expect(Token::VALUES);
    expect(Token::LPAREN);

    // The insert values are like the following:
    // record.id = row["id"].get<int64_t>();
    // strcpy(record.name, row["title"].get<string>().c_str());
    // record.punt_promedio = row["vote_average"].get<float>();
    // record.vote_count = row["vote_count"].get<int64_t>();
    // strcpy(record.release_date, row["release_date"].get<string>().c_str());
    // record.ganancia = row["revenue"].get<int64_t>();
    // record.tiempo = row["runtime"].get<int64_t>();
    // strcpy(record.lang, row["original_language"].get<string>().c_str());


    expect(Token::NUM);
    record.id = stoi(previousToken->lexema);

    expect(Token::COMMA);
    expect(Token::ID);
    strncpy(record.name, previousToken->lexema.c_str(), sizeof(record.name));

    expect(Token::COMMA);
    expect(Token::FLOAT);

    record.punt_promedio = stof(previousToken->lexema);

    expect(Token::COMMA);
    expect(Token::NUM);
    record.vote_count = stoi(previousToken->lexema);

    expect(Token::COMMA);
    expect(Token::ID);
    strncpy(record.release_date, previousToken->lexema.c_str(), sizeof(record.release_date));

    expect(Token::COMMA);
    expect(Token::NUM);
    record.ganancia = stoi(previousToken->lexema);

    expect(Token::COMMA);
    expect(Token::NUM);
    record.tiempo = stoi(previousToken->lexema);

    expect(Token::COMMA);
    expect(Token::ID); // If this doesnt work change to CAMPO
    strncpy(record.lang, previousToken->lexema.c_str(), sizeof(record.lang));

    expect(Token::RPAREN);
    expect(Token::USING);
    expect(Token::CAMPO);

    // KEY = findKeyForTable(tabla);
    if(currentToken->type == Token::END){
        if(previousToken->lexema == "ISAM" or previousToken->lexema == "isam"){
            // AVLFile f(tabla);
            // f.add(record);
            cout << "INSERT exitoso en ISAM" << endl;
        }else if(previousToken->lexema == "SEQUENTIAL" or previousToken->lexema == "sequential"){
            // SequentialFile f(tabla);
            // f.add(record);
            cout << "INSERT exitoso en SEQUENTIAL" << endl;
        }else if(previousToken->lexema == "HASH" or previousToken->lexema == "hash"){
            // DynamicHash f(tabla + ".bin", tabla + "index.bin", D, FB);
            // f.add(record);
            cout << "INSERT exitoso en HASH" << endl;
        }
    }

    record.showData_line();
};

void Parser::parseDeleteStatement(){
    string campo, tabla;

    expect(Token::FROM);
    expect(Token::CAMPO);
    tabla = previousToken->lexema;
    // KEY = findKeyForTable(tabla);
    expect(Token::WHERE);

    expect(Token::CAMPO);
    campo = previousToken->lexema;


    if(match(Token::EQ) || match(Token::GT) || match(Token::GTEQ) || match(Token::LT) ||
       match(Token::LTEQ) || match(Token::BETWEEN)){
        string value;

        expect(Token::NUM);
        value = previousToken->lexema;
        expect(Token::USING);
        expect(Token::CAMPO);
        if(currentToken->type == Token::END){
            if(previousToken->lexema == "SEQUENTIAL" or previousToken->lexema == "sequential"){
                // SequentialFile f(tabla);
                // f.remove(stoi(value));
                cout << "DELETE exitoso en SEQUENTIAL" << endl;
            }else if(previousToken->lexema == "ISAM" or previousToken->lexema == "isam"){
                // AVLFile f(tabla);
                // f.remove(stoi(value));
                cout << "DELETE exitoso en ISAM" << endl;
            }else if(previousToken->lexema == "HASH" or previousToken->lexema == "hash"){
                // DynamicHash f(tabla + ".bin", tabla + "index.bin", D, FB);
                // f.remove(stoi(value));
                cout << "DELETE exitoso en HASH" << endl;
            }
        }
    }
}


#endif //DB2_PROJECT1_PARSERSQL_H
