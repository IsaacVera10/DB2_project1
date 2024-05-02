#ifndef PROYECTO_1_SEQUENTIALFILE_H
#define PROYECTO_1_SEQUENTIALFILE_H
#include <fstream>
#include <vector>
#include <cmath>
#include "data.h"
#define T string

using namespace std;
void test_SequentialFile(){
    try{
        Sequential_File file("data.bin");
        Record_SFile Record_SFile1("202010572", 4, 950.0, "Pagado");
        Record_SFile Record_SFile2("201810572", 4, 950.0, "No pagado");
        Record_SFile Record_SFile3("201910572", 4, 950.0, "Pagado");
        Record_SFile Record_SFile4("202010573", 4, 950.0, "Pagado");
        Record_SFile Record_SFile5("202010571", 4, 950.0, "Pagado");
        Record_SFile Record_SFile6("202010574", 4, 950.0, "Pagado");
        Record_SFile Record_SFile7("201610576", 4, 950.0, "Pagado");
        Record_SFile Record_SFile8("201610577", 4, 950.0, "Pagado");
        Record_SFile Record_SFile9("201010578", 4, 950.0, "Pagado");
        Record_SFile Record_SFile10("203010579", 4, 950.0, "Pagado");
        Record_SFile Record_SFile11("203010580", 4, 950.0, "Pagado");
        Record_SFile Record_SFile12("203010581", 4, 950.0, "Pagado");
        Record_SFile Record_SFile13("203010582", 4, 950.0, "Pagado");
        Record_SFile Record_SFile14("203010583", 4, 950.0, "Pagado");
        Record_SFile Record_SFile15("203010584", 4, 950.0, "Pagado");
        Record_SFile Record_SFile16("203010585", 4, 950.0, "Pagado");
        Record_SFile Record_SFile17("201945623", 4, 950.0, "Pagado");

        file.add(Record_SFile1);
        file.add(Record_SFile2);
        file.add(Record_SFile3);
        file.add(Record_SFile4);
        file.add(Record_SFile5);
        file.add(Record_SFile6);
        file.add(Record_SFile7);
        file.add(Record_SFile8);
        file.add(Record_SFile9);
        file.add(Record_SFile10);
        file.add(Record_SFile11);
        file.add(Record_SFile12);
        file.add(Record_SFile13);

        file.print_file("data.bin");
        cout<<endl;
        file.print_file("aux.bin");
        cout<<endl;
        auto Record_SFile_found = file.search("203010582");
        Record_SFile_found.showData();
        

        
        vector<Record_SFile> vec = file.range_search("201610576", "202010574");

        cout<<endl;
        file.print_file("data.bin");
        cout<<endl;
        file.print_file("aux.bin");
        cout<<endl;

        for(auto& v: vec){
            cout<<v.Codigo<<endl;
        }

        

        // Imprimiremos el archivo positions.bin
        ifstream positions("files/positions_data.bin", ios::binary);
        if(!positions.is_open()){
            throw runtime_error("No se pudo abrir el archivo positions.bin");
        }
        cout<<"\nArchivo positions.bin: "<<endl;
        size_t pos;
        int i=0;
        while(positions.read(reinterpret_cast<char*>(&pos), sizeof(size_t))){
            cout<<setw(3)<<left<<i<<": "<<pos<<" "<<endl;
            i++;
        }
        positions.close();

        file.remove_record("201610576");
        file.add(Record_SFile14);
        file.add(Record_SFile15);
        file.add(Record_SFile16);
        file.remove_record("201010578");
        file.add(Record_SFile17);
        

        cout<<endl;
        file.print_file("data.bin");
        cout<<endl;
        file.print_file("aux.bin");
        cout<<endl;
    }
    catch(exception& e){
        cerr<<"\nError: "<<e.what()<<endl;
    }
    cout<<endl;
}


namespace var_temps_SF{
    int n_data = 0, n_aux=0;
    size_t punt_pos = 0,recordBefore_pos = 0, size_record = 0;
    bool punt_is_in_data = false, recordBefore_is_in_data = false;
    Record_SFile rec_temp;
}

class Sequential_File {
private:
    string filename;
public:
    explicit Sequential_File(string filename) : filename(std::move(filename)) {
        fstream file1("files/" + this->filename, ios::binary | ios::out | ios::trunc);
        fstream file2("files/aux.bin", ios::binary | ios::out | ios::trunc);
        fstream file3("files/positions_data.bin", ios::binary | ios::out | ios::trunc);
        if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
        if (!file2.is_open()) throw runtime_error("No se pudo abrir el archivo metadata.dat");
        if (!file3.is_open()) throw runtime_error("No se pudo abrir el archivo positions.bin");

        file1.write(reinterpret_cast<const char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
        file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
        file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
        
        file2.write(reinterpret_cast<const char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));

        file1.close();
        file2.close();
        file3.close();
    }

    void add(Record_SFile record){
        fstream file1("files/" + this->filename, ios::binary | ios::in | ios::out);
        fstream file2("files/aux.bin", ios::binary | ios::in | ios::out);
        fstream file3("files/positions_data.bin", ios::binary | ios::in | ios::out);
        if (!file1.is_open()) throw runtime_error("No se pudo abrir el archivo " + this->filename);
        if (!file2.is_open()) throw runtime_error("No se pudo abrir el archivo metadata.dat");
        if (!file3.is_open()) throw runtime_error("No se pudo abrir el archivo positions.bin");
        

        file1.seekg(0, ios::beg);
        file2.seekg(0, ios::beg);

        file1.read(reinterpret_cast<char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
        file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
        file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));


        file2.read(reinterpret_cast<char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));


        if(var_temps_SF::n_data<2){//La inserción se hará directamente en el archivo de datos hasta que haya 2 registros
            file1.seekp(0, ios::end);

            if(var_temps_SF::n_data==0){//Si no hay registros, se escribe el registro y se actualiza la metadata

                //Actualizamos la metadata
                var_temps_SF::punt_pos = file1.tellp();
                var_temps_SF::punt_is_in_data = true;
                var_temps_SF::n_data++;
                
                //Escribimos el registro
                char* buffer = record.empaquetar();
                file1.write(buffer, record.size_of());
                delete[] buffer;

                //Escribimos la metadata
                file1.seekp(0, ios::beg);
                file1.write(reinterpret_cast<const char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
                file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
                file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
                
            }else{
                file1.seekp(sizeof(var_temps_SF::n_data), ios::beg);
                file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
                
                file1.seekp(var_temps_SF::punt_pos, ios::beg);
                file1.read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record));
                file1.seekp(-sizeof(var_temps_SF::size_record), ios::cur);

                //Desempaquetamos el registro
                char* buffer = new char[var_temps_SF::size_record];
                file1.read(buffer, var_temps_SF::size_record);
                var_temps_SF::rec_temp.desempaquetar(buffer);
                delete[] buffer;

                
                if(record.Key_Value == var_temps_SF::rec_temp.Key_Value){
                    throw runtime_error("Ya existe un registro con la llave " + record.Key_Value);
                }
                if(record.Key_Value < var_temps_SF::rec_temp.Key_Value){//Si el registro a insertar es menor al primero, se inserta antes de este
                    //Actualizamos la metadata cabezera
                    var_temps_SF::punt_pos = file1.tellp();
                    var_temps_SF::punt_is_in_data = true;
                    var_temps_SF::n_data++;
                    
                    //Actualizamos el puntero del registro a insertar
                    file1.seekp(sizeof(int)+sizeof(size_t)+sizeof(bool), ios::beg);
                    record.punt_nextPosFisica = file1.tellp();
                    record.punt_next_is_In_Data = true;

                    //Escribimos el registro
                    char* buffer = record.empaquetar();
                    file1.seekp(0, ios::end);
                    file1.write(buffer, record.size_of());
                    delete[] buffer;

                    //Escribimos la metadata cabezera
                    file1.seekp(0, ios::beg);
                    file1.write(reinterpret_cast<const char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
                    file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
                    file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));


                }else{

                    //Actualizamos la metadata cabezera y el puntero del registro ya existente
                    var_temps_SF::punt_pos = file1.tellp();
                    var_temps_SF::punt_is_in_data = true;
                    var_temps_SF::n_data++;

                    //Escribimos el puntero del registro ya existente
                    file1.seekp(-sizeof(size_t)-sizeof(bool), ios::cur);
                    file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
                    file1.write(reinterpret_cast<const char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));


                    //Escribimos el registro nuevo
                    char* buffer = record.empaquetar();
                    file1.write(buffer, record.size_of());
                    delete[] buffer;

                    //Escribimos la metadata cabezera
                    file1.seekp(0, ios::beg);
                    file1.write(reinterpret_cast<const char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
                    file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
                    file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));
                }
            }   
        }else{
            getPointer_beforeRecord(record, file1, file2);//Obtenemos el puntero del registro anterior al que se va a insertar

            //Me dirijo al record anterior del que se va a insertar
            fstream* temp_file = nullptr;

            if(var_temps_SF::recordBefore_is_in_data==0 && var_temps_SF::recordBefore_pos==0){//Si el registro a insertar es menor al primero, se inserta antes de este
                temp_file = &file1;
                temp_file->seekg(sizeof(var_temps_SF::n_data), ios::beg);
            }else{
                temp_file = var_temps_SF::recordBefore_is_in_data ? &file1 : &file2;
                temp_file->seekg(var_temps_SF::recordBefore_pos, ios::beg);
                //Leo el puntero del record anterior
                temp_file->read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record));
                temp_file->seekg(var_temps_SF::size_record-(2*sizeof(var_temps_SF::size_record))-sizeof(bool), ios::cur);
            }
            temp_file->read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
            temp_file->read(reinterpret_cast<char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::recordBefore_is_in_data));

            //Escribimos en file2 el registro a insertar con el puntero del record anterior
            record.punt_nextPosFisica = var_temps_SF::punt_pos;
            record.punt_next_is_In_Data = var_temps_SF::punt_is_in_data;
            
            file2.seekp(0, ios::end);

            var_temps_SF::punt_pos = file2.tellp();//Guardamos la posición
            var_temps_SF::punt_is_in_data = false;//del record a insertar para actualizar el puntero del record anterior

            char* buffer = record.empaquetar();
            file2.write(buffer, record.size_of());
            delete[] buffer;

            //Actualizamos el puntero del record anterior
            if(var_temps_SF::recordBefore_is_in_data==0 && var_temps_SF::recordBefore_pos==0){//Si el registro a insertar es menor al primero, se inserta antes de este
                temp_file->seekp(sizeof(var_temps_SF::n_data), ios::beg);
            }else{
                temp_file->seekp(var_temps_SF::recordBefore_pos, ios::beg);
                temp_file->read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record));
                temp_file->seekp(var_temps_SF::size_record-(2*sizeof(var_temps_SF::size_record))-sizeof(bool), ios::cur);
            }
            temp_file->write(reinterpret_cast<const char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
            temp_file->write(reinterpret_cast<const char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));

            //Actualizamos el puntero de la metadata cabezera de aux.bin
            file2.seekp(0, ios::beg);
            var_temps_SF::n_aux++;
            file2.write(reinterpret_cast<const char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));

            if(int(log2(var_temps_SF::n_data))==var_temps_SF::n_aux){//Si hay 2 registros en aux.bin, se hace el merge
                reBuild(file1, file2, file3);
            }
        }

        if(file1.is_open()) file1.close();
        if(file2.is_open()) file2.close();
        if(file3.is_open()) file3.close();

        var_temps_SF::punt_pos = 0;
        var_temps_SF::punt_is_in_data = false;
        var_temps_SF::recordBefore_pos = 0;
        var_temps_SF::recordBefore_is_in_data = false;
   }

    void reBuild(fstream& file1, fstream& file2, fstream& file3){//Reconstruye el archivo data, pero también cierra los archivos para remover y renombrar.
        fstream file4("files/temp.bin", ios::binary | ios::trunc | ios::out);
        if (!file4.is_open()) throw runtime_error("No se pudo abrir el archivo temp.bin");

        file1.seekg(0, ios::beg);
        file1.read(reinterpret_cast<char*>(&var_temps_SF::n_data), sizeof(var_temps_SF::n_data));
        file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_pos), sizeof(var_temps_SF::punt_pos));
        file1.read(reinterpret_cast<char*>(&var_temps_SF::punt_is_in_data), sizeof(var_temps_SF::punt_is_in_data));

        file2.seekg(0, ios::beg);
        file2.read(reinterpret_cast<char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));

        file3.seekg(0, ios::beg);

        //Escribimos la metadata cabezera en temp.bin
        file4.seekg(0, ios::beg);
        int n_temp = var_temps_SF::n_data + var_temps_SF::n_aux;
        file4.write(reinterpret_cast<const char*>(&n_temp), sizeof(n_temp));

        size_t punt_temp = sizeof(var_temps_SF::n_data) + sizeof(var_temps_SF::punt_pos) + sizeof(var_temps_SF::punt_is_in_data);
        file4.write(reinterpret_cast<const char*>(&punt_temp), sizeof(punt_temp));

        bool punt_is_in_data_temp = true;
        file4.write(reinterpret_cast<const char*>(&punt_is_in_data_temp), sizeof(punt_is_in_data_temp));

        //Escribo la posicion del record
        file3.write(reinterpret_cast<const char*>(&punt_temp), sizeof(punt_temp));


        //-----------------------------------------------------------------------------------------------

        fstream* temp_file = nullptr;
        
        while(var_temps_SF::punt_pos!=0){
            temp_file = var_temps_SF::punt_is_in_data ? &file1 : &file2;
            temp_file->seekg(var_temps_SF::punt_pos, ios::beg);
            temp_file->read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record));
            temp_file->seekg(-sizeof(var_temps_SF::size_record), ios::cur);

            //Copiamos el registro a temp.bin
            char* buffer = new char[var_temps_SF::size_record];
            temp_file->read(buffer, var_temps_SF::size_record);
            var_temps_SF::rec_temp.desempaquetar(buffer);
            file4.seekp(0, ios::end);
            file4.write(buffer, var_temps_SF::size_record);
            delete[] buffer;

            if(var_temps_SF::rec_temp.punt_nextPosFisica != 0){
                //Actualizamos el puntero del record insertado
                punt_temp = file4.tellp();
                punt_is_in_data_temp = true;
                //Escribimos la posicion del record
                file3.write(reinterpret_cast<const char*>(&punt_temp), sizeof(punt_temp));
                //Escribimos el puntero actualizado del record insertado
                file4.seekp(-sizeof(punt_temp)-sizeof(punt_is_in_data_temp), ios::cur);
                file4.write(reinterpret_cast<const char*>(&punt_temp), sizeof(punt_temp));
                file4.write(reinterpret_cast<const char*>(&punt_is_in_data_temp), sizeof(punt_is_in_data_temp));
            }

            var_temps_SF::punt_pos = var_temps_SF::rec_temp.punt_nextPosFisica;
            var_temps_SF::punt_is_in_data = var_temps_SF::rec_temp.punt_next_is_In_Data;
        }
        file1.close();
        file2.close();
        file3.close();
        file4.close();

        //------FINISH------------------------
        
        ifstream file_temp("files/temp.bin", ios::binary | ios::in);
        if(file_temp.is_open()){
            remove("files/data.bin");
            file_temp.close();
            rename("files/temp.bin", "files/data.bin");
            file2.open("files/aux.bin", ios::binary | ios::out | ios::trunc);
            file2.seekp(0, ios::beg);
            var_temps_SF::n_aux = 0;
            file2.write(reinterpret_cast<const char*>(&var_temps_SF::n_aux), sizeof(var_temps_SF::n_aux));
            file2.close();
        }else{
            // cout<<"Aun no se ha creado el archivo temp.bin"<<endl;
            file_temp.close();
        }
    }

    void getPointer_beforeRecord(Record_SFile& record, fstream& file1, fstream& file2){
        fstream* temp_file = nullptr;

        while(var_temps_SF::punt_pos != 0){
            temp_file = var_temps_SF::punt_is_in_data ? &file1 : &file2;
            
            //Nos dirigimos al record apuntado
            temp_file->seekg(var_temps_SF::punt_pos, ios::beg);
            temp_file->read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record));
            temp_file->seekg(-sizeof(var_temps_SF::size_record), ios::cur);

            //Desempaquetamos el registro
            char* buffer = new char[var_temps_SF::size_record];
            temp_file->read(buffer, var_temps_SF::size_record);
            var_temps_SF::rec_temp.desempaquetar(buffer);
            delete[] buffer;


            if(record.Key_Value == var_temps_SF::rec_temp.Key_Value){
                throw runtime_error("Ya existe un registro con la llave " + record.Key_Value);
            }

            if(record.Key_Value < var_temps_SF::rec_temp.Key_Value) return;

            var_temps_SF::recordBefore_pos = var_temps_SF::punt_pos;
            var_temps_SF::recordBefore_is_in_data = var_temps_SF::punt_is_in_data;
            
            var_temps_SF::punt_pos = var_temps_SF::rec_temp.punt_nextPosFisica;
            var_temps_SF::punt_is_in_data = var_temps_SF::rec_temp.punt_next_is_In_Data;
        }
    }

    void print_file(const string& namefile){
        ifstream file("files/" + namefile, ios::binary | ios::in);
        if (!file.is_open()) throw runtime_error("No se pudo abrir el archivo " + filename);

        file.seekg(0, ios::beg);

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

        while(file.read(reinterpret_cast<char*>(&var_temps_SF::size_record), sizeof(var_temps_SF::size_record))){
            cout<<setw(5) << left<< (size_t)file.tellg()-sizeof(var_temps_SF::size_record)<<": ";
            file.seekg(-sizeof(var_temps_SF::size_record), ios::cur);

            //Desempaquetamos el registro
            char* buffer = new char[var_temps_SF::size_record];
            file.read(buffer, var_temps_SF::size_record);
            var_temps_SF::rec_temp.desempaquetar(buffer);
            
            var_temps_SF::rec_temp.showData_line();

            delete[] buffer;

        }

        file.close();
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