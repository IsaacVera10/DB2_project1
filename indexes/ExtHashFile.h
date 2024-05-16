#ifndef EXTHASHING_EXTHASHFILE_H
#define EXTHASHING_EXTHASHFILE_H

#include <cstdio>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <functional>
#include <cmath>
#include <bitset>


using namespace std;

const int MAX_BUCKETS = 8;
const int MAX_RECORDS = 100;

template<typename T>
struct Record {
    long id;
    string title;
    float voteAverage;
    int voteCount;
    string releaseDate;
    long revenue;
    int runtime;
    string language;

    void setData(ifstream &file) {
        string line;
        if (getline(file, line)) {
            stringstream iss(line);

            string temp;

            // Read id
            getline(iss, temp, ',');
            id = stol(temp);

            // Read title
            getline(iss, title, ',');

            // Read voteAverage
            getline(iss, temp, ',');
            voteAverage = stof(temp);

            // Read voteCount
            getline(iss, temp, ',');
            voteCount = stoi(temp);

            // Read releaseDate
            getline(iss, releaseDate, ',');

            // Read revenue
            getline(iss, temp, ',');
            revenue = stol(temp);

            // Read runtime
            getline(iss, temp, ',');
            runtime = stoi(temp);

            // Read language
            getline(iss, language, ',');
        }
    }

    void showData() {
        cout<<setw(10)<<left<<id;
        cout<<setw(10)<<left<<title;
        cout<<setw(10)<<left<<voteAverage;
        cout<<setw(10)<<left<<voteCount;
        cout<<setw(10)<<left<<releaseDate;
        cout<<setw(10)<<left<<revenue;
        cout<<setw(10)<<left<<runtime;
        cout<<setw(10)<<left<<language << endl;

    }

    T getKey() {
        return this->id;
    }
};

template<typename T>
struct Bucket {
    Record<T> records[MAX_RECORDS];
    long next_bucket = -1;
    int size = 0;
    int depth = 1;
    long bucketAddress;

    Bucket(){}

    void showData(){
        cout << "Size: " << this->size;
        cout<<" ["<<endl;
        for(int i=0;i<size;i++)
            records[i].showData();
        cout<<"]"<<endl;
    }
};

struct HashIndex {
    string binary;
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
    string fileName;
    vector<HashIndex> indexVector; //To keep in RAM all indexes and save them in disk
    int D;
    int FB;

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
        dataFile.seekp(0, ios::beg);
        dataFile.write(reinterpret_cast<char *>(&bucket), sizeof(bucket)); //Insert first bucket in datafile

        dataFile.seekp(sizeof(bucket), ios::beg);
        dataFile.write(reinterpret_cast<char *>(&bucket), sizeof(bucket)); //Insert second bucket in datafile

        vector<string> binaryStrings = generateBinaryStrings(); //Generate 2^D binary strings
        indexFile.seekp(0, ios::beg);
        for (int i = 0; i < binaryStrings.size(); ++i) {
            HashIndex hashIndex;
            hashIndex.binary = binaryStrings[i];
            hashIndex.bucketAddress = this->makeAddress(binaryStrings[i], 1);

            //Keep in disk all 2^D indexes
            indexFile.write(reinterpret_cast<char *>(&hashIndex), sizeof(hashIndex));
        }

        dataFile.close();
        indexFile.close();
    }

    /*
    Record* searchInBucket(Bucket &bucket, long key) {
        for (int i = 0; i < bucket.size; i++) {
            if (bucket.records[i].getKey() == key)
                return &bucket.records[i];
        }

        return nullptr;
    }
     */

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
            string binaryString = bitset<64>(i).to_string();
            binaryString = binaryString.substr(64 - D);
            binaryStrings.push_back(binaryString);
        }

        return binaryStrings;
    }

public:
    ExtHashFile(string fileName){
        this->fileName = std::move(fileName);
        if (!this->fileExists())
            this->createFile();
    }

    void writeRecord(Record<T> record) {
        fstream dataFile(this->fileName, ios::binary | ios::out | ios::in);

        if(!dataFile.is_open())
            throw runtime_error("Error opening data file");

        //Get directory for record to be inserted
        int directory = makeAddress(record.getKey());
        cout << "directory: " << directory << endl;


        //Locate bucket where record will be inserted
        Bucket<T> bucket;
        //cout << "Reading from position: " << index*sizeof(bucket) << endl;
        dataFile.seekg(directory*sizeof(bucket), ios::beg);
        dataFile.read(reinterpret_cast<char*>(&bucket), sizeof(bucket));

        if (dataFile.gcount() == 0 || bucket.size == 0) {
            //cout << "Entro al if gcount" << endl;
            dataFile.close();
            dataFile.open(this->fileName, ios::binary | ios::out | ios::in);
            Bucket newBucket;
            newBucket.records[0] = record;
            newBucket.size = 1;
            dataFile.seekp(index*sizeof(newBucket), ios::beg);
            dataFile.write(reinterpret_cast<char *>(&newBucket), sizeof(newBucket));
        }
        else {
            //If bucket has free space, insert new record and update bucket in data file
            if (bucket.size < MAX_RECORDS) {
                bucket.records[bucket.size] = record;
                bucket.size++;
                dataFile.seekp(index*sizeof(bucket), ios::beg);
                dataFile.write(reinterpret_cast<char *>(&bucket), sizeof(bucket));
            }

            else {
                //Bring main bucket at the end of data file and become an overflow bucket
                dataFile.seekp(0, ios::end);
                long pos = dataFile.tellp() / sizeof(bucket);
                //cout << "pos: " << pos << endl;
                dataFile.write(reinterpret_cast<char*>(&bucket), sizeof(bucket));

                //Insert new bucket with record and pointer to overflow bucket
                // in the address of previous main bucket
                Bucket<T> newBucket;
                newBucket.records[0] = record;
                newBucket.size = 1;
                newBucket.next_bucket = pos;
                dataFile.seekp(index*sizeof(newBucket), ios::beg);
                dataFile.write(reinterpret_cast<char*>(&newBucket), sizeof(newBucket));
            }
        }

        dataFile.close();

    }

    Record<T> search(string nombre){
        fstream dataFile(this->fileName, ios::binary | ios::in);

        if(!dataFile.is_open())
            throw runtime_error("Error opening data file");

        dataFile.seekg(0, ios::end);
        if(dataFile.tellg()==0)
            throw runtime_error("File is empty");

        //Get index for key being searched
        size_t hashValue = std::hash<string>{}(nombre);
        int index = hashValue % this->nBuckets;
        //cout << "index: " << index << endl;
        Bucket<T> bucket;
        bucket.next_bucket = index;
        Record<T>* record = nullptr;
        do {
            //cout << "Next Bucket: " << bucket.next_bucket << endl;
            dataFile.seekg(bucket.next_bucket*sizeof(bucket), ios::beg);
            dataFile.read(reinterpret_cast<char*>(&bucket), sizeof(bucket));
            record = this->searchInBucket(bucket, nombre);
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

    void delete(Record<T> record) {

    }
};





int main(){

}

#endif //EXTHASHING_EXTHASHFILE_H
