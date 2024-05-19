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


# Autores

|                     **Isaac Vera Romero**                   |                                 **David Torres Osorio**                                 |                       **Pedro Mori**                     |  **Leonardo Candio** |   **Esteban Vasquez**  |
|:----------------------------------------------------------------------------------:|:-----------------------------------------------------------------------------------:|:-----------------------------------------------------------------------------------:|:-----------------------------------------------------------------------------------:|:----:|
|           ![Isaac Vera Romero](https://avatars.githubusercontent.com/u/67709665?v=4)            |      ![David Torres Osorio](https://avatars.githubusercontent.com/u/63759366?v=4)       |              ![Pedro Mori](https://avatars.githubusercontent.com/u/82919499?v=4)              | ![Leonardo Candio](https://avatars.githubusercontent.com/u/75516714?v=4) | ![Esteban Vasquez](https://avatars.githubusercontent.com/u/41312479?v=4) |                                             
| <a href="https://github.com/IsaacVera10" target="_blank">`github.com/IsaacVera10`</a> | <a href="https://github.com/davidt02tech" target="_blank">`github.com/davidt02tech`</a> | <a href="https://github.com/PedroMO11" target="_blank">`github.com/PedroMO11`</a> | <a href="https://github.com/leonardocandio" target="_blank">`github.com/leonardocandio`</a>|<a href="https://github.com/MuchSquid" target="_blank">`github.com/MuchSquid`</a>|