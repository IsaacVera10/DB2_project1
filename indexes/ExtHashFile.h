#include <cstdio>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <functional>
#include <cmath>
#include <bitset>
#include "../dataset/data.h"

using namespace std;

const int D = 8;
const int FB = 100;

template<typename T>
struct Bucket {
    Record<T> records[FB];
    int64_t next_bucket = -1;
    int size = 0;
    int depth = 1;
    string binary;
    int64_t bucketAddress;

    Bucket(){}

    void showData(){
        cout << "Size: " << this->size;
        cout<<" ["<<endl;
        for(int i=0;i<size;i++)
            records[i].showData();
        cout<<"]"<<endl;
    }

    void deleteRecord(int p) {
        // Shift elements to the left
        for (int i = p; i < size - 1; i++) {
            records[i] = records[i + 1];
        }
        
        size--;
    }
};

struct HashIndex {
    string binary;
    int64_t bucketAddress;

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

    bool fileExists()
    {
        ifstream dataFile(this->fileName, ios::binary);
        bool exists = dataFile.good();
        dataFile.close();
        return exists;
    }

    void createFile()
    {
        ofstream dataFile(this->fileName, ios::app | ios::binary);
        ofstream indexFile("indexFile.bin", ios::app | ios::binary);

        Bucket<T> bucket;
        bucket.binary = "0";
        bucket.bucketAddress = 0;
        dataFile.seekp(0, ios::beg);
        dataFile.write(reinterpret_cast<char *>(&bucket), sizeof(bucket)); //Insert first bucket in datafile

        Bucket<T> bucket2;
        bucket2.bucketAddress = dataFile.tellp();
        bucket2.binary = "1";
        //bucket.bucketAddress = sizeof(bucket);
        //dataFile.seekp(sizeof(bucket), ios::beg);
        dataFile.write(reinterpret_cast<char *>(&bucket2), sizeof(bucket2)); //Insert second bucket in datafile

        dataFile.close();

        vector<string> binaryStrings = this->generateBinaryStrings(); //Generate 2^D binary strings

        //Store in disk all 2^D indexes
        indexFile.seekp(0, ios::beg);
        for (int i = 0; i < binaryStrings.size(); ++i) {
            HashIndex hashIndex;
            hashIndex.binary = binaryStrings[i];
            hashIndex.bucketAddress = (this->makeAddress(binaryStrings[i], 1) == 0) ? 0 : sizeof(bucket);

            this->indexVector.push_back(hashIndex);
            indexFile.write(reinterpret_cast<char *>(&hashIndex), sizeof(hashIndex));
        }

        indexFile.close();
    }


    Record<T>* searchInBucket(Bucket<T> &bucket, T key) {
        for (int i = 0; i < bucket.size; i++) {
            if (bucket.records[i].getKey() == key)
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
            string binaryString = bitset<D>(i).to_string();
            binaryStrings.push_back(binaryString);
        }

        return binaryStrings;
    }

    void loadIndex() {
        ifstream indexFile("indexFile.bin", ios::binary);
        HashIndex hashIndex;
        indexFile.seekg(0, ios::beg);
        while(indexFile.read(reinterpret_cast<char*>(&hashIndex), sizeof(hashIndex))) {
            this->indexVector.push_back(hashIndex);
        }

        indexFile.close();
    }

public:
    explicit ExtHashFile(string fileName){
        this->fileName = std::move(fileName);
        if (!this->fileExists())
            this->createFile();
        else
            this->loadIndex();
    }

    void writeRecord(Record<T> record) {
        fstream dataFile(this->fileName, ios::binary | ios::out | ios::in);

        if(!dataFile.is_open())
            throw runtime_error("Error opening data file");

        //Locate bucket where record will be inserted
        size_t hashValue = std::hash<T>{}(record.getKey());
        cout << "hashValue: " << hashValue << endl;
        int index = hashValue % static_cast<int>(pow(2, D));
        cout << "index: " << index << endl;


        int64_t bucketAddress = this->indexVector[index].bucketAddress;
        cout << "bucketAddress: " << bucketAddress << endl;


        //Read bucket from disk
        Bucket<T> bucket;
        dataFile.seekg(bucketAddress, ios::beg);
        dataFile.read(reinterpret_cast<char*>(&bucket), sizeof(bucket));

        cout << "bucket.size: " << bucket.size << endl;
        cout << "bucket.binary: " << bucket.binary << endl;

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
                string newBinary1 = "0" + bucket.binary;
                string newBinary2 = "1" + bucket.binary;

                bucket.depth++;
                bucket.binary = newBinary1;

                Bucket<T> newBucket;
                newBucket.depth = bucket.depth;
                newBucket.binary = newBinary2;

                dataFile.seekp(0, ios::end); //Go to the end of file
                int64_t newBucketAddress = dataFile.tellp();

                //Redistribute elements in current bucket and newBucket
                for (int i = 0; i < bucket.size; i++) {
                    size_t hashKey = std::hash<T>{}(record.getKey());
                    int indexKey = hashKey % static_cast<int>(pow(2, D));
                    string binaryKey = bitset<D>(indexKey).to_string();
                    if (this->makeAddress(binaryKey, bucket.depth) != this->makeAddress(bucket.binary, bucket.depth)) {
                        newBucket.records[newBucket.size] = bucket.records[i];
                        newBucket.size++;
                        bucket.deleteRecord(i);
                    }
                }

                //Insert new bucket to datafile
                dataFile.write(reinterpret_cast<char *>(&newBucket), sizeof(newBucket));


                //Update index on RAM
                for (int i = 0; this->indexVector.size(); i++) {
                    if (this->indexVector[i].bucketAddress == bucket.bucketAddress) {
                        if (this->makeAddress(this->indexVector[i].binary, bucket.depth) != this->makeAddress(bucket.binary, bucket.depth)) {
                            this->indexVector[i].bucketAddress = newBucketAddress;
                        }
                    }
                }

                //Save modified index on Disk
                ofstream indexFile("indexFile.bin",  ios::binary);
                indexFile.seekp(0, ios::beg);
                for (int i = 0; this->indexVector.size(); i++) {
                    HashIndex hashIndex = this->indexVector[i];
                    indexFile.write(reinterpret_cast<char *>(&hashIndex), sizeof(hashIndex));
                }

                indexFile.close();
                dataFile.close();

                this->writeRecord(record); //Call recursively to insert new record that caused a split
            }

            else {
                //Put current bucket at the end of data file and become an overflow bucket
                dataFile.seekp(0, ios::end);
                int64_t overflowAddress = dataFile.tellp();
                dataFile.write(reinterpret_cast<char*>(&bucket), sizeof(bucket));

                //Insert new bucket with record and pointer to overflow bucket
                // in the address of current bucket
                Bucket<T> newBucket;
                newBucket.records[0] = record;
                newBucket.size = 1;
                newBucket.next_bucket = overflowAddress;
                dataFile.seekp(bucketAddress, ios::beg);
                dataFile.write(reinterpret_cast<char*>(&newBucket), sizeof(newBucket));

                dataFile.close();
            }
        }
    }

    Record<T> search(T key){
        fstream dataFile(this->fileName, ios::binary | ios::in);

        if(!dataFile.is_open())
            throw runtime_error("Error opening data file");

        dataFile.seekg(0, ios::end);
        if(dataFile.tellg()==0)
            throw runtime_error("File is empty");

        //Locate bucket where record will be inserted
        size_t hashValue = std::hash<T>{}(key);
        int index = hashValue % static_cast<int>(pow(2, D));

        int64_t bucketAddress = this->indexVector[index].bucketAddress;

        Bucket<T> bucket;
        bucket.next_bucket = bucketAddress;
        Record<T>* record;
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
        ifstream dataFile(fileName, ios::binary);
        if(!dataFile.is_open())
            throw runtime_error("Error opening data file");

        dataFile.seekg(0, ios::end);
        if(dataFile.tellg()==0)
            throw runtime_error("File is empty");

        dataFile.seekg(0, ios::beg);
        Bucket<T> bucket;
        int contador = 0;
        while(dataFile.read(reinterpret_cast<char*>(&bucket), sizeof(bucket))){
            cout << "Bucket: " << contador  << " Next Bucket: " << bucket.next_bucket << endl;
            bucket.showData();
            contador++;
        }

        cout<<endl;
        dataFile.close();
    }

    /*
    void delete(Record<T> record) {

    }
     */
};



//Testing

void writeFileFromFile(string fileName){
    ExtHashFile<int64_t> file(fileName);
    Record<int64_t> record;
    ifstream fileIn("dataset.csv");
    while(true)
    {
        if(fileIn.eof()) break;
        record.setData(fileIn);
        cout << "Inserting: ....." << endl;
        record.showData();
        file.writeRecord(record); //Write to data file
        //cout << "Finish Inserting...." << endl;
        //cout << "---Show Data----" << endl;
        //file.scanAll();
    }
    file.scanAll();
    fileIn.close();
}

void readFile(string fileName){
    ExtHashFile<int64_t> file(fileName);
    cout<<"--------- show all data -----------\n";
    file.scanAll();
    //cout<<"--------- search Pedro -----------\n";
    //Record<int64_t> record = file.search(1);
    //record.showData();

    //cout<<"--------- search key not in file -----------\n";
    //Record<int64_t> record2 = file.search(1);
    //record2.showData();
}


int main(){
    writeFileFromFile("dataHash.bin");
    //readFile("dataHash.bin");
    return 0;
}
