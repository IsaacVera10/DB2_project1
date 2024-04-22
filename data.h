#ifndef PROYECTO_1_DATA_H
#define PROYECTO_1_DATA_H

#include <iostream>
#include <cstring>
#include <iomanip>
using namespace std;

struct Record_SFile{
    string Codigo;
    int Ciclo=0;
    float Mensualidad=0.0;
    string Observaciones;
    size_t punt_nextPosFisica = 0; //8 bits
    bool punt_next_is_In_Data = false; // 1bits

    string Key_Value = Codigo;

    Record_SFile() = default;
    Record_SFile(string Codigo, int Ciclo, float Mensualidad, string Observaciones) : Codigo(std::move(Codigo)), Ciclo(Ciclo), Mensualidad(Mensualidad), Observaciones(std::move(Observaciones)) {}

    static size_t string_with_delimiter_size(const string& str){
        return sizeof(size_t)+str.size();
    }

    size_t size_of(){
        return sizeof(size_t) + 
               string_with_delimiter_size(Codigo) +
               sizeof(Ciclo) +
               sizeof(Mensualidad) +
               string_with_delimiter_size(Observaciones) +
               sizeof(punt_nextPosFisica) +
               sizeof(punt_next_is_In_Data);
    }

    template<typename T>
    static void concat(char*& buffer, const T& value) {
        memcpy(buffer, &value, sizeof(value));
        buffer += sizeof(value);
    }

    static void concat(char*& buffer, const char* str) {
        size_t len = strlen(str);
        memcpy(buffer, &len, sizeof(len));
        buffer += sizeof(len);
        memcpy(buffer, str, len);
        buffer += len;
    }

    char* empaquetar() {
        size_t bufferSize = size_of();
        char* buffer = new char[bufferSize + sizeof(bufferSize)];
        char* current = buffer;

        concat(current, bufferSize);
        concat(current, Codigo.c_str());
        concat(current, Ciclo);
        concat(current, Mensualidad);
        concat(current, Observaciones.c_str());
        concat(current, punt_nextPosFisica);
        concat(current, punt_next_is_In_Data);

        return buffer;
    }

    template<typename F>
    static void extraer(const char*& buffer, F& value) {
        memcpy(&value, buffer, sizeof(value));
        buffer += sizeof(value);
    }
    
    static void extraer(const char*& buffer, string& str) {
        size_t len;
        memcpy(&len, buffer, sizeof(len));
        buffer += sizeof(len);
        str.assign(buffer, len);
        buffer += len;
    }

    void desempaquetar(const char* buffer) {
        const char* current = buffer;
        current += sizeof(size_t);

        extraer(current, Codigo);
        extraer(current, Ciclo);
        extraer(current, Mensualidad);
        extraer(current, Observaciones);
        extraer(current, punt_nextPosFisica);
        extraer(current, punt_next_is_In_Data);

        Key_Value = Codigo;
    }

    void showData(){
        cout<<"Tamanio: "<<size_of()<<endl;
        cout<<"Codigo: "<<Codigo<<endl;
        cout<<"Ciclo: "<<Ciclo<<endl;
        cout<<"Mensualidad: "<<Mensualidad<<endl;
        cout<<"Observaciones: "<<Observaciones<<endl;
        if(punt_nextPosFisica == -1) cout<<"posicion fisica de next: "<<(int)punt_nextPosFisica<<endl;
        else cout<<"posicion fisica de next: "<<punt_nextPosFisica<<endl;
        cout<<"next esta en data?: "<<punt_next_is_In_Data<<endl;
    }
    void showData_line(){
        cout << setw(5) << left << size_of()<< " | "
             << setw(11) << left << Codigo<< " | "
             << setw(3) << left << Ciclo<< " | "
             << setw(8) << left << Mensualidad<< " | "
             << setw(11) << left << Observaciones<< " | "
             << setw(5) << left << punt_nextPosFisica<< " | "
             << setw(5) << left << punt_next_is_In_Data << endl;
    }
};

struct Record_AVLFile{
    string Codigo;
    int Ciclo=0;
    float Mensualidad=0.0;
    string Observaciones;
    long pos_left = -1, pos_right = -1;
    long height = 0;

    string Key_Value = Codigo;

    Record_AVLFile() = default;
    Record_AVLFile(string Codigo, int Ciclo, float Mensualidad, string Observaciones) : Codigo(std::move(Codigo)), Ciclo(Ciclo), Mensualidad(Mensualidad), Observaciones(std::move(Observaciones)) {}

    static size_t string_with_delimiter_size(const string& str){
        return sizeof(size_t)+str.size();
    }

    size_t size_of(){
        return sizeof(size_t) + 
               string_with_delimiter_size(Codigo) +
               sizeof(Ciclo) +
               sizeof(Mensualidad) +
               string_with_delimiter_size(Observaciones) +
                sizeof(pos_left) +
                sizeof(pos_right) +
                sizeof(height);
    }

    template<typename T>
    static void concat(char*& buffer, const T& value) {
        memcpy(buffer, &value, sizeof(value));
        buffer += sizeof(value);
    }

    static void concat(char*& buffer, const char* str) {
        size_t len = strlen(str);
        memcpy(buffer, &len, sizeof(len));
        buffer += sizeof(len);
        memcpy(buffer, str, len);
        buffer += len;
    }

    char* empaquetar() {
        size_t bufferSize = size_of();
        char* buffer = new char[bufferSize + sizeof(bufferSize)];
        char* current = buffer;

        concat(current, bufferSize);
        concat(current, Codigo.c_str());
        concat(current, Ciclo);
        concat(current, Mensualidad);
        concat(current, Observaciones.c_str());
        concat(current, pos_left);
        concat(current, pos_right);
        concat(current, height);

        return buffer;
    }

    template<typename F>
    static void extraer(const char*& buffer, F& value) {
        memcpy(&value, buffer, sizeof(value));
        buffer += sizeof(value);
    }
    
    static void extraer(const char*& buffer, string& str) {
        size_t len;
        memcpy(&len, buffer, sizeof(len));
        buffer += sizeof(len);
        str.assign(buffer, len);
        buffer += len;
    }

    void desempaquetar(const char* buffer) {
        const char* current = buffer;
        current += sizeof(size_t);

        extraer(current, Codigo);
        extraer(current, Ciclo);
        extraer(current, Mensualidad);
        extraer(current, Observaciones);
        extraer(current, pos_left);
        extraer(current, pos_right);
        extraer(current, height);

        Key_Value = Codigo;
    }

    void showData(){
        cout<<"Tamanio: "<<size_of()<<endl;
        cout<<"Codigo: "<<Codigo<<endl;
        cout<<"Ciclo: "<<Ciclo<<endl;
        cout<<"Mensualidad: "<<Mensualidad<<endl;
        cout<<"Observaciones: "<<Observaciones<<endl;
        cout<<"left: "<<left<<endl;
        cout<<"right: "<<right<<endl;
        cout<<"height: "<<height<<endl;
    }
    void showData_line(){
        cout << setw(5) << left << size_of()<< " | "
             << setw(11) << left << Codigo<< " | "
             << setw(3) << left << Ciclo<< " | "
             << setw(8) << left << Mensualidad<< " | "
             << setw(11) << left << Observaciones<< " | "
             << setw(5) << left << pos_left << " | "
             << setw(5) << left << pos_right << " | "
             << setw(5) << left << height << endl;
    }
};

#endif //PROYECTO_1_DATA_H