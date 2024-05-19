#include "indexes/Sequential_file.h"
#include "dataset/data.h"
#include "parser/parsersql.h"
#include <chrono>


//Variables globales constantes
const int64_t MAX_RECORDS = 10000;

void visualizar_generar_bin(bool visualizar = false){
    records_csv_to_bin("movie_dataset.csv");
    // ++++++++++Manejo de movie_dataset.bin++++++++++++
    ifstream movie_bin(data_path+"movie_dataset.bin", ios::binary);
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

    auto start_delete = chrono::high_resolution_clock::now();
    Sequential_File file("data_sf.bin",1);

    //2. Descomentar para usar leer los registros de .bin
    // ++++++++++Manejo de movie_dataset.bin++++++++++++
    ifstream movie_bin(data_path+"movie_dataset.bin", ios::binary);
    if(!movie_bin.is_open()) throw runtime_error("Error al abrir el archivo");

    Record record;
    for(int i = 0; i<MAX_RECORDS; i++){
        movie_bin.read(reinterpret_cast<char*>(&record), sizeof(Record));
        file.add(record);
    }
    movie_bin.close();
    // // +++++++++++++++++++++++++++++++++++++++++++++++++
    auto end_delete = chrono::high_resolution_clock::now();
    
    //3. Visualización de archivos
    cout<<endl;
    file.print_file("data_sf.bin");
    cout<<endl;
    file.print_file("aux_sf.bin");
    cout<<endl;

    auto start_delete1 = chrono::high_resolution_clock::now();
    //6. Busqueda de registros
    file.search("227156").showData_line();
    cout<<endl;
    auto end_delete1 = chrono::high_resolution_clock::now();
    cout << "Add: " << chrono::duration_cast<chrono::milliseconds>(end_delete - start_delete).count() << " ms" << endl;
    cout << "Search: " << chrono::duration_cast<chrono::milliseconds>(end_delete1 - start_delete1).count() << " ms" << endl;

    //ADVERTENCIA: Si se comentó la sección ADD, cambiar a 0 el segundo parámetro del constructor de Sequential_File
    //4. Remove record
    // if(file.remove_record("9802")) cout<<"Registro eliminado"<<endl;
    // if(file.remove_record("11")) cout<<"Registro eliminado"<<endl;
    // if(file.remove_record("791373")) cout<<"Registro eliminado"<<endl;

    // cout<<endl;
    // file.print_file("data_sf.bin");
    // cout<<endl;
    // file.print_file("aux_sf.bin");
    // cout<<endl;

    // //5. Busqueda de registros
    // auto vec = file.range_search("23","150");
    // for(auto& r: vec){
    //     r.showData_line();
    // }
    cout<<endl;

}

void testParser(){
    // Test for SELECT
    string query = "SELECT * FROM tablaHash WHERE id = 1 USING ISAM";
    Parser parser(query);
    vector<Record> records;
    parser.parse(records);

    cout << string(120, '-') << endl;
    // Test for SELECT with WHERE BETWEEN
    query = "SELECT * FROM tablaHash WHERE id BETWEEN (1, 10) USING ISAM";
    Parser parser1(query);
    parser1.parse(records);
    cout << string(120, '-') << endl;
    // Test for CREATE TABLE
    query = "CREATE TABLE tablaPruebaHash FROM FILE \"movies.csv\" USING INDEX SEQUENTIAL(\"id\")";
    Parser parser2(query);
    parser2.parse(records);
    cout << string(120, '-') << endl;
    // Test for INSERT
    query = "INSERT INTO tablaHash VALUES (1, \"The Shawshank Redemption\", 9.33, 33333, \"1994-10-14\", 28341469, 142, \"en\") USING ISAM";
    Parser parser3(query);
    parser3.parse(records);
    cout << string(120, '-') << endl;
    // Test for DELETE
    query = "DELETE FROM tablaHash WHERE id = 1 USING ISAM";
    Parser parser4(query);
    parser4.parse(records);
}




int main(){
   test_sequential_file();
    // ExtHashFile extHashFile("data_eh.bin", 1);

    // testParser();



    return 0;

}