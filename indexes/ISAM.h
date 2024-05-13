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
    size_t n = 0; // Counts the number of keys in the index page.

    char *c_str() { return reinterpret_cast<char *>(this); }
};

/////////// Data Page ////////

template <typename DataKeyType, typename DataAddressType>
static constexpr size_t N = (PAGE_SIZE - sizeof(size_t) - sizeof(size_t)) /
                            sizeof(pair<DataKeyType, DataAddressType>);

template <typename DataKeyType, typename DataAddressType> struct DataPage {
    pair<DataKeyType, DataAddressType>
        records[N<DataKeyType, DataAddressType>]; // Array of pair <key:address>
    size_t next = -1;                             // Pointer to overflow page
    size_t n_records = 0;                         // Number of records
    //

    char *c_str() { return reinterpret_cast<char *>(this); }
};

template <typename RecordType> class ISAM {
  public:
    ISAM(filesystem::path _data_path) : data_path(_data_path) { initialize(); }

  private:
    filesystem::path data_path;
    unordered_map<int, filesystem::path> index_paths;
    size_t data_size;

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

        if (all_indexes_exist()) {
            restore();
        } else {
            build(false);
        }
    }

    void restore() {}

    // Building ISAM indexes bottom-up
    void build(bool drop) {

        if (drop) {
            for (int i = 0; i < 3; i++) {
                remove(index_paths[i]);
            }
        }

        build_bottom_level();
        build_upper_levels();
    }

    // Build first and second index pages
    void build_upper_levels() {
        fstream data_index_file(index_paths[2], ios::in | ios::binary);
        fstream second_index_file(index_paths[1], ios::out | ios::binary);
        DataPage<typename RecordType::id, size_t> data_index_page;
        IndexPage<typename RecordType::id> second_index_page;

        size_t data_index_page_address = 0;

        // TODO: Build de second index page

        while (second_index_page.n < M<typename RecordType::id>) {
            data_index_file.read(data_index_page.c_str(),
                                 sizeof(data_index_page));
            second_index_page.key[second_index_page.n] =
                data_index_page.records[0].first;
            second_index_page.children[second_index_page.n] =
                data_index_page_address;
            second_index_page.n++;
            if (data_index_page.next == -1) {
                break;
            }
            data_index_page_address = data_index_file.tellg();
        }

        // TODO:  build the first index page
    }

    // Build the bottom level (leaves) of the ISAM index (data pages)
    void build_bottom_level() {
        fstream data_file(data_path, ios::in | ios::binary);
        fstream index_file(index_paths[2], ios::out | ios::binary);

        DataPage<typename RecordType::id, size_t> data_page;

        while (data_file.tellg() < data_size) {

            while (data_page.n_records < N<size_t, size_t>) {
                typename RecordType::id read_id;
                typename RecordType::address read_address;
                read_address = static_cast<size_t>(data_file.tellg());
                if (!data_file.read(reinterpret_cast<char *>(read_id),
                                    sizeof(RecordType::id))) {
                    break;
                }
                data_page.records[data_page.n_records] =
                    make_pair(read_id, read_address);
                data_file.seekg(static_cast<size_t>(data_file.tellg()) +
                                sizeof(RecordType) - sizeof(RecordType::id));
            }
            index_file.write(data_page.c_str(), sizeof(data_page));
        }
        index_file.close();
        data_file.close();
    }

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
