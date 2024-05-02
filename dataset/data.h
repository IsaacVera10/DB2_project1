#ifndef PROYECTO_1_DATA_H
#define PROYECTO_1_DATA_H

#include <iostream>
# include "csv.hpp"
#include <cstring>
#include <iomanip>

using namespace csv;
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

struct Record{
    long id;
    char name[71];
    float punt_promedio;
    long vote_count;
    char release_date[11];
    long ganancia;
    long tiempo;
    char imdb_id[11];
    char lang[3];

    Record() = default;
    Record(long id, const char* name, float punt_promedio, long vote_count, const char* release_date, long ganancia, long tiempo, const char* imdb_id, const char* lang) : id(id), punt_promedio(punt_promedio), vote_count(vote_count), ganancia(ganancia), tiempo(tiempo) {
        strcpy(this->name, name);
        strcpy(this->release_date, release_date);
        strcpy(this->imdb_id, imdb_id);
        strcpy(this->lang, lang);
    }

    void showData_line(){
        cout << setw(8) << left << id<< " | "
             << setw(74) << left << name<< " | "
             << setw(6) << left << punt_promedio<< " | "
             << setw(6) << left << vote_count<< " | "
             << setw(11) << left << release_date<< " | "
             << setw(11) << left << ganancia<< " | "
             << setw(6) << left << tiempo<< " | "
             << setw(11) << left << imdb_id<< " | "
             << setw(3) << left << lang << endl;
    }
};


vector<Record> generate_struct_records(int count = -1){
    vector<Record> records;
    try {
        csv::CSVFormat format;
        format.delimiter(',');    // Establecer la coma como delimitador
        format.quote('"');        // Establecer las comillas dobles como carácter de cita
        format.header_row(0);

        csv::CSVReader reader("./dataset/movie_dataset.csv", format);
        // cout<<"Headers:"<<endl;
        // cout<<"-----------------------------------------"<<endl;
        // for (const auto& header : reader.get_col_names()) {
        //     std::cout << header << " ";
        // }
        // std::cout << std::endl;

        // std::cout << "Datos:" << std::endl;
        // cout<<"-----------------------------------------"<<endl;
        // for (auto& row : reader) {
        //     for (const auto& field : row) {
        //         std::cout << field<< " - ";
        //     }
        //     std::cout << std::endl;
        // }


        for(auto& row : reader){
            if(count>0) count --;
            else if(count == 0) break;

            Record record;
            record.id = row["id"].get<long>();
            strcpy(record.name, row["title"].get<string>().c_str());
            record.punt_promedio = row["vote_average"].get<float>();
            record.vote_count = row["vote_count"].get<long>();
            strcpy(record.release_date, row["release_date"].get<string>().c_str());
            record.ganancia = row["revenue"].get<long>();
            record.tiempo = row["runtime"].get<long>();
            strcpy(record.imdb_id, row["imdb_id"].get<string>().c_str());
            strcpy(record.lang, row["original_language"].get<string>().c_str());

            records.push_back(record);
            
        }
    }
    catch(exception& e){
        cerr<<"\nError: "<<e.what()<<endl;
    }

    return records;

}


#endif //PROYECTO_1_DATA_H