#define MAX_LINE_LENGTH 65	    //Longitud maxima de la linea
#define THREAD_NUMBER 64

void __error(int cod, char *s);	    //Función error
void copy_file(char *buff);	    //Función para copiar contenido del fichero
void copy_to_buff(FILE *f1, char *filename, char *buff, int lineas);
void random_hex(char hex[9], int n);	//Función para calcular valor hexadecimal
int block_lines(FILE *f1, char *filemane);
int count_zeros(char hash[65]);	    //Función para contar ceros
void parse_hex(char *hex, int nonce_len);