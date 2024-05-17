# Proyecto 1-BD2: Organización de Archivos
- [Introducción](#introducción)
  - [Objetivos](#objetivos)
    - [Principal](#principal)
    - [Secundarios](#secundario)
  - [Dataset](#dataset)
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

>Todo este proceso de limpieda nos dejó con un total de **913035** registros.

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

# Técnicas de indexación
## Sequential File



## ISAM
## Extendible Hashing


## Autores

|                     **Isaac Vera Romero**                   |                                 **David Torres Osorio**                                 |                       **Pedro Mori**                     |  **Leonardo Candio** |   **ESteban Vasquez**  |
|:----------------------------------------------------------------------------------:|:-----------------------------------------------------------------------------------:|:-----------------------------------------------------------------------------------:|:-----------------------------------------------------------------------------------:|:----:|
|           ![Isaac Vera Romero](https://avatars.githubusercontent.com/u/67709665?v=4)            |      ![David Torres Osorio](https://avatars.githubusercontent.com/u/63759366?v=4)       |              ![Pedro Mori](https://avatars.githubusercontent.com/u/82919499?v=4)              | ![Leonardo Candio](https://avatars.githubusercontent.com/u/75516714?v=4) | ![Esteban Vasquez](https://avatars.githubusercontent.com/u/41312479?v=4) |                                             
| <a href="https://github.com/IsaacVera10" target="_blank">`github.com/IsaacVera10`</a> | <a href="https://github.com/davidt02tech" target="_blank">`github.com/davidt02tech`</a> | <a href="https://github.com/PedroMO11" target="_blank">`github.com/PedroMO11`</a> | <a href="https://github.com/leonardocandio" target="_blank">`github.com/leonardocandio`</a>|<a href="https://github.com/MuchSquid" target="_blank">`github.com/MuchSquid`</a>|