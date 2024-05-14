#ifndef PROYECTO_1_DATA_H
#define PROYECTO_1_DATA_H

#include <iostream>
# include "csv.hpp"
#include <cstring>
#include <iomanip>

using namespace csv;
using namespace std;

struct Record_SFile{
    int64_t id;
    char name[71];
    float punt_promedio;
    int64_t vote_count;
    char release_date[11];
    int64_t ganancia;
    int64_t tiempo;
    char lang[3];
    int64_t punt_nextPosLogic = -1; //8 bits
    bool punt_next_is_In_Data = false; // 1bits

    int64_t key_value(){
        return id;
    }

    Record_SFile() = default;
    Record_SFile(int64_t id, const char* name, float punt_promedio, int64_t vote_count, const char* release_date, int64_t ganancia, int64_t tiempo, const char* lang){
        this->id = id;
        strcpy(this->name, name);
        this->punt_promedio = punt_promedio;
        this->vote_count = vote_count;
        strcpy(this->release_date, release_date);
        this->ganancia = ganancia;
        this->tiempo = tiempo;
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
             << setw(3) << left << lang << " | "
             << setw(8) << left << punt_nextPosLogic << " | "
             << setw(8) << left << punt_next_is_In_Data << endl;
    }
};


vector<Record_SFile> generate_struct_records(string route_file, int64_t count = -1){
    vector<Record_SFile> records;
    Record_SFile record;
    try {
        csv::CSVFormat format;
        format.delimiter(',');
        format.quote('"');
        format.header_row(0);

        csv::CSVReader reader("./"+route_file, format);
        for(auto& row : reader){
            if(count>0) count --;
            else if(count == 0) break;

            
            record.id = row["id"].get<int64_t>();
            strcpy(record.name, row["title"].get<string>().c_str());
            record.punt_promedio = row["vote_average"].get<float>();
            record.vote_count = row["vote_count"].get<int64_t>();
            strcpy(record.release_date, row["release_date"].get<string>().c_str());
            record.ganancia = row["revenue"].get<int64_t>();
            record.tiempo = row["runtime"].get<int64_t>();
            strcpy(record.lang, row["original_language"].get<string>().c_str());

            records.push_back(record);
            
        }
    }
    catch(exception& e){
        cerr<<"\nError: "<<e.what()<<endl;
    }

    return records;

}

void records_csv_to_bin(string route_file, int64_t count = -1){
    Record_SFile record;
    try {
        csv::CSVFormat format;
        format.delimiter(',');
        format.quote('"');
        format.header_row(0);

        csv::CSVReader reader("./"+route_file, format);

        fstream file("./dataset/movie_dataset.bin", ios::binary | ios::out | ios::trunc);
        if(!file.is_open()) throw runtime_error("Error al abrir el archivo");

        for(auto& row : reader){
            if(count>0) count --;
            else if(count == 0) break;
            
            record.id = row["id"].get<int64_t>();
            strcpy(record.name, row["title"].get<string>().c_str());
            record.punt_promedio = row["vote_average"].get<float>();
            record.vote_count = row["vote_count"].get<int64_t>();
            strcpy(record.release_date, row["release_date"].get<string>().c_str());
            record.ganancia = row["revenue"].get<int64_t>();
            record.tiempo = row["runtime"].get<int64_t>();
            strcpy(record.lang, row["original_language"].get<string>().c_str());
            file.write(reinterpret_cast<char*>(&record), sizeof(Record_SFile));
        }
        file.close();
    }
    catch(exception& e){
        cerr<<"\nError: "<<e.what()<<endl;
    }

}
#endif //PROYECTO_1_DATA_H