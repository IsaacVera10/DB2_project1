#ifndef PROYECTO_1_SEQUENTIALFILE_H
#define PROYECTO_1_SEQUENTIALFILE_H
#include <fstream>
#include <vector>
#include <cmath>
#include "dataset/data.h"
#define T string

using namespace std;


//Trabajaremos con posiciones lógicas

namespace var_temps_SF{
    int64_t n_data = 0, n_aux=0, u_before, l_after, pos_found;
    int64_t punt_pos = 0;
    bool punt_is_in_data = false, u_before_is_in_data = true, found_in_data = false;
    Record_SFile rec_temp, rec_found;
}

class Sequential_File {
private:
    string filename;

    int64_t get_pos_logical(int64_t pos_fisica, bool is_in_data){
        if(is_in_data){
            return (pos_fisica - (sizeof(var_temps_SF::n_data)+ sizeof(var_temps_SF::punt_pos) + sizeof(var_temps_SF::punt_is_in_data))) / sizeof(Record_SFile);
        }else{
            return (pos_fisica - sizeof(var_temps_SF::n_aux)) / sizeof(Record_SFile);
        }
    }

    int64_t get_pos_fisica(int64_t pos_logical, bool is_in_data){
        if(is_in_data){
            return (pos_logical * sizeof(Record_SFile)) + (sizeof(var_temps_SF::n_data)+ sizeof(var_temps_SF::punt_pos) + sizeof(var_temps_SF::punt_is_in_data));
        }else{
            return (pos_logical * sizeof(Record_SFile)) + sizeof(var_temps_SF::n_aux);
        }
    }

    bool binary_search(int64_t key){//Busqueda binaria
        bool found = false;
        ifstream file("files/" + this->filename, ios::binary | ios::in);
        if (!file.is_open()) throw runtime_error("No se pudo abrir el archivo " + filename);

        file.seekg(0, ios::beg);

        file.read(reinterpret_cast<char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));

        int64_t l=0, u=var_temps_SF::n_data-1, m=0;

        while(l<=u){
            m = (l+u)/2;
            file.seekg(sizeof(var_temps_SF::n_data)+ sizeof(var_temps_SF::punt_pos) + sizeof(var_temps_SF::punt_is_in_data) + m*sizeof(Record_SFile), ios::beg);
            file.read(reinterpret_cast<char*>(&var_temps_SF::rec_found), sizeof(var_temps_SF::rec_found));
            if(var_temps_SF::rec_found.key_value() < key){
                l = m+1;
            }else if(var_temps_SF::rec_found.key_value() > key){
                u = m-1;
            }else{
                var_temps_SF::pos_found = m;
                var_temps_SF::found_in_data = true;
                found = true;
                break;
            }
            found = false;
        }

        var_temps_SF::u_before = u;
        var_temps_SF::u_before_is_in_data = true;
        var_temps_SF::l_after = l;
        var_temps_SF::punt_is_in_data = true;

        file.close();

        return found;
    }

    void get_u_before(int64_t record, fstream& file1, fstream& file2, bool add=0){//O(log(n)) + O(log(n))
        //1. Verificamos si el registro anterior está en data.bin o ya existe el record a insertar
        if(binary_search(record)){ //O(log(n)) -> Busqueda binaria
                if(add)throw runtime_error("El registro ya existe");
        }

        // if(record.key_value()==14160){
        //     cout<<"in data: "<<endl;
        //     cout<<"u_before: "<<var_temps_SF::u_before<<" | u_before_is_in_data: "<<var_temps_SF::u_before_is_in_data<<endl;
        //     cout<<"Record_insert: ";
        //     record.showData_line();
        // }

        if(var_temps_SF::u_before!=-1){//Si el registro a insertar no es el primero
            file1.seekp(get_pos_fisica(var_temps_SF::u_before, true), ios::beg);
            file1.read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
            var_temps_SF::punt_pos = var_temps_SF::rec_temp.punt_nextPosLogic;
            var_temps_SF::punt_is_in_data = var_temps_SF::rec_temp.punt_next_is_In_Data;
        }

        // if(record.key_value()==14160)cout<<"punt_pos: "<<var_temps_SF::punt_pos<<" | punt_is_in_data: "<<var_temps_SF::punt_is_in_data<<endl<<endl;
        //2. Verificamos si el puntero del record anterior se dirige a un registro en aux.bin
        if(var_temps_SF::punt_is_in_data == false && var_temps_SF::punt_pos!=-1){
            while(record > var_temps_SF::rec_temp.key_value() && var_temps_SF::punt_pos!=-1 && var_temps_SF::punt_is_in_data==false){
                file2.seekg(get_pos_fisica(var_temps_SF::punt_pos, false), ios::beg);
                file2.read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));

                if(record > var_temps_SF::rec_temp.key_value()){
                    var_temps_SF::u_before = var_temps_SF::punt_pos;
                    var_temps_SF::u_before_is_in_data = var_temps_SF::punt_is_in_data;
                    var_temps_SF::punt_pos = var_temps_SF::rec_temp.punt_nextPosLogic;
                    var_temps_SF::punt_is_in_data = var_temps_SF::rec_temp.punt_next_is_In_Data;
                }
            }
        }
    }

public:
    explicit Sequential_File(string filename, bool activate_trunc=0) : filename(std::move(filename)) {
        fstream file1;
        fstream file2;
        if(activate_trunc){
            file1.open("files/" + this->filename, ios::binary | ios::out | ios::trunc);
            file2.open("files/aux_sf.bin", ios::binary | ios::out | ios::trunc);
            if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
            if (!file2.is_open()) throw runtime_error("No se pudo abrir el archivo metadata.dat");

            file1.write(reinterpret_cast<const char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
            file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
            file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
            
            file2.write(reinterpret_cast<const char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));
        }else{
            file1.open("files/" + this->filename, ios::binary | ios::out | ios::app);
            file2.open("files/aux_sf.bin", ios::binary | ios::out | ios::app);
            if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
            if (!file2.is_open()) throw runtime_error("No se pudo abrir el archivo metadata.dat");
        }

        file1.close();
        file2.close();
    }

    void add(Record_SFile record){
        fstream file1("files/" + this->filename, ios::binary | ios::in | ios::out);
        fstream file2("files/aux_sf.bin", ios::binary | ios::in | ios::out);
        if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
        if (!file2.is_open()) throw runtime_error("No se pudo abrir el archivo metadata.dat");
        
        file1.seekg(0, ios::beg);
        file2.seekg(0, ios::beg);

        //Seteo los valores de las variables temporales - data.bin
        file1.read(reinterpret_cast<char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
        file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
        file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
        
        //Seteo los valores de las variables temporales - aux.bin
        file2.read(reinterpret_cast<char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));

        //Se insertará directamente en el archivo data.bin hasta que haya 2 registros
        if(var_temps_SF::n_data==0){
            file1.seekp(0, ios::end);
            
            //Actualizamos la metadata
            var_temps_SF::n_data++;
            var_temps_SF::punt_pos = get_pos_logical(file1.tellp(), true);
            var_temps_SF::punt_is_in_data = true;

            //Escribimos el registro
            file1.write(reinterpret_cast<const char*>(&record), sizeof(record));

            //Actualizamos la metadata
            file1.seekp(0, ios::beg);
            file1.write(reinterpret_cast<const char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
            file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
            file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
        }else if(var_temps_SF::n_data==1){
            //Leemos el record que está en data.bin
            file1.read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));

            if(record.key_value() < var_temps_SF::rec_temp.key_value()){//Si el registro a insertar es menor al primero, se inserta antes de este
                //Actualizamos la metadata del record a insertar
                record.punt_nextPosLogic = get_pos_logical(file1.tellp(), true);
                record.punt_next_is_In_Data = true;
                
                //Escribimos el registro
                file1.seekp(get_pos_fisica(var_temps_SF::punt_pos, true), ios::beg);
                file1.write(reinterpret_cast<const char*>(&record), sizeof(record));

                //Re-escribimos el record que estaba en data.bin
                file1.write(reinterpret_cast<const char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));

            }else if(record.key_value() > var_temps_SF::rec_temp.key_value()){//Si el registro a insertar es mayor al primero, se inserta después de este
                var_temps_SF::rec_temp.punt_nextPosLogic = get_pos_logical(file1.tellp(), true);
                var_temps_SF::rec_temp.punt_next_is_In_Data = true;

                file1.seekp(get_pos_fisica(var_temps_SF::punt_pos, true), ios::beg);
                file1.write(reinterpret_cast<const char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));

                file1.write(reinterpret_cast<const char*>(&record), sizeof(record));
            }else{
                throw runtime_error("El registro ya existe");
            }
            //Actualizamos la metadata
            var_temps_SF::n_data++;
            file1.seekp(0, ios::beg);
            file1.write(reinterpret_cast<const char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
        }else{
            //Obtenemos el registro anterior al que se insertará
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            get_u_before(record.key_value(), file1, file2, true);
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // cout<<"rec_before: ";
            // var_temps_SF::rec_temp.showData_line();
            //Actualizamos la metadata del record a insertar y escribimos el registro en file2
            
            record.punt_nextPosLogic = var_temps_SF::punt_pos;
            record.punt_next_is_In_Data = var_temps_SF::punt_is_in_data;

            
            //Actualizamos la metadata del record anterior al que se insertará
            if(var_temps_SF::u_before!=-1){//Si el registro a insertar no es el primero
                if(var_temps_SF::u_before_is_in_data==true){
                    file1.seekp(get_pos_fisica(var_temps_SF::u_before, true), ios::beg);
                    file1.read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
                    
                    file2.seekp(0, ios::end);
                    var_temps_SF::rec_temp.punt_nextPosLogic = get_pos_logical(file2.tellp(), false);
                    var_temps_SF::rec_temp.punt_next_is_In_Data = false;
                    //Escribimos el record que estaba en data.bin
                    file1.seekp(get_pos_fisica(var_temps_SF::u_before, true), ios::beg);
                    file1.write(reinterpret_cast<const char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
                }else{
                    file2.seekp(get_pos_fisica(var_temps_SF::u_before, false), ios::beg);
                    file2.read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));

                    file2.seekp(0, ios::end);
                    var_temps_SF::rec_temp.punt_nextPosLogic = get_pos_logical(file2.tellp(), false);
                    var_temps_SF::rec_temp.punt_next_is_In_Data = false;

                    //Escribimos el record que estaba en aux.bin
                    file2.seekp(get_pos_fisica(var_temps_SF::u_before, false), ios::beg);
                    file2.write(reinterpret_cast<const char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
                }
            }else{
                file2.seekp(0, ios::end);
                var_temps_SF::punt_pos = get_pos_logical(file2.tellp(), false);
                var_temps_SF::punt_is_in_data = false;
                file1.seekp(sizeof(var_temps_SF::n_data), ios::beg);
                file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
                file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
            }

            file2.seekp(0, ios::end);
            file2.write(reinterpret_cast<const char*>(&record), sizeof(record));      

            //Actualizamos la metadata
            var_temps_SF::n_aux++;
            file2.seekp(0, ios::beg);
            file2.write(reinterpret_cast<const char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));

            if(int64_t(log2(var_temps_SF::n_data))==var_temps_SF::n_aux){//Si hay 2 registros en aux.bin, se hace el merge
                reBuild(file1, file2);
            }
        }
        
        if(file1.is_open()) file1.close();
        if(file2.is_open()) file2.close();

        var_temps_SF::punt_pos = 0;
        var_temps_SF::punt_is_in_data = false;
    }

    void reBuild(fstream& file1, fstream& file2){//Reconstruye el archivo data, pero también cierra los archivos para remover y renombrar.
        fstream file_temp("files/temp.bin", ios::binary | ios::trunc | ios::out);
        if (!file_temp.is_open()) throw runtime_error("No se pudo abrir el archivo temp.bin");

        file1.seekg(0, ios::beg);
        file1.read(reinterpret_cast<char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
        file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
        file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));

        file2.seekg(0, ios::beg);
        file2.read(reinterpret_cast<char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));

        //Escribimos la metadata cabezera en temp.bin
        file_temp.seekg(0, ios::beg);

        int64_t n_temp = var_temps_SF::n_data + var_temps_SF::n_aux;
        file_temp.write(reinterpret_cast<const char*>(&n_temp), sizeof(n_temp));

        int64_t punt_temp = 0;
        file_temp.write(reinterpret_cast<const char*>(&punt_temp), sizeof(punt_temp));

        bool punt_is_in_data_temp = true;
        file_temp.write(reinterpret_cast<const char*>(&punt_is_in_data_temp), sizeof(punt_is_in_data_temp));

        //-----------------------------------------------------------------------------------------
        //Reconstruyendo el archivo data.bin
        fstream* temp_file = nullptr;

        while(var_temps_SF::punt_pos!=-1 ){
            temp_file = var_temps_SF::punt_is_in_data ? &file1 : &file2;
            temp_file->seekg(get_pos_fisica(var_temps_SF::punt_pos, var_temps_SF::punt_is_in_data), ios::beg);
            temp_file->read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
            //Escribimos el registro en temp.bin
            var_temps_SF::punt_pos = var_temps_SF::rec_temp.punt_nextPosLogic;
            var_temps_SF::punt_is_in_data = var_temps_SF::rec_temp.punt_next_is_In_Data;

            //Escribimos el registro en temp.bin
            file_temp.seekp(0, ios::end);
            if(var_temps_SF::punt_pos!=-1){
                var_temps_SF::rec_temp.punt_nextPosLogic = get_pos_logical(int64_t(file_temp.tellp())+int64_t(sizeof(var_temps_SF::rec_temp)), true);
                var_temps_SF::rec_temp.punt_next_is_In_Data = true;
            }
            file_temp.write(reinterpret_cast<const char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
        }
        
        file1.close();
        file2.close();
        file_temp.close();

        // ----------------------FINISH-----------------------------
        ifstream file_temp1("files/temp.bin", ios::binary | ios::in);
        if(file_temp1.is_open()){
            remove("files/data_sf.bin");
            file_temp1.close();
            rename("files/temp.bin", "files/data_sf.bin");
            file2.open("files/aux_sf.bin", ios::binary | ios::out | ios::trunc);
            file2.seekp(0, ios::beg);
            var_temps_SF::n_aux = 0;
            file2.write(reinterpret_cast<const char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));
            file2.close();
        }else{
            cout<<"Aun no se ha creado el archivo temp.bin"<<endl;
            file_temp1.close();
        }

    }
    
    Record_SFile search(T key, bool admin=false){
        fstream file1("files/" + this->filename, ios::binary | ios::in | ios::out);
        if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
        file1.seekg(0, ios::beg);
        file1.read(reinterpret_cast<char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
        
        if(var_temps_SF::n_data==0) throw runtime_error("No hay registros");

        if(binary_search(stoll(key))){ //O(log(n))
            cout<<"++++++ Registro encontrado ++++++"<<endl;
            return var_temps_SF::rec_found;
        }else{
            file1.seekg(get_pos_fisica(var_temps_SF::u_before, var_temps_SF::u_before_is_in_data), ios::beg);
            file1.read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
            var_temps_SF::punt_pos = var_temps_SF::rec_temp.punt_nextPosLogic;
            var_temps_SF::punt_is_in_data = var_temps_SF::rec_temp.punt_next_is_In_Data;

            fstream file2("files/aux_sf.bin", ios::binary | ios::in | ios::out);
            file2.seekg(get_pos_fisica(var_temps_SF::punt_pos, var_temps_SF::punt_is_in_data), ios::beg);

            while(var_temps_SF::punt_is_in_data!=true && var_temps_SF::punt_pos!=-1){//O(log(n))
                file2.read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
                if(var_temps_SF::rec_temp.key_value()==stoll(key, nullptr, 10)){
                    var_temps_SF::pos_found = var_temps_SF::punt_pos;
                    var_temps_SF::found_in_data = false;
                    cout<<"++++++ Registro encontrado ++++++"<<endl;
                    return var_temps_SF::rec_temp;
                }
                var_temps_SF::pos_found = var_temps_SF::punt_pos;
                var_temps_SF::found_in_data = var_temps_SF::punt_is_in_data;

                var_temps_SF::punt_pos = var_temps_SF::rec_temp.punt_nextPosLogic;
                var_temps_SF::punt_is_in_data = var_temps_SF::rec_temp.punt_next_is_In_Data;
                file2.seekg(get_pos_fisica(var_temps_SF::punt_pos, var_temps_SF::punt_is_in_data), ios::beg);
            }
            
            file1.close();
            file2.close();
            if(!admin) throw runtime_error("Registro no encontrado");
        }
        return var_temps_SF::rec_temp; //Evita un warning
    }

    vector<Record_SFile> range_search(T begin_key, T end_key){
        if(stoll(begin_key)>stoll(end_key)) throw runtime_error("El rango de busqueda es incorrecto");
        else if(stoll(begin_key)==stoll(end_key)) return {search(begin_key)};
        else if(stoll(begin_key)<0 || stoll(end_key)<0) throw runtime_error("Las claves deben ser positivas");

        vector<Record_SFile> records;
        fstream file1("files/" + this->filename, ios::binary | ios::in);
        if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + filename);
        fstream file2("files/aux_sf.bin", ios::binary | ios::in);
        if (!file2.is_open()) throw runtime_error("No se pudo abrir el archivo metadata.dat");

        int64_t begin_pos;
        bool begin_is_in_data;


        if(binary_search(stoll(begin_key))){ //O(log(n))
            begin_pos = var_temps_SF::pos_found;
            begin_is_in_data = var_temps_SF::found_in_data;
        }else{
            begin_pos = var_temps_SF::u_before;
            begin_is_in_data = var_temps_SF::u_before_is_in_data;
        }

        fstream* temp_file = nullptr;
        if(begin_pos==-1){
            file1.seekg(0, ios::beg);
            file1.read(reinterpret_cast<char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
            file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
            file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));

            temp_file = var_temps_SF::punt_is_in_data ? &file1 : &file2;
            temp_file->seekg(get_pos_fisica(var_temps_SF::punt_pos, var_temps_SF::punt_is_in_data), ios::beg);
            temp_file->read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
            if(var_temps_SF::rec_temp.key_value()>=stoll(begin_key) && var_temps_SF::rec_temp.key_value()<=stoll(end_key)){
                records.push_back(var_temps_SF::rec_temp);
            }
            var_temps_SF::punt_pos = var_temps_SF::rec_temp.punt_nextPosLogic;
            var_temps_SF::punt_is_in_data = var_temps_SF::rec_temp.punt_next_is_In_Data;
        }else{
            //Start
            temp_file = begin_is_in_data ? &file1 : &file2;
            temp_file->seekg(get_pos_fisica(begin_pos, begin_is_in_data), ios::beg);
            temp_file->read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
            if(var_temps_SF::rec_temp.key_value()>=stoll(begin_key) && var_temps_SF::rec_temp.key_value()<=stoll(end_key)){
                records.push_back(var_temps_SF::rec_temp);
            }
            var_temps_SF::punt_pos = var_temps_SF::rec_temp.punt_nextPosLogic;
            var_temps_SF::punt_is_in_data = var_temps_SF::rec_temp.punt_next_is_In_Data;
        }

        while(var_temps_SF::punt_pos!=-1 && var_temps_SF::rec_temp.key_value() <= stoll(end_key)){
            temp_file = var_temps_SF::punt_is_in_data ? &file1 : &file2;
            temp_file->seekg(get_pos_fisica(var_temps_SF::punt_pos, var_temps_SF::punt_is_in_data), ios::beg);
            temp_file->read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
            if(var_temps_SF::rec_temp.key_value()>=stoll(begin_key) && var_temps_SF::rec_temp.key_value()<=stoll(end_key)){
                records.push_back(var_temps_SF::rec_temp);
            }
            var_temps_SF::punt_pos = var_temps_SF::rec_temp.punt_nextPosLogic;
            var_temps_SF::punt_is_in_data = var_temps_SF::rec_temp.punt_next_is_In_Data;
        }


        return records;
    }


    bool remove_record(T key){
        fstream file1("files/" + this->filename, ios::binary | ios::in | ios::out);
        fstream file2("files/aux_sf.bin", ios::binary | ios::in | ios::out);
        if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
        if (!file2.is_open()) throw runtime_error("No se pudo abrir el archivo metadata.dat");

        file1.seekg(0, ios::beg);
        file2.seekg(0, ios::beg);

        file1.read(reinterpret_cast<char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
        file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));

        file2.read(reinterpret_cast<char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));

        

        return true;
    }

    void print_file(const string& namefile){
        ifstream file("files/" + namefile, ios::binary | ios::in);
        if (!file.is_open()) throw runtime_error("No se pudo abrir el archivo " + filename);

        file.seekg(0, ios::beg);
        cout<<"****** "<<namefile<<" ******"<<endl;

        file.read(reinterpret_cast<char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
        cout<<setw(6)<<"n_data: "<<left<<var_temps_SF::n_data;

        if(namefile==this->filename || namefile=="temp.bin"){
            if(var_temps_SF::n_data==0 && var_temps_SF::punt_pos==0 && var_temps_SF::punt_is_in_data==false){
                cout<<endl<<"-------------------------------------------------"<<endl;
                cout<<"No hay registros"<<endl;
                return;
            }
            file.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
            file.read(reinterpret_cast<char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
            cout<<" | "<<setw(6)<<"punt_pos: "<<left<<var_temps_SF::punt_pos<<" | "
            <<setw(6)<<"In_Data?: "<<left<<boolalpha<<var_temps_SF::punt_is_in_data<<endl;
        }else{ 
            cout<<endl;
            if(var_temps_SF::n_data==0){
                cout<<"-------------------------------------------------"<<endl;
                cout<<"No hay registros"<<endl;
                return;
            }
        }

        cout<<"-------------------------------------------------"<<endl;
        int64_t iter_pos = 0;
        
        while(!file.eof()){
            file.read(reinterpret_cast<char*>(&var_temps_SF::rec_temp), sizeof(var_temps_SF::rec_temp));
            if(file.eof()) break;
            cout<<iter_pos<<": ";
            var_temps_SF::rec_temp.showData_line();
            iter_pos++;
        }

        file.close();
    }
};

#endif //PROYECTO_1_SEQUENTIALFILE_H