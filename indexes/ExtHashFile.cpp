#include <cstdio>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <functional>
#include <cmath>
#include <bitset>
#include <cstring>
#include <cstdint>



using namespace std;

const int D = 3;
const int FB = 10;

template<typename T>
struct Record {
    int64_t id;
    char title[71];
    float voteAverage;
    int64_t voteCount;
    char releaseDate[11];
    int64_t revenue;
    int64_t runtime;
    char language[3];

    void setData(ifstream &file) {
        std::string line;
        if (std::getline(file, line)) {
            std::vector<std::string> fields;
            std::string field;
            bool inQuotes = false;

            // Custom parsing to handle quoted fields
            for (char ch: line) {
                if (ch == '"') {
                    inQuotes = !inQuotes;
                } else if (ch == ',' && !inQuotes) {
                    fields.push_back(field);
                    field.clear();
                } else {
                    field += ch;
                }
            }
            fields.push_back(field);

            if (fields.size() == 8) { // Ensure we have the right number of fields
                id = std::stoll(fields[0]);

                std::strncpy(title, fields[1].c_str(), sizeof(title));
                title[sizeof(title) - 1] = '\0'; // Ensure null-termination

                voteAverage = std::stof(fields[2]);
                voteCount = std::stoll(fields[3]);

                std::strncpy(releaseDate, fields[4].c_str(), sizeof(releaseDate));
                releaseDate[sizeof(releaseDate) - 1] = '\0'; // Ensure null-termination

                revenue = std::stoll(fields[5]);
                runtime = std::stoll(fields[6]);

                std::strncpy(language, fields[7].c_str(), sizeof(language));
                language[sizeof(language) - 1] = '\0'; // Ensure null-termination
            }
        }
    }

    void showData() {
        cout<< "id: " << setw(10)<<left<<id;
        cout<< "title: " << setw(10)<<left<<title;
        cout<< "voteAverage: " << setw(10)<<left<<voteAverage;
        cout<< "voteCount: " << setw(10)<<left<<voteCount;
        cout<< "releaseDate: " << setw(10)<<left<<releaseDate;
        cout<< "revenue: " << setw(10)<<left<<revenue;
        cout<< "runtime: " << setw(10)<<left<<runtime;
        cout<< "language: " << setw(10)<<left<<language << endl;

    }

    T getKey() {
        return this->id;
    }

    // Default constructor
    Record() = default;

    // Copy constructor
    /*Record(const Record<T>& other) {
        id = other.id;
        //title = other.title;
        voteAverage = other.voteAverage;
        voteCount = other.voteCount;
        //releaseDate = other.releaseDate;
        revenue = other.revenue;
        runtime = other.runtime;
        //language = other.language;
    }
     */

    /*
    Record<T>& operator=(const Record<T>& other) {
        if (this != &other) {
            id = other.id;
            //title = other.title;
            voteAverage = other.voteAverage;
            voteCount = other.voteCount;
            //releaseDate = other.releaseDate;
            revenue = other.revenue;
            runtime = other.runtime;
            //language = other.language;
        }
        return *this;
    }
    */
};

template<typename T>
struct Bucket {
    Record<T> records[FB];
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
        bucket.binary = 0;
        bucket.bucketAddress = 0;
        dataFile.seekp(0, ios::beg);
        dataFile.write(reinterpret_cast<char *>(&bucket), sizeof(bucket)); //Insert first bucket in datafile

        Bucket<T> bucket2;
        bucket2.bucketAddress = dataFile.tellp();
        bucket2.binary = 1;
        //bucket.bucketAddress = sizeof(bucket);
        //dataFile.seekp(sizeof(bucket), ios::beg);
        dataFile.write(reinterpret_cast<char *>(&bucket2), sizeof(bucket2)); //Insert second bucket in datafile

        dataFile.close();

        vector<string> binaryStrings = this->generateBinaryStrings(); //Generate 2^D binary strings

        //Store in disk all 2^D indexes
        indexFile.seekp(0, ios::beg);
        for (int i = 0; i < binaryStrings.size(); ++i) {
            HashIndex hashIndex;
            hashIndex.binary = stoi(binaryStrings[i]);
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
        //cout << "hashValue: " << hashValue << endl;
        int index = hashValue % static_cast<int>(pow(2, D));
        //cout << "index: " << index << endl;


        long bucketAddress = this->indexVector[index].bucketAddress;
        //cout << "bucketAddress: " << bucketAddress << endl;


        //Read bucket from disk
        Bucket<T> bucket;
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

                Bucket<T> newBucket;
                newBucket.depth = bucket.depth;
                newBucket.binary = this->makeAddress(newBinary2, bucket.depth);



                //Redistribute elements in current bucket and newBucket
                for (int i = 0; i < bucket.size; i++) {
                    size_t hashKey = std::hash<T>{}(bucket.records[i].getKey());
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

        long bucketAddress = this->indexVector[index].bucketAddress;

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


    void deleteRecord(T key) {
        fstream dataFile(fileName, ios::binary | ios::in | ios::out);

        if (!dataFile.is_open()) {
            throw runtime_error("Error opening data file");
        }

        size_t hashValue = hash<T>{}(key);
        int index = hashValue % static_cast<int>(pow(2, D));
        long bucketAddress = indexVector[index].bucketAddress;

        Bucket<T> bucket;
        dataFile.seekg(bucketAddress, ios::beg);
        dataFile.read(reinterpret_cast<char*>(&bucket), sizeof(bucket));

        bool recordFound = false;
        for (int i = bucket.size - 1; i >= 0; --i) {
            if (bucket.records[i].getKey() == key) {
                // Move the last record to the position of the deleted record
                bucket.records[i] = bucket.records[bucket.size - 1];
                bucket.size--;
                recordFound = true;
                break;
            }
        }

        if (!recordFound) {
            dataFile.close();
            throw runtime_error("Record not found");
        }

        dataFile.seekp(bucketAddress, ios::beg);
        dataFile.write(reinterpret_cast<char*>(&bucket), sizeof(bucket));

        // Handle empty bucket scenario
        if (bucket.size == 0) {
            if (bucket.next_bucket != -1) {
                // Load the overflow bucket
                Bucket<T> nextBucket;
                dataFile.seekg(bucket.next_bucket, ios::beg);
                dataFile.read(reinterpret_cast<char*>(&nextBucket), sizeof(nextBucket));

                // Copy records from the overflow bucket to the current bucket
                for (int i = 0; i < nextBucket.size; ++i) {
                    bucket.records[i] = nextBucket.records[i];
                }
                bucket.size = nextBucket.size;
                bucket.next_bucket = nextBucket.next_bucket;

                dataFile.seekp(bucketAddress, ios::beg);
                dataFile.write(reinterpret_cast<char*>(&bucket), sizeof(bucket));
            } else {
                mergeBuckets(bucket, bucketAddress);
            }
        }

        dataFile.close();
    }

    void mergeBuckets(Bucket<T>& emptyBucket, long emptyBucketAddress) {
        // Identify the buddy bucket
        int buddyBinary = emptyBucket.binary ^ 1; // Flip the last bit to find the buddy
        long buddyAddress = -1;
        for (const auto& hashIndex : indexVector) {
            if (hashIndex.binary == buddyBinary) {
                buddyAddress = hashIndex.bucketAddress;
                break;
            }
        }


        fstream dataFile(fileName, ios::binary | ios::in | ios::out);
        if (!dataFile.is_open()) {
            throw runtime_error("Error opening data file");
        }

        Bucket<T> buddyBucket;
        dataFile.seekg(buddyAddress, ios::beg);
        dataFile.read(reinterpret_cast<char*>(&buddyBucket), sizeof(buddyBucket));

        // Check if the buddy bucket is mergeable
        if (buddyBucket.depth == emptyBucket.depth) {
            buddyBucket.depth--;

            for (auto& hashIndex : indexVector) {
                if (hashIndex.bucketAddress == emptyBucketAddress || hashIndex.bucketAddress == buddyAddress) {
                    hashIndex.bucketAddress = buddyAddress;
                }
            }

            // Update buddy bucket to data file
            dataFile.seekp(buddyAddress, ios::beg);
            dataFile.write(reinterpret_cast<char*>(&buddyBucket), sizeof(buddyBucket));
        }

        dataFile.close();

        // Update index vector back to index file
        ofstream indexFile("indexFile.bin", ios::binary | ios::out | ios::trunc);
        for (const auto& hashIndex : indexVector) {
            indexFile.write(reinterpret_cast<const char*>(&hashIndex), sizeof(hashIndex));
        }
        indexFile.close();
    }
};



//Testing

void writeFileFromFile(string fileName){
    ExtHashFile<int64_t> file(fileName);
    Record<int64_t> record;
    ifstream fileIn("dataset.csv");
    int contador = 0;
    while(contador <= 50)
    {
        if(fileIn.eof()) break;
        record.setData(fileIn);
        cout << "Inserting: ....." << endl;
        //record.showData();
        file.writeRecord(record); //Write to data file
        //cout << "Finish Inserting...." << endl;
        //cout << "---Show Data----" << endl;
        //file.scanAll();
        contador++;
    }
    file.scanAll();
    fileIn.close();
}

void readFile(string fileName){
    ExtHashFile<int64_t> file(fileName);
    //cout<<"--------- show all data -----------\n";
    //file.scanAll();
    cout<<"--------- search 27205 -----------\n";
    Record<int64_t> record = file.search(27205);
    record.showData();

    cout<<"--------- delete 27205 -----------\n";
    file.deleteRecord(27205);

    cout<<"--------- search key not in file -----------\n";
    Record<int64_t> record2 = file.search(27205);
    record2.showData();
}


int main(){
    writeFileFromFile("dataHash.bin");
    readFile("dataHash.bin");
    return 0;
}
