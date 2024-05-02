import csv

# Nombre del archivo de entrada y salida
archivo_entrada = './dataset/movie_dataset.csv'
archivo_salida = 'movie_dataset_modify.csv'


def modify_file():# CUIDADO: Este script modificará el archivo CSV original
    # Índice de la columna que quieres eliminar (índice 0 para la primera columna, 1 para la segunda, y así sucesivamente)
    indice_columna_a_eliminar = 7  # Por ejemplo, eliminar la tercera columna

    # Lista para almacenar las filas modificadas
    filas_modificadas = []

    # Abre el archivo CSV en modo lectura
    with open(archivo_entrada, newline='') as csvfile:
        reader = csv.reader(csvfile)
        
        # Itera sobre las filas del archivo CSV
        for row in reader:
            # Elimina la columna deseada de cada fila
            del row[indice_columna_a_eliminar]
            # Agrega la fila modificada a la lista
            filas_modificadas.append(row)

    # Escribe las filas modificadas en un nuevo archivo CSV
    with open(archivo_salida, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile, quoting=csv.QUOTE_ALL)
        # Escribe cada fila en el archivo de salida
        for row in filas_modificadas:
            writer.writerow(row)

    print("Columna eliminada y archivo modificado creado con éxito.")

def obtener_max_longitud_columna(archivo_csv, indice_columna):
    max_longitud = 0
    dato_max_longitud = None

    with open(archivo_csv, newline='') as csvfile:
        reader = csv.reader(csvfile)
        
        # Saltar la primera fila si contiene encabezados
        next(reader, None)
        
        for row in reader:
            if len(row) > indice_columna:
                longitud_actual = len(row[indice_columna])
                if longitud_actual > max_longitud:
                    max_longitud = longitud_actual
                    dato_max_longitud = row[indice_columna]

    return dato_max_longitud, max_longitud

def obtain_date():
    indice_columna = 6 # Por ejemplo, verificar la longitud de la segunda columna

    dato_max_longitud, max_longitud = obtener_max_longitud_columna(archivo_entrada, indice_columna)
    print(f"El dato con la longitud máxima en la columna {indice_columna} es: {dato_max_longitud} con {max_longitud} caracteres.")


if __name__ == '__main__':
    obtain_date()