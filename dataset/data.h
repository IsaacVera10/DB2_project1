#ifndef PROYECTO_1_DATA_H
#define PROYECTO_1_DATA_H

#include "csv.hpp"
#include <cstring>
#include <iomanip>
#include <iostream>

using namespace csv;

using namespace std;

//-------- Global Path --------
string data_path = "dataset/";
string bin_path = "files/";
//-------- Global Path --------

struct Record {
    int64_t id{};
    char name[71]{};
    float punt_promedio{};
    int64_t vote_count{};
    char release_date[11]{};
    int64_t ganancia{};
    int64_t tiempo{};
    char lang[3]{};

    int64_t key_value() { return id; }

    Record() = default;

    Record(int64_t id, const char *name, float punt_promedio,
           int64_t vote_count, const char *release_date, int64_t ganancia,
           int64_t tiempo, const char *lang) {
        this->id = id;
        strcpy(this->name, name);
        this->punt_promedio = punt_promedio;
        this->vote_count = vote_count;
        strcpy(this->release_date, release_date);
        this->ganancia = ganancia;
        this->tiempo = tiempo;
        strcpy(this->lang, lang);
    }

    explicit Record(const string &line) {
        string temp;
        bool inside_quotes = false;
        int current = 0;
        for (auto c: line) {
            if (c == '\"') {
                inside_quotes = !inside_quotes;
            } else if (inside_quotes) {
                temp += c;
            } else if (c == ',') {
                switch (current) {
                    case 0:
                        id = stoll(temp);
                        break;
                    case 1:
                        strcpy(name, temp.c_str());
                        break;
                    case 2:
                        punt_promedio = stof(temp);
                        break;
                    case 3:
                        vote_count = stoll(temp);
                        break;
                    case 4:
                        strcpy(release_date, temp.c_str());
                        break;
                    case 5:
                        ganancia = stoll(temp);
                        break;
                    case 6:
                        tiempo = stoll(temp);
                        break;
                    case 7:
                        strcpy(lang, temp.c_str());
                        break;
                    default:
                        break;
                }
                temp = "";
                current++;
            } else {
                temp += c;
            }
        }


    }

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

                std::strncpy(name, fields[1].c_str(), sizeof(name));
                name[sizeof(name) - 1] = '\0'; // Ensure null-termination

                punt_promedio = std::stof(fields[2]);
                vote_count = std::stoll(fields[3]);

                std::strncpy(release_date, fields[4].c_str(), sizeof(release_date));
                release_date[sizeof(release_date) - 1] = '\0'; // Ensure null-termination

                ganancia = std::stoll(fields[5]);
                tiempo = std::stoll(fields[6]);

                std::strncpy(lang, fields[7].c_str(), sizeof(lang));
                lang[sizeof(lang) - 1] = '\0'; // Ensure null-termination
            }
        }
    }
    virtual void showData_line() {
        cout << setw(8) << left << id << " | " << setw(74) << left << name
             << " | " << setw(6) << left << punt_promedio << " | " << setw(6)
             << left << vote_count << " | " << setw(11) << left << release_date
             << " | " << setw(11) << left << ganancia << " | " << setw(6)
             << left << tiempo << " | " << setw(3) << left << lang << endl;
    }
};

void records_csv_to_bin(const string &name_file, int64_t count = -1) {
    Record record;
    try {
        csv::CSVFormat format;
        format.delimiter(',');
        format.quote('"');
        format.header_row(0);

        csv::CSVReader reader(data_path + name_file, format);

        fstream file(data_path + "movie_dataset.bin",
                     ios::binary | ios::out | ios::trunc);
        if (!file.is_open())
            throw runtime_error("Error al abrir el archivo");
        for (auto &row: reader) {

            if (count > 0)
                count--;
            else if (count == 0)
                break;

            record.id = row["id"].get<int64_t>();
            strcpy(record.name, row["title"].get<string>().c_str());
            record.punt_promedio = row["vote_average"].get<float>();
            record.vote_count = row["vote_count"].get<int64_t>();
            strcpy(record.release_date,
                   row["release_date"].get<string>().c_str());
            record.ganancia = row["revenue"].get<int64_t>();
            record.tiempo = row["runtime"].get<int64_t>();
            strcpy(record.lang, row["original_language"].get<string>().c_str());
            file.write(reinterpret_cast<char *>(&record), sizeof(Record));
        }
        file.close();
    } catch (exception &e) {
        cerr << "\nError: " << e.what() << endl;
    }
}


#endif // PROYECTO_1_DATA_H
