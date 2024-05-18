#include <cstdio>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <functional>
#include <cmath>
#include <bitset>
#include <cstring>
#include <cstdint>
#include "../dataset/data.h"

using namespace std;

const int D = 3;
const int FB = 10;


struct Bucket {
    Record records[FB];
    long next_bucket = -1;
    int size = 0;
    int depth = 1;
    int binary = 0;
    long bucketAddress = 0;

    Bucket(){}

    void showData(){
        cout << "Size: " << this->size;
        cout<<" ["<<endl;
        for(int i=0;i<size;i++)
            records[i].showData_line();
        cout<<"]"<<endl;
    }

    void deleteRecord(int p) {
        // Shift elements to the left
        for (int i = p; i < size - 1; i++) {
            records[i] = records[i + 1];
        }

        size--;
    }

    void info_line(){
        cout<<"Size: "<<setw(10)<<left<<size<<" | "
            <<"Depth: "<<setw(10)<<left<<depth<<" | "
            <<"Binary: "<<setw(10)<<left<<binary<<" | "
            <<"BucketAddress: "<<setw(10)<<left<<bucketAddress<<endl;
    }

    string toBinaryString() {
        stringstream ss;
        for (size_t i = 0; i < this->depth; ++i) {
            ss << ((this->binary >> (this->depth - 1 - i)) & 1);
        }
        return ss.str();
    }
};

struct HashIndex {
    int binary;
    long bucketAddress;

    HashIndex(){}

    void showData(){
        cout<<setw(10)<<left<<binary;
        cout<<setw(10)<<left<<bucketAddress<< endl;
    }
};


template<typename T>
class ExtHashFile {
private:
    string fileName; //Data file name
    vector<HashIndex> indexVector; //To keep in RAM all indexes and save them in disk

    bool fileExists(){
        ifstream dataFile(this->fileName, ios::binary);
        bool exists = dataFile.good();
        dataFile.close();
        return exists;
    }

    void createFile(){
        fstream dataFile(bin_path + this->fileName, ios::binary | ios::out | ios::trunc);
        fstream indexFile(bin_path + "indexFile.bin", ios::binary | ios::out | ios::trunc);
        if(!dataFile.is_open()) throw runtime_error("Error opening data file");
        if(!indexFile.is_open()) throw runtime_error("Error opening index file");

        Bucket bucket;
        bucket.binary = 0;
        bucket.bucketAddress = 0;
        dataFile.seekp(0, ios::beg);
        dataFile.write(reinterpret_cast<char *>(&bucket), sizeof(bucket)); //Insert first bucket in datafile

        Bucket bucket2;
        bucket2.bucketAddress = dataFile.tellp();
        bucket2.binary = 1;
        //bucket.bucketAddress = sizeof(bucket);
        //dataFile.seekp(sizeof(bucket), ios::beg);
        dataFile.write(reinterpret_cast<char *>(&bucket2), sizeof(bucket2)); //Insert second bucket in datafile
        bucket.info_line();
        bucket2.info_line();
        dataFile.close();
        cout<<"Data file created"<<endl;

        vector<string> binaryStrings = this->generateBinaryStrings(); //Generate 2^D binary strings
        cout<<"Binary strings generated"<<endl;

        //Store in disk all 2^D indexes
        indexFile.seekp(0, ios::beg);
        for (int i = 0; i < binaryStrings.size(); ++i) {
            cout<<"Binary: "<<binaryStrings[i]<<endl;
            HashIndex hashIndex;
            hashIndex.binary = stoi(binaryStrings[i]);
            hashIndex.bucketAddress = (this->makeAddress(binaryStrings[i], 1) == 0) ? 0 : sizeof(bucket);

            this->indexVector.push_back(hashIndex);
            indexFile.write(reinterpret_cast<char *>(&hashIndex), sizeof(hashIndex));
        }

        indexFile.close();
    }


    Record* searchInBucket(Bucket &bucket, T key) {
        for (int i = 0; i < bucket.size; i++) {
            if (bucket.records[i].key_value() == key)
                return &bucket.records[i];
        }

        return nullptr;
    }

    int makeAddress(string binaryString, int depth) { //Returns position of bucket depending on its local depth
        int response = 0;
        int mask = 1;
        int binary = std::stoi(binaryString);

        for(int j = 0; j < depth; j++) {
            response = response << 1;
            int lowBit = binary & mask;
            response = response | lowBit;
            binary = binary >> 1;
        }

        return response;
    }

    vector<string> generateBinaryStrings() {
        vector<string> binaryStrings;

        for (int i = 0; i < pow(2, D); ++i) {
            bitset<D> num_binary(i);
            string binaryString = num_binary.to_string();
            cout<<binaryString<<endl;
            binaryStrings.push_back(binaryString);
        }

        return binaryStrings;
    }

    void loadIndex() {
        ifstream indexFile("indexFile.bin", ios::binary);
        if(!indexFile.is_open())
            throw runtime_error("Error opening index file");

        HashIndex hashIndex;
        indexFile.seekg(0, ios::beg);
        while(indexFile.read(reinterpret_cast<char*>(&hashIndex), sizeof(hashIndex))) {
            this->indexVector.push_back(hashIndex);
        }

        indexFile.close();
    }

public:
    explicit ExtHashFile(string fileName, bool activate_trunc=false) : fileName(std::move(fileName)){
        fstream file1;
        fstream file2;
        
        if(activate_trunc){
            cout<<"entra"<<endl;
            this->createFile();
        }
        else{
            cout<<"Entra 2"<<endl;
            if (!this->fileExists())
                this->createFile();
            else
                this->loadIndex();
        }
    }

    void writeRecord(Record record) {
        fstream dataFile(this->fileName, ios::binary | ios::out | ios::in);

        if(!dataFile.is_open())
            throw runtime_error("Error opening data file");

        //Locate bucket where record will be inserted
        size_t hashValue = std::hash<T>{}(record.key_value());
        //cout << "hashValue: " << hashValue << endl;
        int index = hashValue % static_cast<int>(pow(2, D));
        //cout << "index: " << index << endl;


        long bucketAddress = this->indexVector[index].bucketAddress;
        //cout << "bucketAddress: " << bucketAddress << endl;


        //Read bucket from disk
        Bucket bucket;
        dataFile.seekg(bucketAddress, ios::beg);
        dataFile.read(reinterpret_cast<char*>(&bucket), sizeof(bucket));

        //cout << "bucket.size: " << bucket.size << endl;
        //cout << "bucket.binary: " << bucket.binary << endl;

        //If bucket has free space, insert new record and update bucket in data file
        if (bucket.size < FB) {
            bucket.records[bucket.size] = record;
            bucket.size++;
            dataFile.seekp(bucketAddress, ios::beg);
            dataFile.write(reinterpret_cast<char *>(&bucket), sizeof(bucket));
            dataFile.close();
        }

        else {

            //Check bucket's local depth against global depth
            if (bucket.depth < D) {
                //Split bucket
                string binaryString = bucket.toBinaryString();

                string newBinary1 = "0" + binaryString;
                string newBinary2 = "1" + binaryString;

                bucket.depth++;
                bucket.binary = this->makeAddress(newBinary1, bucket.depth);

                Bucket newBucket;
                newBucket.depth = bucket.depth;
                newBucket.binary = this->makeAddress(newBinary2, bucket.depth);



                //Redistribute elements in current bucket and newBucket
                for (int i = 0; i < bucket.size; i++) {
                    size_t hashKey = std::hash<T>{}(bucket.records[i].key_value());
                    int indexKey = hashKey % static_cast<int>(pow(2, D));
                    string binaryKey = bitset<D>(indexKey).to_string();
                    //cout << "binaryKey: " << binaryKey << endl;
                    int bucketNum = this->makeAddress(binaryKey, bucket.depth);
                    //cout << "bucketNum: " << bucketNum << endl;

                    if (bucketNum != bucket.binary) {
                        newBucket.records[newBucket.size] = bucket.records[i];
                        newBucket.size++;
                        bucket.deleteRecord(i);
                    }
                }

                //Save modified bucket to datafile
                dataFile.seekp(bucketAddress, ios::beg);
                dataFile.write(reinterpret_cast<char *>(&bucket), sizeof(bucket));

                //Insert new bucket to datafile
                dataFile.seekp(0, ios::end); //Go to the end of file
                long newBucketAddress = dataFile.tellp();
                newBucket.bucketAddress = newBucketAddress;
                dataFile.write(reinterpret_cast<char *>(&newBucket), sizeof(newBucket));
                dataFile.close();

                //Update index on RAM
                for (int i = 0; i < this->indexVector.size(); i++) {
                    if (this->indexVector[i].bucketAddress == bucket.bucketAddress) {
                        string binaryKey = bitset<D>(this->indexVector[i].binary).to_string();
                        int bucketNum2 = this->makeAddress(binaryKey, bucket.depth);
                        //cout << "bucketNum2: " << bucketNum2 << endl;

                        if (bucketNum2 != bucket.binary) {
                            this->indexVector[i].bucketAddress = newBucketAddress;
                        }
                    }
                }

                //Save modified index on Disk
                ofstream indexFile("indexFile.bin",  ios::binary);
                indexFile.seekp(0, ios::beg);
                for (int i = 0; i < this->indexVector.size(); i++) {
                    HashIndex hashIndex = this->indexVector[i];
                    indexFile.write(reinterpret_cast<char *>(&hashIndex), sizeof(hashIndex));
                }

                indexFile.close();

                this->writeRecord(record); //Call recursively to insert new record that caused a split
            }

            else {
                //Put current bucket at the end of data file and become an overflow bucket
                dataFile.seekp(0, ios::end);
                long overflowAddress = dataFile.tellp();
                dataFile.write(reinterpret_cast<char*>(&bucket), sizeof(bucket));

                //Insert new bucket with record and pointer to overflow bucket
                // in the address of current bucket
                Bucket newBucket;
                newBucket.records[0] = record;
                newBucket.size = 1;
                newBucket.next_bucket = overflowAddress;
                dataFile.seekp(bucketAddress, ios::beg);
                dataFile.write(reinterpret_cast<char*>(&newBucket), sizeof(newBucket));

                dataFile.close();
            }
        }
    }

    Record search(T key){
        fstream dataFile(this->fileName, ios::binary | ios::in);

        if(!dataFile.is_open())
            throw runtime_error("Error opening data file");

        dataFile.seekg(0, ios::end);
        if(dataFile.tellg()==0)
            throw runtime_error("File is empty");

        //Locate bucket where record will be inserted
        size_t hashValue = std::hash<T>{}(key);
        int index = hashValue % static_cast<int>(pow(2, D));

        long bucketAddress = this->indexVector[index].bucketAddress;

        Bucket bucket;
        bucket.next_bucket = bucketAddress;
        Record* record;
        do {
            //cout << "Next Bucket: " << bucket.next_bucket << endl;
            dataFile.seekg(bucket.next_bucket, ios::beg);
            dataFile.read(reinterpret_cast<char*>(&bucket), sizeof(bucket));
            record = this->searchInBucket(bucket, key);
            //cout << "Search in Bucket finished" << endl;
        } while(bucket.next_bucket != -1 && record == nullptr);

        dataFile.close();

        if (record == nullptr)
            throw runtime_error("Key not found in data file");

        Record rObj = (*record);
        return rObj;
    }

    void scanAll(){
        ifstream dataFile(bin_path + fileName, ios::binary);
        if(!dataFile.is_open())
            throw runtime_error("Error opening data file");

        dataFile.seekg(0, ios::end);
        if(dataFile.tellg()==0)
            throw runtime_error("File is empty");

        dataFile.seekg(0, ios::beg);
        Bucket bucket;
        int contador = 0;
        while(dataFile.read(reinterpret_cast<char*>(&bucket), sizeof(bucket))){
            cout << "Bucket: " << contador  << " Next Bucket: " << bucket.next_bucket << endl;
            bucket.showData();
            contador++;
        }

        cout<<endl;
        dataFile.close();
    }

    void print_file(const string& namefile) {
        ifstream file(bin_path + namefile, ios::binary | ios::in);

        if (!file.is_open()) {
            cout << "Error opening file: " << namefile << endl;
            return;
        }

        if (namefile == "indexFile.bin") { // Imprime el contenido del archivo de índices (indexFile)
            HashIndex hashIndex;
            while (file.read(reinterpret_cast<char*>(&hashIndex), sizeof(HashIndex))) {
                cout << "Binary: " << hashIndex.binary << ", BucketAddress: " << hashIndex.bucketAddress << endl;
            }
        } else if (namefile == fileName) {// Imprime el contenido del archivo de datos (dataFile)
            Bucket bucket;
            while (file.read(reinterpret_cast<char*>(&bucket), sizeof(Bucket))) {
                cout << "Binary: " << bucket.binary << ", BucketAddress: " << bucket.bucketAddress << endl;
            }
        } else {
            cout << "Invalid file name" << endl;
        }

        file.close();
    }

    /*
    void delete(Record record) {

    }
     */
};



//Testing

// void writeFileFromFile(string fileName){
//     ExtHashFile<int64_t> file(fileName);
//     Record record;
//     ifstream fileIn("dataset.csv");
//     int contador = 0;
//     while(contador <= 50)
//     {
//         if(fileIn.eof()) break;
//         record.setData(fileIn);
//         cout << "Inserting: ....." << endl;
//         record.showData();
//         file.writeRecord(record); //Write to data file
//         cout << "Finish Inserting...." << endl;
//         cout << "---Show Data----" << endl;
//         file.scanAll();
//         contador++;
//     }
//     file.scanAll();
//     fileIn.close();
// }

// void readFile(string fileName){
//     ExtHashFile<int64_t> file(fileName);
//     cout<<"--------- show all data -----------\n";
//     file.scanAll();
//     cout<<"--------- search 27205 -----------\n";
//     Record<int64_t> record = file.search(27205);
//     record.showData();

//     cout<<"--------- search key not in file -----------\n";
//     Record<long> record2 = file.search(1);
//     record2.showData();
// }


// int main(){
//     writeFileFromFile("dataHash.bin");
//     readFile("dataHash.bin");
//     return 0;
// }
