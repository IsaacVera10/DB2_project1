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

struct Record_ISAM {
    long id;
    char name[71];
    float punt_promedio;
    long vote_count;
    char release_date[11];
    long ganancia;
    long tiempo;
    char lang[3];

    Record_ISAM() = default;
    Record_ISAM(long _id, const char *_name, float _punt_promedio,
                long _vote_count, const char *_release_date, long _ganancia,
                long _tiempo, const char *_lang)
        : id(_id), punt_promedio(_punt_promedio), vote_count(_vote_count),
          ganancia(_ganancia), tiempo(_tiempo) {
        strcpy(name, _name);
        strcpy(release_date, _release_date);
        strcpy(lang, _lang);
    }
    void showData_line() const {
        cout << setw(8) << left << id << " | " << setw(74) << left << name
             << " | " << setw(6) << left << punt_promedio << " | " << setw(6)
             << left << vote_count << " | " << setw(11) << left << release_date
             << " | " << setw(11) << left << ganancia << " | " << setw(6)
             << left << tiempo << " | " << setw(3) << left << lang << " | ";
    }
};

////////////// Index Page ////////////////////
const int PAGE_SIZE = 512; // Page size for the index

template <typename KeyType>
static constexpr size_t M = (PAGE_SIZE - sizeof(size_t) - sizeof(size_t)) /
                            (sizeof(size_t) + sizeof(KeyType));

template <typename KeyType> struct IndexPage {
    KeyType key[M<KeyType>];           // Array of keys
    size_t children[M<KeyType> + 1]{}; // Pointers to children
    size_t n_keys = 0; // Counts the number of keys in the index page.

    char *c_str() { return reinterpret_cast<char *>(this); }
};

/////////// Data Page ////////

template <typename RecordType>
static constexpr size_t N =
    (PAGE_SIZE - sizeof(size_t) - sizeof(size_t)) / sizeof(RecordType);
template <typename RecordType> struct DataPage {
    RecordType records[N<RecordType>]; // Array of records
    size_t next = -1;                  // Pointer to overflow page
    size_t n_records = 0;              // Number of records

    char *c_str() { return reinterpret_cast<char *>(this); }
};

template <typename RecordType> class ISAM {
  public:
    ISAM(filesystem::path _data_path) : data_path(_data_path) { initialize(); }

  private:
    filesystem::path data_path; // Path to the data file
    unordered_map<int, filesystem::path>
        index_paths;  // Paths to the index files
    size_t data_size; // Size of the data file in bytes

    void initialize() {

        fstream data_file(data_path, ios::in | ios::binary);
        for (int i = 0; i < 3; i++) {
            string parent_path;
            if (data_path.has_parent_path()) {
                parent_path = data_path.parent_path().string();
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
        IndexPage<typename RecordType::id> second_index_page;

        size_t data_index_page_address = 0;
        // Build second index page from the data index page
        while (data_index_file.read(reinterpret_cast<char *>(data_index_page),
                                    sizeof(data_index_page))) {

            if (second_index_page.n_keys == M<typename RecordType::id>) {
                second_index_page.children[second_index_page.n_keys] =
                    data_index_page_address;
                second_index_file.write(second_index_page.c_str(),
                                        sizeof(second_index_page));
                second_index_page.n_keys = 0;
            }
            second_index_page.children[second_index_page.n_keys] =
                data_index_page_address;
            second_index_page.key[second_index_page.n_keys] =
                data_index_page.records[0].id;
            second_index_page.n_keys++;
        }

        if (second_index_page.n_keys > 0) {
            second_index_page.children[second_index_page.n_keys] =
                data_index_page_address;
            second_index_file.write(second_index_page.c_str(),
                                    sizeof(second_index_page));
        }

        data_index_file.close();

        // Builds uppermost index page (root) from the second index page
        fstream first_index_file(index_paths[0], ios::out | ios::binary);
        IndexPage<typename RecordType::id> first_index_page;
        size_t second_index_page_address = 0;
        while (
            second_index_file.read(reinterpret_cast<char *>(second_index_page),
                                   sizeof(second_index_page))) {
            if (first_index_page.n_keys == M<typename RecordType::id>) {
                first_index_page.children[first_index_page.n_keys] =
                    second_index_page_address;
                first_index_file.write(first_index_page.c_str(),
                                       sizeof(first_index_page));
                first_index_page.n_keys = 0;
            }
            first_index_page.children[first_index_page.n_keys] =
                second_index_page_address;
            first_index_page.key[first_index_page.n_keys] =
                second_index_page.key[0];
            first_index_page.n_keys++;
        }

        if (first_index_page.n_keys > 0) {
            first_index_page.children[first_index_page.n_keys] =
                second_index_page_address;
            first_index_file.write(first_index_page.c_str(),
                                   sizeof(first_index_page));
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

        while (
            data_file.read(reinterpret_cast<char *>(&record), sizeof(record))) {
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
