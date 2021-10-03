#include <stdio.h>
#include <mysql.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
	MYSQL *conn;
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	int err;
	char nombre[25];
	char consulta[80];
	
	//Creamos una conexion a la base de datos
	conn = mysql_init(NULL);
	if (conn==NULL) {
		printf ("Error al crear la conexion: %u %s\n",
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	
	//Iniciamos la conexion con la base de datos
	conn = mysql_real_connect (conn, "localhost","root", "mysql", "videojuego",0, NULL, 0);
	if (conn==NULL) {
		printf ("Error al inicializar la conexion: %u %s\n",
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	
	//Consultamos al usuario su nombre y buscaremos en la BBDD su contraseña
	printf("Introduce tu nombre: \n");
	err=scanf("%s",nombre);
	if (err!=1){
		printf ("Error al introducir los datos \n");
		exit (1);
	}
	strcpy(consulta,"SELECT contrasenya FROM jugadores WHERE nombre='");
	strcat(consulta,nombre);
	strcat(consulta,"'");
	err=mysql_query (conn, consulta);
	if (err!=0) {
		printf ("Error al consultar datos de la base %u %s\n",
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	resultado = mysql_store_result (conn);
	row = mysql_fetch_row (resultado);
	
	if (row==NULL)
		printf ("El jugador especificado no esta registrado \n");
	else
		printf("La contrasenya de %s es %s",nombre,row[0]);
	
	mysql_close(conn);
	exit(0);
	
	
}

