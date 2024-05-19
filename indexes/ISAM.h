//
// Created by Leonardo Candio on 11/05/24.
//

#ifndef DB2_PROJECT1_ISAM_H
#define DB2_PROJECT1_ISAM_H

#include "iostream"
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <utility>

using namespace std;

////////////// Index Page ////////////////////
const int INDEX_PAGE_SIZE = 512; // Page size for the index

template<typename KeyType>
static constexpr size_t M = (INDEX_PAGE_SIZE - sizeof(size_t) - sizeof(size_t)) /
                            (sizeof(size_t) + sizeof(KeyType));

template<typename KeyType>
struct IndexPage {
    KeyType key[M<KeyType>];           // Array of keys
    size_t children[M<KeyType> + 1]{}; // Pointers to children
    size_t n_keys = 0; // Counts the number of keys in the index page.

    char *c_str() { return reinterpret_cast<char *>(this); }

    char *deserialize() { return reinterpret_cast<char *>(this); }
};

/////////// Data Page ////////

const int DATA_PAGE_SIZE = 4096; // Page size for the data
template<typename RecordType>
static constexpr size_t N =
        (DATA_PAGE_SIZE - sizeof(size_t) - sizeof(long long)) / sizeof(RecordType);

template<typename RecordType>
struct DataPage {
    RecordType records[N<RecordType>]; // Array of records
    long long next = -1;                  // Pointer to overflow page
    size_t n_records = 0;              // Number of records

    char *c_str() { return reinterpret_cast<char *>(this); }

    char *deserialize() { return reinterpret_cast<char *>(this); }
};

template<typename RecordType>
class ISAM {
public:
    explicit ISAM(filesystem::path _csv_path) : csv_path(std::move(_csv_path)) { initialize(); }

    bool add(RecordType new_record) {
        fstream root_index_file(index_paths[0], ios::in | ios::binary);
        IndexPage<decltype(RecordType::id)> search_page;
        size_t found_page_address = 0;
        bool found = false;
        while (!found && root_index_file.read(search_page.deserialize(), sizeof(search_page))) {
            for (int i = 0; i < search_page.n_keys; i++ && !found) {
                if (new_record.id < search_page.key[i]) {
                    found_page_address = search_page.children[i];
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            found_page_address = search_page.children[search_page.n_keys];
        }

        fstream second_index_file(index_paths[1], ios::in | ios::binary);
        IndexPage<decltype(RecordType::id)> second_index_page;
        second_index_file.seekg((streamoff) found_page_address);
        size_t found_second_page_address = 0;
        found = false;
        while (!found && second_index_file.read(second_index_page.deserialize(), sizeof(second_index_page))) {
            for (int i = 0; i < second_index_page.n_keys; i++ && !found) {
                if (new_record.id < second_index_page.key[i]) {
                    found_second_page_address = second_index_page.children[i];
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            found_second_page_address = second_index_page.children[second_index_page.n_keys];
        }

        fstream data_index_file(index_paths[2], ios::in | ios::out | ios::binary);
        DataPage<RecordType> data_page;
        data_index_file.seekg((streamoff) found_second_page_address);
        data_index_file.read(data_page.deserialize(), sizeof(data_page));
        size_t last_page_address = found_second_page_address;
        while (data_page.next != -1) {
            data_index_file.seekg(data_page.next);
            last_page_address = data_index_file.tellg();
            data_index_file.read(data_page.deserialize(), sizeof(data_page));
        }
        if (data_page.n_records == N<RecordType>) {
            DataPage<RecordType> new_data_page;
            streamoff new_data_page_address = get_free_page_address();
            new_data_page.records[0] = new_record;
            new_data_page.n_records++;
            if (new_data_page_address == -1) {
                data_index_file.seekp(0, ios::end);
            } else {
                data_index_file.seekp(new_data_page_address);
            }
            data_page.next = data_index_file.tellp();
            data_index_file.write(new_data_page.deserialize(), sizeof(new_data_page));
            data_index_file.seekp((streamoff) last_page_address);
            data_index_file.write(data_page.deserialize(), sizeof(data_page));
        } else {
            data_page.records[data_page.n_records] = new_record;
            data_page.n_records++;
            data_index_file.seekp((streamoff) last_page_address);
            data_index_file.write(data_page.deserialize(), sizeof(data_page));
        }

        //close files
        root_index_file.close();
        second_index_file.close();
        data_index_file.close();
        return true;
    }

    vector<RecordType> search(decltype(RecordType::id) search_id) {
        fstream root_index_file(index_paths[0], ios::in | ios::binary);
        IndexPage<decltype(RecordType::id)> search_page;
        size_t found_page_address = 0;
        bool found = false;
        while (!found && root_index_file.read(search_page.deserialize(), sizeof(search_page))) {
            for (size_t i = 0; i < search_page.n_keys; i++ && !found) {
                if (search_id < search_page.key[i]) {
                    found_page_address = search_page.children[i];
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            found_page_address = search_page.children[search_page.n_keys];
        }

        fstream second_index_file(index_paths[1], ios::in | ios::binary);
        IndexPage<decltype(RecordType::id)> second_index_page;
        second_index_file.seekg((streamoff) found_page_address);
        size_t found_second_page_address = 0;
        found = false;
        while (!found && second_index_file.read(second_index_page.deserialize(), sizeof(second_index_page))) {
            for (int i = 0; i < second_index_page.n_keys; i++ && !found) {
                if (search_id < second_index_page.key[i]) {
                    found_second_page_address = second_index_page.children[i];
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            found_second_page_address = second_index_page.children[second_index_page.n_keys];
        }

        fstream data_index_file(index_paths[2], ios::in | ios::binary);
        DataPage<RecordType> data_page;
        data_index_file.seekg((streamoff) found_second_page_address);
        data_index_file.read(data_page.deserialize(), sizeof(data_page));
        vector<RecordType> records;
        while (data_page.next != -1) {
            for (int i = 0; i < data_page.n_records; i++) {
                if (data_page.records[i].id == search_id) {
                    records.push_back(data_page.records[i]);
                }
            }
            data_index_file.seekg(data_page.next);
            data_index_file.read(data_page.deserialize(), sizeof(data_page));
        }
        for (int i = 0; i < data_page.n_records; i++) {
            if (data_page.records[i].id == search_id) {
                records.push_back(data_page.records[i]);
            }
        }
        //close files
        root_index_file.close();
        second_index_file.close();
        data_index_file.close();

        return records;
    }

    vector<RecordType> range_search(decltype(RecordType::id) start_id, decltype(RecordType::id) end_id) {
        fstream root_index_file(index_paths[0], ios::in | ios::binary);
        IndexPage<decltype(RecordType::id)> search_page;
        size_t found_page_address = 0;
        bool found = false;
        while (!found && root_index_file.read(search_page.deserialize(), sizeof(search_page))) {
            for (size_t i = 0; i < search_page.n_keys; i++ && !found) {
                if (start_id < search_page.key[i]) {
                    found_page_address = search_page.children[i];
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            found_page_address = search_page.children[search_page.n_keys];
        }

        fstream second_index_file(index_paths[1], ios::in | ios::binary);
        IndexPage<decltype(RecordType::id)> second_index_page;
        second_index_file.seekg((streamoff) found_page_address);
        size_t found_second_page_address = 0;
        found = false;
        while (!found && second_index_file.read(second_index_page.deserialize(), sizeof(second_index_page))) {
            for (int i = 0; i < second_index_page.n_keys; i++ && !found) {
                if (start_id < second_index_page.key[i]) {
                    found_second_page_address = second_index_page.children[i];
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            found_second_page_address = second_index_page.children[second_index_page.n_keys];
        }

        fstream data_index_file(index_paths[2], ios::in | ios::binary);
        DataPage<RecordType> data_page;
        data_index_file.seekg((streamoff) found_second_page_address);
        bool inside_range = true;
        streamoff last_page_address = found_second_page_address;
        vector<RecordType> records;
        while (inside_range && data_index_file.read(data_page.deserialize(), sizeof(data_page))) {
            for (int i = 0; i < data_page.n_records; i++) {
                if (data_page.records[i].id >= start_id && data_page.records[i].id <= end_id) {
                    records.push_back(data_page.records[i]);
                } else if (data_page.records[i].id > end_id) {
                    inside_range = false;
                    break;
                }
            }
            if (data_page.next != -1) {
                last_page_address = data_index_file.tellg();
                while (data_page.next != -1) {
                    data_index_file.seekg(data_page.next);
                    data_index_file.read(data_page.deserialize(), sizeof(data_page));
                    for (int i = 0; i < data_page.n_records; i++) {
                        if (data_page.records[i].id >= start_id && data_page.records[i].id <= end_id) {
                            records.push_back(data_page.records[i]);
                        }
                    }
                }
                data_index_file.seekg(last_page_address);
            }

        }
        //close files
        root_index_file.close();
        second_index_file.close();
        data_index_file.close();

        std::sort(records.begin(), records.end(), [](const RecordType &a, const RecordType &b) {
            return a.id < b.id;
        });

        return records;
    }

    bool remove(decltype(RecordType::id) search_id) {
        fstream root_index_file(index_paths[0], ios::in | ios::binary);
        IndexPage<decltype(RecordType::id)> search_page;
        size_t found_page_address = 0;
        bool found = false;
        while (!found && root_index_file.read(search_page.deserialize(), sizeof(search_page))) {
            for (int i = 0; i < search_page.n_keys; i++ && !found) {
                if (search_id < search_page.key[i]) {
                    found_page_address = search_page.children[i];
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            found_page_address = search_page.children[search_page.n_keys];
        }

        fstream second_index_file(index_paths[1], ios::in | ios::binary);
        IndexPage<decltype(RecordType::id)> second_index_page;
        second_index_file.seekg((streamoff) found_page_address);
        size_t found_second_page_address = 0;
        found = false;
        size_t found_id;
        while (!found && second_index_file.read(second_index_page.deserialize(), sizeof(second_index_page))) {
            for (int i = 0; i < second_index_page.n_keys; i++ && !found) {
                if (search_id < second_index_page.key[i]) {
                    found_second_page_address = second_index_page.children[i];
                    found_id = second_index_page.key[i];
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            found_second_page_address = second_index_page.children[second_index_page.n_keys];
        }

        fstream data_index_file(index_paths[2], ios::in | ios::out | ios::binary);
        DataPage<RecordType> data_page;
        data_index_file.seekg((streamoff) found_second_page_address);
        data_index_file.read(data_page.deserialize(), sizeof(data_page));
        size_t last_page_address = found_second_page_address;

        bool read_this_page = true;

        while (read_this_page) {
            for (int i = 0; i < data_page.n_records; i++) {
                if (data_page.records[i].id == search_id) {
                    data_page.records[i] = data_page.records[data_page.n_records - 1];
                    data_page.n_records--;
                    data_index_file.seekp((streamoff) last_page_address);
                    data_index_file.write(data_page.deserialize(), sizeof(data_page));
                    if (data_page.n_records == 0 && data_page.next != -1) {
                        free_page_addresses.push_back((streamoff) last_page_address);
                    }
                    root_index_file.close();
                    second_index_file.close();
                    data_index_file.close();
                    return true;
                }
            }
            if (data_page.next != -1) {
                data_index_file.seekg(data_page.next);
                last_page_address = data_index_file.tellg();
                data_index_file.read(data_page.deserialize(), sizeof(data_page));
            } else {
                read_this_page = false;
            }
        }
        //close files
        root_index_file.close();
        second_index_file.close();
        data_index_file.close();
        return false;
    }

private:
    filesystem::path csv_path;
    filesystem::path data_path; // Path to the data file
    unordered_map<int, filesystem::path>
            index_paths;  // Paths to the index files
    size_t data_size{}; // Size of the data file in bytes
    vector<streamoff> free_page_addresses; // Addresses of free pages

    streamoff get_free_page_address() {
        if (free_page_addresses.empty()) {
            return -1;
        }
        streamoff address = free_page_addresses.back();
        free_page_addresses.pop_back();
        return address;
    }


    void build_data_file() {
        vector<RecordType> records;
        fstream sorted_data_file(data_path, ios::out | ios::binary | ios::trunc);
        fstream csv_file(csv_path, ios::in);

        if (!csv_file.is_open()) {
            throw ios_base::failure("Error opening file");
        }

        string line;
        getline(csv_file, line); // Skip header
        while (getline(csv_file, line)) {
            RecordType record(line);
            records.push_back(record);
        }


        std::sort(records.begin(), records.end(), [](const RecordType &a, const RecordType &b) {
            return a.id < b.id;
        });

        if (!sorted_data_file.is_open()) {
            throw ios_base::failure("Error opening file");
        }

        sorted_data_file.write(reinterpret_cast<char *>(records.data()), records.size() * sizeof(RecordType));

        sorted_data_file.close();
    }

    void initialize() {

        if (!filesystem::exists(csv_path)) {
            throw ios_base::failure("File does not exist");
        }

        string parent_path;
        if (csv_path.has_parent_path()) {
            parent_path = csv_path.parent_path().string() + "/";
        }
        data_path = filesystem::path(parent_path + "sorted_" +
                                     csv_path.stem().string() + ".bin");

        parent_path = "";

        if (!filesystem::exists(data_path)) {
            build_data_file(); //build sorted data file from csv
        }
        fstream data_file(data_path, ios::in | ios::binary);
        for (int i = 0; i < 3; i++) {

            if (data_path.has_parent_path()) {
                parent_path = data_path.parent_path().string() + "/";
            }
            index_paths[i] =
                    filesystem::path(parent_path + "index_" + to_string(i) + "_" +
                                     data_path.filename().string());
        }

        if (!data_file) {
            throw ios_base::failure("Error opening file");
        }

        data_file.seekg(0, ios::end);
        data_size = data_file.tellg();

        if (data_size == 0) {
            throw ios_base::failure("Empty file");
        }
        data_file.close();

        if (!all_indexes_exist()) {
            build(true);
        }
    }

    // Building ISAM indexes bottom-up
    void build(bool drop_existing) {

        if (drop_existing) {
            for (int i = 0; i < 3; i++) {
                filesystem::remove(index_paths[i]);
            }
        }

        build_bottom_level();
        build_upper_levels();
    }

    // Build first and second index pages
    void build_upper_levels() {
        fstream data_index_file(index_paths[2], ios::in | ios::binary);
        fstream second_index_file(index_paths[1], ios::out | ios::binary);
        DataPage<RecordType> data_index_page;
        IndexPage<decltype(RecordType::id)> second_index_page;

        size_t data_index_page_address = 0;
        // Build second index page from the data index page
        while (data_index_file.read(data_index_page.deserialize(), sizeof(data_index_page))) {
            if (second_index_page.n_keys == M<decltype(RecordType::id)>) {
                second_index_page.children[second_index_page.n_keys] = data_index_page_address;
                second_index_file.write(second_index_page.c_str(), sizeof(second_index_page));
                second_index_page.n_keys = 0;
            }
            second_index_page.children[second_index_page.n_keys] = data_index_page_address;
            second_index_page.key[second_index_page.n_keys] = data_index_page.records[data_index_page.n_records - 1].id;
            second_index_page.n_keys++;
            data_index_page_address = data_index_file.tellg();
        }

        if (second_index_page.n_keys > 0) {
            second_index_page.children[second_index_page.n_keys] = data_index_page_address;
            second_index_file.write(second_index_page.c_str(), sizeof(second_index_page));
        }

        data_index_file.close();
        second_index_file.close();

        second_index_file.open(index_paths[1], ios::in | ios::binary);
        // Builds uppermost index page (root) from the second index page
        fstream first_index_file(index_paths[0], ios::out | ios::binary);
        IndexPage<decltype(RecordType::id)> first_index_page;
        size_t second_index_page_address = 0;
        while (second_index_file.read(second_index_page.deserialize(), sizeof(second_index_page))) {
            if (first_index_page.n_keys == M<decltype(RecordType::id)>) {
                first_index_page.children[first_index_page.n_keys] = second_index_page_address;
                first_index_file.write(first_index_page.c_str(), sizeof(first_index_page));
                first_index_page.n_keys = 0;
            }
            first_index_page.children[first_index_page.n_keys] = second_index_page_address;
            first_index_page.key[first_index_page.n_keys] = second_index_page.key[second_index_page.n_keys - 1];
            first_index_page.n_keys++;
            second_index_page_address = second_index_file.tellg();
        }

        if (first_index_page.n_keys > 0) {
            first_index_page.children[first_index_page.n_keys] = second_index_page_address;
            first_index_file.write(first_index_page.c_str(), sizeof(first_index_page));
        }

        second_index_file.close();
        first_index_file.close();
    }

    // Build the bottom level (leaves) of the ISAM index (data pages)
    void build_bottom_level() {
        fstream data_file(data_path, ios::in | ios::binary);
        fstream index_file(index_paths[2], ios::out | ios::binary);

        DataPage<RecordType> data_page;
        RecordType record;

        while (data_file.read(reinterpret_cast<char *>(&record), sizeof(record))) {
            if (data_page.n_records == N<RecordType>) {
                index_file.write(data_page.c_str(), sizeof(data_page));
                data_page.n_records = 0;
            }
            data_page.records[data_page.n_records++] = record;
        }
        if (data_page.n_records > 0) {
            index_file.write(data_page.c_str(), sizeof(data_page));
        }
    }

    // Ckecks for the existence of the files that store the indexes
    bool all_indexes_exist() {
        for (int i = 0; i < 3; i++) {
            if (!filesystem::exists(index_paths[i])) {
                return false;
            }
        }
        return true;
    }
};

#endif // DB2_PROJECT1_ISAM_H
