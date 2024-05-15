#include "Sequential_file.h"
#include <limits>

//Variables globales constantes
const int64_t MAX_RECORDS = 100;

int main(){
    //1. Descomentar y generar los registros de .csv a .bin
    // records_csv_to_bin("dataset/movie_dataset.csv"); //Comentar luego de generar toda la data en .bin

    Sequential_File file("data_sf.bin",1);

    //2. Descomentar para usar leer los registros de .bin
    // ++++++++++Manejo de movie_dataset.bin++++++++++++
    ifstream movie_bin("./dataset/movie_dataset.bin", ios::binary);
    if(!movie_bin.is_open()) throw runtime_error("Error al abrir el archivo");

    Record_SFile record;
    for(int i = 0; i<MAX_RECORDS; i++){
        movie_bin.read(reinterpret_cast<char*>(&record), sizeof(Record_SFile));
        file.add(record);
        // record.showData_line();
    }
    movie_bin.seekg(0, ios::end);
    cout<<"\nCantidad de registros: "<<movie_bin.tellg()/sizeof(Record_SFile)<<endl;
    movie_bin.close();
    // +++++++++++++++++++++++++++++++++++++++++++++++++

    //3. Visualización de archivos
    cout<<endl;
    file.print_file("data_sf.bin");
    cout<<endl;
    file.print_file("aux_sf.bin");
    cout<<endl;
    // file.print_file("temp.bin");
    file.search("299536").showData_line();
    cout<<endl;

    // file.remove("14");
    // auto vec = file.range_search("99862","14");

    //Busqueda de registros
    // file.search("99862").showData_line();
    // cout<<endl;


    return 0;

}