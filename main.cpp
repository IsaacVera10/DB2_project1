#include "Sequential_file.h"
#include <limits>

//Variables globales constantes
const int64_t MAX_RECORDS = 913041;

vector<Record_SFile> vec_data;
void using_vector_records(){
    vec_data = generate_struct_records("dataset/movie_dataset.csv", MAX_RECORDS);

    // for(auto& v: vec_data){
    //     v.showData_line();
    // }

    cout<<endl;
}

int main(){
    //1. Descomentar y generar los registros de .csv a .bin
    // records_csv_to_bin("dataset/movie_dataset.csv"); //Comentar luego de generar toda la data en .bin


    Sequential_File file("data_sf.bin",1);

    //2. Descomentar para usar leer los registros de .bin
    // ++++++++++Manejo de movie_dataset.bin++++++++++++
    // ifstream movie_bin("./dataset/movie_dataset.bin", ios::binary);
    // if(!movie_bin.is_open()) throw runtime_error("Error al abrir el archivo");

    // Record_SFile record;
    // for(int i = 0; i<MAX_RECORDS; i++){
    //     movie_bin.read((char*)&record, sizeof(Record_SFile));
    //     file.add(record);
    //     // record.showData_line();
    // }
    // movie_bin.seekg(0, ios::end);
    // cout<<"\nCantidad de registros: "<<movie_bin.tellg()/sizeof(Record_SFile)<<endl;
    // movie_bin.close();
    // +++++++++++++++++++++++++++++++++++++++++++++++++


    //3. Visualización de archivos
    file.print_file("data_sf.bin");
    cout<<endl;
    file.print_file("aux_sf.bin");
    cout<<endl;
    // file.print_file("temp.bin");
    // file.search("27206").showData_line();


    return 0;

}