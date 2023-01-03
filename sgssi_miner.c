
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <openssl/sha.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "sgssi_miner.h"
#include "sha256calc.h"

/*
 * Función principal
 */
int main(int argc, char * argv[]){
    
    //Comprobar los argumentos
    if(argc != 4){
        printf("ERROR: Bad argument!");
        printf("\nUse: ./minero <file> <number_of_hex> <code>");
        exit(0);
    }

    //Declaración de variables
    FILE *bloque, *nuevo_bloque, *final_block;
    char *filename;

    char hex_num[THREAD_NUMBER][20], max_hex_num[20];//buffer que se va a usar para ir completanto el nonce en cada iteracion
    char hash1[THREAD_NUMBER][65], hash[THREAD_NUMBER][65], max_hash[65];//hash del bloque
    char code[9];//el codigo del alumno
    char nonces[THREAD_NUMBER][20];//el mejor nonce obtenido en cada hilo
    int count = 0, find = 0, iter = 0, max;
    int n, nonce_len=atoi(argv[2]), tid;
    long i, number_of_iterations=pow(16,atoi(argv[2]));


    char *buff, *new_buff[THREAD_NUMBER], *max_new_buff;
    
    time_t start, end;
    double dif;

    sprintf(code, " %s", argv[3]);


    filename = argv[1];                                 //Nombre del fichero
    max = 0;                                            //numero de ceros
    max--;
    int lineas = block_lines(bloque, filename);         //contar las lineas que tiene el bloque
    buff = malloc(lineas*MAX_LINE_LENGTH*sizeof(char));

    //alocar memoria para cada hilo
    for(int i = 0; i<THREAD_NUMBER; i++){
        new_buff[i] = malloc(lineas*MAX_LINE_LENGTH*sizeof(char));
    }
    
    //alocar memoria para el bloque con mas ceros
    max_new_buff = malloc(lineas*MAX_LINE_LENGTH*sizeof(char));
    copy_to_buff(bloque, filename, buff ,lineas);
    


    //---------------------- Inicio del minado del bloque -------------------------

    printf("[*] Minando bloque: %s\n", filename);

    time(&start);
    #pragma omp parallel for private(i,count, tid) firstprivate(new_buff, code, hash1, hex_num, max) shared(number_of_iterations, buff, nonces, nonce_len) num_threads(THREAD_NUMBER)
    for(i = 0; i < number_of_iterations; i++){ 
        tid = omp_get_thread_num();               

        //Hash del fichero
        strcpy(new_buff[tid], buff);//copiar el bloque al hilo
        sprintf(hex_num[tid],"%x", i);//guardar el numero hexadecimal del nonce en el hilo correspondiente
        parse_hex(hex_num[tid], nonce_len);//añadir los 0's necesarios al numero hexadecimal para que tenga los digitos necesarios
        strcat(hex_num[tid], code);//añadir el codigo del grupo o usuario al nonce
        strcat(new_buff[tid], hex_num[tid]);//añadir el nonce completo al final del bloque
        sha256(new_buff[tid], hash1[tid]);//calcular el sha256 del bloque

        //contar el numero de ceros
        count = count_zeros(hash1[tid]);//contar los 0's del hash obtenido

        //si se encuentra un hash con mas o el mismo numero de ceros se guarda el nonce obtenido en nonces[]
        if(count > max){
            max = count;
            strcpy(nonces[tid], hex_num[tid]);
        }
    }

    //una vez hemos visto todas las posibilidades, en cada hilo tenemos guardado cual ha sido el nonce que mas 0's nos ha dado
    //comprobar el resultado de cada hilo y obtener el mejor. Esto se ejecuta en SERIE
    max = 0;
    for(int i = 0; i < THREAD_NUMBER; i++){        
        strcpy(new_buff[i], buff);//copiar el bloque sin el nonce
        strcat(new_buff[i], nonces[i]);//añadir el nonce
        sha256(new_buff[i], hash1[i]);//obtener el sha256 del bloque

        //contar el numero de ceros
        count = count_zeros(hash1[i]);//contar los ceros

        //si se encuentra un hash con mas o el mismo numero de ceros establecido se guarda el sha256 de dicho bloque el nonce y el bloque entero
        if(count > max){
            max = count;
            strcpy(max_hash, hash1[i]);
            strcpy(max_hex_num, hex_num[i]);
            strcpy(max_new_buff, new_buff[i]);
        }
    }
    time(&end);
    copy_file(max_new_buff);//se guarda el bloque en un fichero
    dif = difftime(end, start);

    printf("[*] Resumen: %s\n", max_hash);//printeamos el hash del mejor bloque
    printf("[OK] Minado con exito!\n	-Numero de ceros: %d\n	-Tiempo transcurrido: %1.1f s\n", max, dif);


    //-------------------------- Fin del minado ----------------------------
    
}

void __error(int cod, char *s){
    switch(cod){
	case 0:
	    printf("******** %s *********\n", s);
	    break;
	default:
	    break;
    }
    exit(0);
}

void parse_hex(char *hex, int nonce_len){
    char *zeros = malloc((nonce_len-strlen(hex))*sizeof(char));
    for (int i = 0; i < nonce_len-strlen(hex); i++){
        zeros[i]='0';
    }
    strcat(zeros, hex);
    strcpy(hex, zeros);
    free(zeros);
       
}


/*
 * Función para copiar el contenido del bloque original al nuevo y añadir la linea con el numero hexadecimal
 */
void copy_file(char *buff){
    //Añadir la ultima linea
    FILE *f=fopen("bloque_minado.txt", "w+");
    fprintf(f, buff);
    fclose(f);
}

/*
*   Funcion para copiar el contenido del fichero a un buffer
*/
void copy_to_buff(FILE *f1, char *filename, char *buff, int lineas){
    
    char line[MAX_LINE_LENGTH];
    //Abrir el documento
    if((f1 = fopen(filename, "r")) == NULL){
	    __error(0, "No se ha encontrado el fichero 1");
    }
	
    //leer el contenido y guardarlo en buff
    fread(buff, 1, lineas*MAX_LINE_LENGTH-1, f1);

    //añadir el hezadecimal
    
    fclose(f1);
}

int block_lines(FILE *f1, char *filename){
    int line_counter = 0;
    char line[MAX_LINE_LENGTH];
    
    //Abrir el documento
    if((f1 = fopen(filename, "r")) == NULL){
	__error(0, "No se ha encontrado el fichero contador de lineas");
    }
    
    //Copiar lso documentos
    while(fgets(line, sizeof(line), f1) != NULL){
	line_counter++;
    }
    return line_counter;
}

/*
 * Función para contar los ceros que hay.
 */
int count_zeros(char hash[65]){
    int counter, i;
    counter = 0;
    if(hash[0] == '0'){
	    counter++;
        for(i = 1; i < 65; i++){
            if(hash[i] == '0')
            counter++;
            else
            break;
        }
    }
    return counter;
}
