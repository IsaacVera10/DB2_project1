#include "dataset/data.h"
#include <limits>

int main(){

    auto vec = generate_struct_records(100000); //Mi limite
    for(auto& v: vec){
        v.showData_line();
    }

    return 0;

}