//Consulta en C per saber la partida més llarga
#include <mysql.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
int main(int argc, char **argv)
{
	MYSQL *conn;
	int err;
	
	MYSQL_RES *resultado;
	MYSQL_ROW row;

	int partida;
	
	char consulta [80];´

	//Es crea una connexió al servidor MYSQL 

	conn = mysql_init(NULL);
	if (conn==NULL) {
		printf ("Error al crear la connexio: %u %s\n", 
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}

	//S'inicia la connexio
	conn = mysql_real_connect (conn, "localhost","user", "pass", "videojuego",0, NULL, 0);
	if (conn==NULL) {
		printf ("Error al crear la connexio: %u %s\n", 
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}

	// 

	err=mysql_query (conn, "SELECT partidas.id FROM partidas WHERE partidas.duracion = (SELECT MAX(partidas.duracion) FROM partidas)");
	if (err!=0) {
		printf ("Error al consultar les dades: %u %s\n",
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	// Es recull el resultat
	// Ha de ser un numero

	resultado = mysql_store_result (conn);
	row = mysql_fetch_row (resultado);

	if (row == NULL)
		printf ("No se han obtenido datos en la consulta\n");
	else {
		partida = atoi (row[0]);	
		printf ("La partida que mes ha durat ha estat la numero: %s\n", partida);
	
	}
			
	// Tancar la connexio amb el servidor MYSQL 
	mysql_close (conn);
	exit(0);
}
