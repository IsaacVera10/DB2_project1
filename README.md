# Proyecto 1-BD2: Organización de Archivos
- [Introducción](#introducción)
  - [Objetivos](#objetivos)
    - [Principal](#principal)
    - [Secundarios](#secundario)
  - [Dataset](#dataset)
    - [Generación de registros en binario](#generación-de-registros-en-binario)
- [Técnicas de indexación](#tecnicas-de-indexación)
  - [Sequential File](#sequential-file)
  - [ISAM](#isam)
  - [Extendible Hashing](#extendible-hashing)
- [Autores](#autores)

# Introducción
De lo aprendido implementaremos técnicas de organización de archivos involucrando un manejo eficiente de la memoria secundaria.
## Objetivos
### Principal
- Desarrollar e implementar en C++ las siguientes estructuras de indexación: Sequential File, ISAM, y Extendible Hashing; junto a sus operaciones completas de búsqueda, inserción, eliminación y búsqueda por rango. 
Además, debe haber un manejo eficiente de la memoria secundaria, ya que es muy costosa.

### Secundarios
- Implementar un parser y un GUI con QT para simular un SDGB.
- Analizar el comportamiento computacional de cada técnica y sus funciones implementadas.
- Identificar las ventajas y desventajas de cada técnica de organización.

## Dataset
Usamos un dataset publicado en **[Kagle](https://www.kaggle.com/datasets/asaniczka/tmdb-movies-dataset-2023-930k-movies/data)** por TMDB(The Movie Database), una base de datos en línea dedicada a la información sobre películas.

Elegimos este dataset ya que contiene alrededor de 1 000 000 de registros y 24 atributos. Sin embargo, para el correcto funcionamiento de nuestros algoritmos y la integridad de nuestro proyecto hemos hecho las siguientes modificaciones con ayuda de un script en Python:
- **Eliminación de las películas para adultos**: Eliminamos todos los registros cuyo atributo booleano *adult* era `true`. Estas películas representaban el 9% del total de registros.
- **Eliminación de caracteres especiales**: Eliminamos los caracteres especiales de los atributos *title*, que generaban problemas en la lectura e impresión de los datos. Fue un trabajo manual y de prueba-error, ya que algunos caracteres especiales no se eliminaban correctamente con el script y otros no generaban problemas.

>Todo este proceso de limpieza nos dejó con un total de **913035** registros.

También hemos eliminado algunos atributos que no consideramos relevantes para nuestro proyecto, y nos quedamos con *8* atributos:
 ```c++
 struct Record{
    int64_t id{};
    char name[71]{};
    float punt_promedio{};
    int64_t vote_count{};
    char release_date[11]{};
    int64_t ganancia{};
    int64_t tiempo{};
    char lang[3]{};
 }
 ```
|    **Campo**    |                            **Descripción**                                          | 
|:---------------:|:-----------------------------------------------------------------------------------:|
| ```id```        | Identificador único para cada película.             |  
| ```name```  | Título de la película respectiva. | 
| ```punt_promedio``` | Puntaje promedio de la película [0-10]. |
| ```vote_count``` | Recuento total de votos recibidos para la película.|
| ```release_date``` | Fecha en que se estrenó la película. |
| ```ganancia``` | Ingresos totales generados por la película. |
| ```tiempo``` | Duración de la película en minutos |
| ```lang``` | Idioma original(abreviatura) en el que se produjo la película. |

### Generación de registros en binario
Para la generación de registros en binario, en el archivo [`data.h`](https://github.com/IsaacVera10/DB2_project1/blob/main/dataset/data.h) de la carpeta [**dataset**](https://github.com/IsaacVera10/DB2_project1/tree/main/dataset) se encuentra la función `records_csv_to_bin(const string&, int64_t)`, que con ayuda de la librería construida en [`csv.hpp`](https://github.com/IsaacVera10/DB2_project1/blob/main/dataset/csv.hpp), se encarga de leer el archivo .csv y escribir los registros en un archivo binario.
  
  ```c++
  void records_csv_to_bin(const string& route_file, int64_t count = -1){
    Record record;
    try {
        csv::CSVFormat format;
        format.delimiter(',');
        format.quote('"');
        format.header_row(0);

        csv::CSVReader reader("./"+route_file, format);

        fstream file("./dataset/movie_dataset.bin", ios::binary | ios::out | ios::trunc);
        if(!file.is_open()) throw runtime_error("Error al abrir el archivo");
        for(auto& row : reader){
            if(count>0) count --;
            else if(count == 0) break;
            
            record.id = row["id"].get<int64_t>();
            strcpy(record.name, row["title"].get<string>().c_str());
            record.punt_promedio = row["vote_average"].get<float>();
            record.vote_count = row["vote_count"].get<int64_t>();
            strcpy(record.release_date, row["release_date"].get<string>().c_str());
            record.ganancia = row["revenue"].get<int64_t>();
            record.tiempo = row["runtime"].get<int64_t>();
            strcpy(record.lang, row["original_language"].get<string>().c_str());
            file.write(reinterpret_cast<char*>(&record), sizeof(Record));
        }
        file.close();
    }
    catch(exception& e){
        cerr<<"\nError: "<<e.what()<<endl;
    }

}
```
En el archivo `movie_dataset.bin` se encontrán los registros en binario, listos para ser leídos por nuestros algoritmos de indexación.


# Técnicas de indexación
## Sequential File
Esta técnica de organización se caracteriza, principalmente, por su forma de organizar y almacenar los registros en un archivo de forma secuencial, es decir, uno detrás de otro. Cada record tiene un puntero que apunta al siguiente registro (***posición lógica***), y el último registro apunta a un valor nulo.

<div style="text-align: center;">
    <img src="images/sequential_file.png" alt="Sequential File: Estrategia 1" width = 250"/>
</div>


>**Implementación**: Para la implementación de esta técnica, hemos elegido la ***Estrategia 1*** planteada en clases, que se refiere al uso de dos archivos: uno llamado  `data.bin` que contiene los registros ordenados por el *key* y otro llamado `aux.bin` que es el archivo donde insertaremos los registros nuevos como un Heap File pero estarán ordenados por sus respectivos punteros.


### Elementos de ayuda
Para la implementación del sequential file, nos ayudamos de algunos elementos que facilitan la manipulación e implementación de las funciones de búsqueda, inserción, eliminación y búsqueda por rango.

#### Estrutura Hija de Record: *Record_SFile*
Cuando indexemos los registros con esta estructura, cada registro necesitará de un puntero que apunte a la siguiente posición lógica en el archivo. Para esto, creamos una estructura hija de `Record` que contenga estos elementos extras.
```c++
struct Record_SFile : public Record {
    int64_t punt_nextPosLogic = -1; // 8 bits
    bool punt_next_is_In_Data = false; // 1 bit
    
    Record_SFile(int64_t id, const char* name, float punt_promedio, int64_t vote_count, const char* release_date, int64_t ganancia, int64_t tiempo, const char* lang)
        : Record(id, name, punt_promedio, vote_count, release_date, ganancia, tiempo, lang) {}
    ...
};
```
#### Variables globales
Hemos hecho uso de vairables globales para evitar la creación constante de diversas variables en las funciones a implementar. Estas variables se encuentran encapsuladas en un *namespace* llamado `var_temps_SF`, algunas de las importantes son:
 - `punt_pos`: De tipo `int64_t`, almacenará la posición lógica de un regitro.
 - `punt_is_in_data`: De tipo `bool`, indicará si el registro de la posición lógica `punt_pos` se encuentra en el archivo `data.bin` o no(en `aux.bin`).
 - `u_before`: De tipo `int64_t`, si es que el registro a *insertar* no existe, almacenará la posición lógica del que debería ser su antecesor. También ayudará en las funciones *search* y *range_search*.

 - `u_before_is_in_data`: De tipo `bool`, indicará si el registro de la posición lógica `u_before` se encuentra en el archivo `data.bin` o no(en `aux.bin`).
 - `rec_temp`: De tipo `Record_SFile`, almacenará temporalmente un registro para realizar operaciones con él.

### `Class Sequential_File`: Funciones privadas
Implementamos algunas funciones privadas que nos ayudarán a realizar código recurrente en las funciones públicas de la clase `Sequential_File`.
####
- **`int64_t get_pos_logical(int64_t, bool)`**: Dado un valor de tipo `int64_t`(posición física) y un valor de tipo `bool`(si el registro está en `data.bin` o no), nos devolverá la posición lógica del registro en el archivo.
- **`int64_t get_pos_fisica(int64_t, bool)`**: Dado un valor de tipo int64_t(posición lógica) y un valor de tipo bool(si el registro está en data.bin o no), nos devolverá la posición física del registro en el archivo.
- **`bool binary_search(int64_t key)`**: Realiza una búsqueda binaria en el archivo `data.bin` para encontrar un registro con un *key* específico. 
  * Si no lo encuentra, nos devolverá `false` y la variable `u_before` almacenará la posición lógica del registro que debería ser su antecesor. `u_before` será -1 si el registro a buscar es menor que el primer registro.
  * Si lo encuentra, nos devolverá `true` y la variable `punt_pos` almacenará la posición lógica del registro.

- **`get_u_before(int64_t, fstream&, fstream&)`**: Dado un valor de tipo `int64_t` que es la **key** de un registro, nos devolverá la posición lógica del registro que debería ser su antecesor almacenado en la variable global `u_before` y `u_before_is_in_data` dicho registro se encuentra en `data.bin` o no. 
Esta función solo funcionará en las siguiente funciones:
  * `insert(Record_SFile&)`: El record que se quiere insertar no existe.
  * `range_search(int64_t, int64_t)`: No nos interesa si el registro existe o no, solo queremos obtener los records límites:
    - Si el key límite inferior o superior existen, la **búsqueda binaria** seteará a `u_before` al record encontrado, si no existe, `u_before` será el record que debería ser su antecesor.

## ISAM


## Extendible Hashing

El Extendible Hashing es un método de hashing dinámico que se ajusta a la cantidad de datos existentes. Es decir, el tamaño del achivo donde se aplica la técnica crece o se reduce en el tiempo. 

<div style="text-align: center;">
    <img src="images/ext_hashing.png" alt="Extendible Hashing: Estrategia 3" width = 250"/>
</div>

Las principales características del Extendible Hashing son las siguientes:

- Directorios: Los directorios guardan punteros hacia las posiciones físicas de los buckets. La cantidad de directorios está definido por la profundidad global D que se define como hiperparámetro que es igual a 2^D. La función hash que se utiliza identifica el directorio al cual ubicarse.
- Buckets: Estos pueden contener los keys con el cual se identifican los registros o los registros en sí mismos.
- Profundidad global: Denota el número de bits que utiliza la función hash para identificar un directorio.
- Profundidad local: Es igual a la profundidad global solo que está asociado a los buckets y no a los directorios. Se utiliza este dato para decidir el momento de hacer un split o un encadenamiento de buckets.

Su implementación en este proyecto se hace principalmente en dos archivos: Uno para los directorios y otro para los buckets. Algo importante a mencionar es que al inicializar la clase ExtHashing se crean los 2^D directorios para aminorar las complejidades de inserción, búsqueda y eliminación.

### Métodos importantes

1. Insertar registro: Dado que el archivo de directorios ya fue construido inicialmente, en este punto solo usamos la función hash para ubicar el directorio que nos dará el puntero del bucket al que le corresponde el registro en el archivo de datos. De acuerdo con la capacidad del bucket FB y su profundidad local, se hacen las siguientes operaciones:
   
   - Inserta el registro en el bucket
   - Se hace split del bucket. Es decir, se construye un nuevo bucket y se redistribuyen los registros entre el bucket actual y el nuevo bucket.
   - Se hacer encadenamiento de buckets.
   
```cpp
void writeRecord(Record<T> record) {
        fstream dataFile(this->fileName, ios::binary | ios::out | ios::in);

        if(!dataFile.is_open())
            throw runtime_error("Error opening data file");

        //Locate bucket where record will be inserted
        size_t hashValue = std::hash<T>{}(record.getKey());
        //cout << "hashValue: " << hashValue << endl;
        int index = hashValue % static_cast<int>(pow(2, D));

        long bucketAddress = this->indexVector[index].bucketAddress;

        //Read bucket from disk
        Bucket<T> bucket;
        dataFile.seekg(bucketAddress, ios::beg);
        dataFile.read(reinterpret_cast<char*>(&bucket), sizeof(bucket));

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
                    int bucketNum = this->makeAddress(binaryKey, bucket.depth);

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
```

También se hace uso de una función auxiliar makeAddress(), la cual recibe un número binario en string y retorna su representación en entero de los bits menos significativos de acuerdo a la profundidad que se indique. La función iene la siguiente implementación:

```cpp
    int makeAddress(string binaryString, int depth) {
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

```
La complejidad de este método en el peor caso es O(FB + 2^D).

2. Buscar registro: Busca el directorio correspondiente a la key del registro luego de aplicar la función hash, con la dirección de allí se busca en el archivo de datos para traer a memoria RAM el bucket. Busca dentro del bucket inicial o entre lo buckets encadenados y retorna el registro. Caso contrario lanza una excepción de que no encontró el registro. La complejidad de este método en el peor caso es O(FB).

```cpp
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
            dataFile.seekg(bucket.next_bucket, ios::beg);
            dataFile.read(reinterpret_cast<char*>(&bucket), sizeof(bucket));
            record = this->searchInBucket(bucket, key);
        } while(bucket.next_bucket != -1 && record == nullptr);

        dataFile.close();

        if (record == nullptr)
            throw runtime_error("Key not found in data file");

        Record rObj = (*record);
        return rObj;
    }
```
   
3. Eliminar registro: Ubica el directorio correspondiente al valor obtenido luego de aplicar la función hash a la key del record a eliminar. Busca en los registros dentro del bucket correspondiente o los buckets encadenados y si lo encuentra, lo elimina moviendo todos los registros a la izquierda y disminuye el size del bucket. Luego de eso verifica si el bucket quedó vacío, si es así verifica también si se trata de un bucket encadenado o no. En caso no lo sea, aplica el merge con su bucket amigo (tienen mismo prefijo en la profundidad local anterior). En caso sí lo sea, copia los registros del siguiente bucket encadenado al bucket actual. Para el merge, actualiza el archivo de directorios y el vector de directorios en RAM. La complejidad de este método en el peor caso es O(FB + 2^D).

```cpp

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

```
# SQL Parser

La implementación del parser SQL sigue los pasos de un compilador tradicional: análisis léxico y análisis sintáctico. En el análisis léxico tenemos un scanner que se encarga de generar tokens a partir del texto en SQL. Aquí se reconocen las palabras clave como **(SELECT, INSERT, DELETE, CREATE)**, identificadores **(Campos o ID's)**, operadores y valores. Luego, en el análisis sintáctico tenemos un parser que se encarga de analizar secuencialmente estos tokens, siguiendo las reglas definidas por la gramática que se muestra a continuación:

```
StmtList ::= ( SelectStmt | InsertStmt | CreateStmt
            | DeleteStmt ) "USING"

SelectStmt ::= "SELECT" colum-list "FROM" 
              table-name [where-clause]

where-clause ::= "WHERE" condition

condition ::= expression (comparison-operator expression)

expression ::= column-name | literal

comparison-operator ::= "=" | "!=" | ">" | "<" 
                      | ">=" | "<="
```

## Consultas

| **Consulta** | **Sentencia** | 
|--------------|--------------|
| `SELECT`  | SELECT * FROM table WHERE column = value | 
| `SELECT`  | SELECT * FROM table WHERE column BETWEEN value1 AND value2  | 
| `CREATE` | CREATE INDEX nameIndex ON column USING typeIndex | 
| `INSERT` |INSERT INTO table VALUES (value1,value2,...,valuen) |
| `DELETE` | DELETE FROM table WHERE column = value |


# Autores

|                     **Isaac Vera Romero**                   |                                 **David Torres Osorio**                                 |                       **Pedro Mori**                     |  **Leonardo Candio** |   **Esteban Vasquez**  |
|:----------------------------------------------------------------------------------:|:-----------------------------------------------------------------------------------:|:-----------------------------------------------------------------------------------:|:-----------------------------------------------------------------------------------:|:----:|
|           ![Isaac Vera Romero](https://avatars.githubusercontent.com/u/67709665?v=4)            |      ![David Torres Osorio](https://avatars.githubusercontent.com/u/63759366?v=4)       |              ![Pedro Mori](https://avatars.githubusercontent.com/u/82919499?v=4)              | ![Leonardo Candio](https://avatars.githubusercontent.com/u/75516714?v=4) | ![Esteban Vasquez](https://avatars.githubusercontent.com/u/41312479?v=4) |                                             
| <a href="https://github.com/IsaacVera10" target="_blank">`github.com/IsaacVera10`</a> | <a href="https://github.com/davidt02tech" target="_blank">`github.com/davidt02tech`</a> | <a href="https://github.com/PedroMO11" target="_blank">`github.com/PedroMO11`</a> | <a href="https://github.com/leonardocandio" target="_blank">`github.com/leonardocandio`</a>|<a href="https://github.com/MuchSquid" target="_blank">`github.com/MuchSquid`</a>|
