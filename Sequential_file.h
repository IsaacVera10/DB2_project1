#ifndef PROYECTO_1_SEQUENTIALFILE_H
#define PROYECTO_1_SEQUENTIALFILE_H
#include <fstream>
#include <vector>
#include <cmath>
#include "data.h"
#define T string

using namespace std;

namespace var_temps_SF{
    int n_data = 0, n_aux=0;
    size_t punt_pos = 0;
    bool punt_is_in_data = false;
    Record rec_temp;
}

class Sequential_File {
private:
    string filename;
public:
    explicit Sequential_File(string filename) : filename(std::move(filename)) {
        fstream file1("files/" + this->filename, ios::binary | ios::out | ios::trunc);
        fstream file2("files/aux.bin", ios::binary | ios::out | ios::trunc);
        if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
        if (!file2.is_open()) throw runtime_error("No se pudo abrir el archivo metadata.dat");

        file1.write(reinterpret_cast<const char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
        file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
        file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
        
        file2.write(reinterpret_cast<const char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));

        file1.close();
        file2.close();
    }

    void add(Record record){

    }

    void reBuild(fstream& file1, fstream& file2, fstream& file3){//Reconstruye el archivo data, pero también cierra los archivos para remover y renombrar.
    
    }
    
    Record_SFile search(T key){
        Record_SFile record;

        ifstream file("files/" + this->filename, ios::binary | ios::in);
        if (!file.is_open()) throw runtime_error("No se pudo abrir el archivo " + filename);

        file.seekg(0, ios::beg);

        file.read(reinterpret_cast<char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));

        if(var_temps_SF::n_data==0){
            throw runtime_error("No se encontro el registro con la llave " + key);
        }else if(var_temps_SF::n_data==1){
            file.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
            file.read(reinterpret_cast<char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
            file.seekg(var_temps_SF::punt_pos, ios::beg);
            file.read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record));
            file.seekg(-sizeof(var_temps_SF::size_record), ios::cur);

            //Desempaquetamos el registro
            char* buffer = new char[var_temps_SF::size_record];
            file.read(buffer, var_temps_SF::size_record);
            var_temps_SF::rec_temp.desempaquetar(buffer);
            delete[] buffer;

            if(var_temps_SF::rec_temp.Key_Value == key){
                cout<<"-------------------------------------------------"<<endl;
                cout<<"Registro encontrado "<<endl;
                // var_temps_SF::rec_temp.showData();
                return var_temps_SF::rec_temp;
            }else throw runtime_error("No se encontro el registro con la llave " + key);
            
        }else if(var_temps_SF::n_data==2){
            file.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
            file.read(reinterpret_cast<char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
            file.seekg(var_temps_SF::punt_pos, ios::beg);
            file.read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record));
            file.seekg(-sizeof(var_temps_SF::size_record), ios::cur);

            //Desempaquetamos el registro
            char* buffer = new char[var_temps_SF::size_record];
            file.read(buffer, var_temps_SF::size_record);
            var_temps_SF::rec_temp.desempaquetar(buffer);
            delete[] buffer;

            if(var_temps_SF::rec_temp.Key_Value == key){
                cout<<"-------------------------------------------------"<<endl;
                cout<<"Registro encontrado: "<<endl;
                // var_temps_SF::rec_temp.showData();
                return var_temps_SF::rec_temp;
            }else{
                file.seekg(-sizeof(var_temps_SF::punt_pos)-sizeof(var_temps_SF::punt_is_in_data), ios::cur);
                file.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
                file.read(reinterpret_cast<char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
                file.seekg(var_temps_SF::punt_pos, ios::beg);
                file.read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record));
                file.seekg(-sizeof(var_temps_SF::size_record), ios::cur);

                //Desempaquetamos el registro
                char* buffer = new char[var_temps_SF::size_record];
                file.read(buffer, var_temps_SF::size_record);
                var_temps_SF::rec_temp.desempaquetar(buffer);
                delete[] buffer;

                if(var_temps_SF::rec_temp.Key_Value == key){
                    cout<<"-------------------------------------------------"<<endl;
                    cout<<"Registro encontrado: "<<endl;
                    // var_temps_SF::rec_temp.showData();
                    return var_temps_SF::rec_temp;
                }else{
                    throw runtime_error("No se encontro el registro con la llave " + key);
                }
            }
        }

        ifstream positions("files/positions_data.bin", ios::binary | ios::in);
        if (!positions.is_open()) throw runtime_error("No se pudo abrir el archivo positions.bin");

        int l = 0, u = var_temps_SF::n_data-1, m = 0;

        while(u>=l){
            m = ((l+u)/2);
            positions.seekg(m*sizeof(size_t), ios::beg);
            positions.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));

            file.seekg(var_temps_SF::punt_pos, ios::beg);
            file.read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record));
            file.seekg(-sizeof(var_temps_SF::size_record), ios::cur);

            //Desempaquetamos el registro
            char* buffer = new char[var_temps_SF::size_record];
            file.read(buffer, var_temps_SF::size_record);
            var_temps_SF::rec_temp.desempaquetar(buffer);
            delete[] buffer;

            if(var_temps_SF::rec_temp.Key_Value < key){
                l = m+1;
            }else if(var_temps_SF::rec_temp.Key_Value > key){
                u = m-1;
            }else{
                cout<<"-------------------------------------------------"<<endl;
                cout<<"Registro encontrado: "<<endl;
                // var_temps_SF::rec_temp.showData();
                return var_temps_SF::rec_temp;
            } 
        }

        ifstream aux("files/aux.bin", ios::binary | ios::in);
        if (!aux.is_open()) throw runtime_error("No se pudo abrir el archivo aux.bin");

        aux.seekg(0, ios::beg);
        aux.read(reinterpret_cast<char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux)); 
        if(var_temps_SF::n_aux!=0){
            //Recorremos linealmente el archivo aux.bin
            while(aux.read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record))){
                aux.seekg(-sizeof(var_temps_SF::size_record), ios::cur);
                //Desempaquetamos el registro
                char* buffer = new char[var_temps_SF::size_record];
                aux.read(buffer, var_temps_SF::size_record);
                var_temps_SF::rec_temp.desempaquetar(buffer);
                delete[] buffer;

                if(var_temps_SF::rec_temp.Key_Value == key){
                    cout<<"-------------------------------------------------"<<endl;
                    cout<<"Registro encontrado: "<<endl;
                    // var_temps_SF::rec_temp.showData();
                    return var_temps_SF::rec_temp;
                }
            }
        }

        throw runtime_error("No se encontro el registro con la llave " + key);
        
        if(file.is_open()) file.close();
        if(positions.is_open()) positions.close();
        if(aux.is_open()) aux.close();

        return record;   
    }

    vector<Record_SFile> range_search(T begin_key, T end_key){
        if(begin_key>end_key) throw runtime_error("La llave de inicio debe ser menor a la llave final");

        vector<Record_SFile> records;

        fstream file1("files/" + this->filename, ios::binary | ios::in);
        fstream file2("files/aux.bin", ios::binary | ios::in | ios::out);
        fstream file3("files/positions_data.bin", ios::binary | ios::in | ios::out);
        if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
        if (!file2.is_open()) throw runtime_error("No se pudo abrir el archivo metadata.dat");
        if (!file3.is_open()) throw runtime_error("No se pudo abrir el archivo positions.bin");
        
        file2.read(reinterpret_cast<char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));
        
        if(var_temps_SF::n_aux!=0) reBuild(file1, file2, file3);
        search(begin_key);
        auto init = var_temps_SF::punt_pos;
        cout<<init<<endl;
        search(end_key);
        auto finish = var_temps_SF::punt_pos;
        cout<<finish<<endl;
        
        
        if(!file1.is_open()){
            file1.open("files/" + this->filename, ios::binary | ios::in);
            if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
        }

        file1.seekg(init, ios::beg);

        while(init<=finish){
            file1.read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record));
            file1.seekg(-sizeof(var_temps_SF::size_record), ios::cur);
            
            char* buffer = new char[var_temps_SF::size_record];
            file1.read(buffer,var_temps_SF::size_record);
            var_temps_SF::rec_temp.desempaquetar(buffer);

            records.push_back(var_temps_SF::rec_temp);
            init = file1.tellp();
        }

        if(file1.is_open()) file1.close();
        if(file2.is_open()) file2.close();
        if(file3.is_open()) file3.close();

        return records;
    }

    bool remove_record(T key){
        fstream file1("files/" + this->filename, ios::binary | ios::in | ios::out);
        fstream file2("files/aux.bin", ios::binary | ios::in | ios::out);
        fstream file3("files/positions_data.bin", ios::binary | ios::in | ios::out);
        if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
        if (!file2.is_open()) throw runtime_error("No se pudo abrir el archivo metadata.dat");
        if (!file3.is_open()) throw runtime_error("No se pudo abrir el archivo positions.bin");
        
        file2.read(reinterpret_cast<char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));
        
        if(var_temps_SF::n_aux!=0) reBuild(file1, file2, file3);

        var_temps_SF::punt_pos = 0;

        search(key);

        if(var_temps_SF::punt_pos==0) return false;

        if(!file1.is_open()){
            file1.open("files/" + this->filename, ios::binary | ios::in);
            if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
        }
        file1.seekg(var_temps_SF::punt_pos, ios::beg);
        
        file1.read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record));
        file1.seekg(-sizeof(var_temps_SF::size_record), ios::cur);
        
        char* buffer = new char[var_temps_SF::size_record];
        file1.read(buffer,var_temps_SF::size_record);
        var_temps_SF::rec_temp.desempaquetar(buffer);

        var_temps_SF::recordBefore_pos = -1;
        var_temps_SF::recordBefore_is_in_data = false;

        var_temps_SF::punt_pos = var_temps_SF::rec_temp.punt_nextPosFisica;
        var_temps_SF::punt_is_in_data = var_temps_SF::rec_temp.punt_next_is_In_Data;

        //Actualizo el puntero del record a eliminar
        file1.seekg(-sizeof(var_temps_SF::recordBefore_pos)-sizeof(var_temps_SF::recordBefore_is_in_data), ios::cur);

        file1.write(reinterpret_cast<const char*>(&var_temps_SF::recordBefore_pos), sizeof(var_temps_SF::recordBefore_pos));
        file1.write(reinterpret_cast<const char*>(&var_temps_SF::recordBefore_is_in_data), sizeof(var_temps_SF::recordBefore_is_in_data));

        file1.seekg(-var_temps_SF::size_record, ios::cur);

        //Actualizo el puntero del record anterior
        file1.seekg(-sizeof(var_temps_SF::recordBefore_pos)-sizeof(var_temps_SF::recordBefore_is_in_data),ios::cur);
        file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
        file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));

        //Reescribimos n_data de data.bin
        file1.seekg(0,ios::beg);
        file1.read(reinterpret_cast<char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
        var_temps_SF::n_data--;
        file1.seekp(0,ios::beg);
        file1.write(reinterpret_cast<const char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));

        reBuild(file1,file2,file3);

        file1.close();
        file2.close();
        file3.close();

        return true;
    }

};

#endif //PROYECTO_1_SEQUENTIALFILE_H