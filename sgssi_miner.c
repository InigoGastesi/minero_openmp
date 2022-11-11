

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
    if(argc != 3){
	printf("ERROR: Bad argument!");
	printf("\nUse: ./BTC_miner <file> <number>");
	exit(0);
    }

    //Declaración de variables
    FILE *bloque, *nuevo_bloque, *final_block;
    char *filename;
    char new_filename[9] = "new_file";
    printf("%s\n", new_filename);
    char hex_num[THREAD_NUMBER][20], max_hex_num[20];
    char hash1[THREAD_NUMBER][65], hash[THREAD_NUMBER][65], max_hash[65];
    char code[9];
    char nonces[THREAD_NUMBER][20];
    int count = 0, find = 0, iter = 0, max;
    int n, nonce_len=atoi(argv[2]), tid;
    long i, number_of_iterations=pow(16,atoi(argv[2]));


    char *buff, *new_buff[THREAD_NUMBER], *max_new_buff;
    
    time_t start, end;
    double dif;
    //n = 0;

    strcpy(code, " G293132");

    filename = argv[1];                                 //Nombre del fichero
    max = 0;                                //numero de ceros
    max--;
    int lineas = block_lines(bloque, filename);         //contar las lineas que tiene el bloque
    buff = malloc(lineas*MAX_LINE_LENGTH*sizeof(char));
    //sprintf(new_filename, "new_%s", filename);          //guardar el nombre del nuevo fichero
    //sprintf(final_filename, "new_%s", filename);        //guardar el nombre del nuevo fichero
    for(int i = 0; i<THREAD_NUMBER; i++){
        new_buff[i] = malloc(lineas*MAX_LINE_LENGTH*sizeof(char));
    }
    
    max_new_buff = malloc(lineas*MAX_LINE_LENGTH*sizeof(char));
    copy_to_buff(bloque, filename, buff ,lineas);
    


    //---------------------- Inicio del minado del bloque -------------------------

    printf("[*] Minando bloque: %s\n", filename);

    time(&start);
    #pragma omp parallel for private(i,count, tid) firstprivate(new_buff, code, hash1, hex_num, max) shared(number_of_iterations, buff, nonces, nonce_len) num_threads(THREAD_NUMBER)
    for(i = 0; i < number_of_iterations; i++){ 
        tid = omp_get_thread_num();               

        //Hash del fichero
        strcpy(new_buff[tid], buff);
        sprintf(hex_num[tid],"%x", i);
        parse_hex(hex_num[tid], nonce_len);
        strcat(hex_num[tid], code);
        strcat(new_buff[tid], hex_num[tid]);
        sha256(new_buff[tid], hash1[tid]);

        //contar el numero de ceros
        count = count_zeros(hash1[tid]);

        //si se encuentra un hash con mas o el mismo numero de ceros establecido se copia el resultado a un fichero
        if(count > max){
            max = count;
            strcpy(nonces[tid], hex_num[tid]);
        }
    }
    max = 0;
    for(int i = 0; i < THREAD_NUMBER; i++){        
        //Hash del fichero
        strcpy(new_buff[i], buff);
        strcat(new_buff[i], nonces[i]);
        printf("%s\n",nonces[i]);
        sha256(new_buff[i], hash1[i]);

        //contar el numero de ceros
        count = count_zeros(hash1[i]);

        //si se encuentra un hash con mas o el mismo numero de ceros establecido se copia el resultado a un fichero
        if(count > max){
            max = count;
            strcpy(max_hash, hash1[i]);
            strcpy(max_hex_num, hex_num[i]);
            strcpy(max_new_buff, new_buff[i]);
        }
    }
    time(&end);
    copy_file(max_new_buff);
    dif = difftime(end, start);

    printf("[*] Resumen: %s\n", max_hash);
    printf("[OK] Minado con exito!\n	-Numero de ceros: %d\n	-Tiempo transcurrido: %1.1f s\n", max, dif);


    //-------------------------- Fin del minado ----------------------------
    
}

/*
 * Función para printear el error.
 */
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
 * Función para guardar un valor hexadecimal aleatorio en la variable hex
 */
void random_hex(char hex[9], int n){
    FILE * f = fopen("/dev/urandom", "rb");
    fread(&n, sizeof(int), 1, f);
    sprintf(hex, "%08x", n);
    fclose(f);
}

/*
 * Función para copiar el contenido del bloque original al nuevo y añadir la linea con el numero hexadecimal
 */
void copy_file(char *buff){
    
    
    //Añadir la ultima linea
    FILE *f=fopen("prueba.txt", "w+");
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
