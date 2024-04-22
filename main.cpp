#include "Sequential_file.h"

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

int main(){
    test_SequentialFile();


    return 0;

}