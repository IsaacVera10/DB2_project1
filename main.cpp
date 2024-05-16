#include "indexes/Sequential_file.h"
#include <limits>

//Variables globales constantes
const int64_t MAX_RECORDS = 1000;

void visualizar_generar_bin(bool visualizar = false){
    records_csv_to_bin("dataset/movie_dataset.csv");
    // ++++++++++Manejo de movie_dataset.bin++++++++++++
    ifstream movie_bin("./dataset/movie_dataset.bin", ios::binary);
    if(!movie_bin.is_open()) throw runtime_error("Error al abrir el archivo");

    movie_bin.seekg(0, ios::end);
    cout<<"------------------------------------------\n";
    cout<<"Cantidad de registros en .bin: "<<movie_bin.tellg()/sizeof(Record)<<endl;
    cout<<"------------------------------------------\n"<<endl;

    if(visualizar){
        cout<<"---------------------------------------------Visualización de registros---------------------------------------------\n";
        movie_bin.seekg(0, ios::beg);
        Record record;
        while(movie_bin.read(reinterpret_cast<char*>(&record), sizeof(Record))){
            record.showData_line();
        }
        cout<<"---------------------------------------------Visualización de registros---------------------------------------------\n";
    }

    movie_bin.close();
    // +++++++++++++++++++++++++++++++++++++++++++++++++
}

void test_sequential_file(){
    //1. Descomentar y generar los registros de .csv a .bin
    // visualizar_generar_bin();

    Sequential_File file("data_sf.bin",1);

    //2. Descomentar para usar leer los registros de .bin
    // ++++++++++Manejo de movie_dataset.bin++++++++++++
    ifstream movie_bin("./dataset/movie_dataset.bin", ios::binary);
    if(!movie_bin.is_open()) throw runtime_error("Error al abrir el archivo");

    Record record;
    Record_SFile record_sf;
    for(int i = 0; i<MAX_RECORDS; i++){
        movie_bin.read(reinterpret_cast<char*>(&record), sizeof(Record));
        record_sf = Record_SFile(record);
        file.add(record_sf);
    }
    movie_bin.close();
    // +++++++++++++++++++++++++++++++++++++++++++++++++

    //3. Visualización de archivos
    cout<<endl;
    file.print_file("data_sf.bin");
    cout<<endl;
    file.print_file("aux_sf.bin");
    cout<<endl;

    //ADVERTENCIA: Si se comentó la sección ADD, cambiar a 0 el segundo parámetro del constructor de Sequential_File
    // //4. Remove record
    if(file.remove_record("284054")) cout<<"Registro eliminado"<<endl;

    cout<<endl;
    file.print_file("data_sf.bin");
    cout<<endl;
    file.print_file("aux_sf.bin");
    cout<<endl;

    // //5. Busqueda de registros
    auto vec = file.range_search("23","150");
    for(auto& r: vec){
        r.showData_line();
    }
    // cout<<endl;
    // //6. Busqueda de registros
    // file.search("99861").showData_line();
    // cout<<endl;
}

int main(){
    test_sequential_file();

    return 0;

}