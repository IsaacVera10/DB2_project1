#include "Sequential_file.h"
#include <limits>

int main(){

    auto vec_data = generate_struct_records("dataset/movie_dataset.csv", 20);
    // for(auto& v: vec_data){
    //     v.showData_line();
    // }
    // cout<<endl;
    
    Sequential_File file("data_sf.bin",1);
    for(auto& v: vec_data){
        file.add(v);
    }
    
    file.print_file("data_sf.bin");
    cout<<endl;
    file.print_file("aux_sf.bin");
    cout<<endl;
    // file.print_file("temp.bin");
    // file.search("27206").showData_line();


    return 0;

}