#include <stdio.h>
#include <string.h>
#include <stdlib.h> //Necesario para atof
#include <mysql.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
	
	int sock_conn, sock_listen, ret;
	struct sockaddr_in serv_adr;
	char buff[512];
	char buff2[512];
	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	// Fem el bind al port
	
	
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// escucharemos en el port 9050
	serv_adr.sin_port = htons(9070);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	//La cola de peticiones pendientes no podr? ser superior a 4
	if (listen(sock_listen, 2) < 0)
		printf("Error en el Listen");
	
	// Bucle infinito
	for(;;){
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexi?n\n");
		//sock_conn es el socket que usaremos para este cliente
		
		//Creamos la connexión a la BBDD
		MYSQL *conn;
		MYSQL_RES *resultado;
		MYSQL_ROW row;
		int err;
		conn = mysql_init(NULL);
		if (conn == NULL){
			printf("Error al crear la connexión: %u %s\n",mysql_errno(conn),mysql_error(conn));
			exit(1);
		}
		
		conn = mysql_real_connect(conn,"localhost","root","mysql","trivial_BBDD",0,NULL,0);
		if (conn==NULL){
			printf("Error al crear la connexión: %u %s\n",mysql_errno(conn),mysql_error(conn));
			exit(1);
		}
			
		
		//Variable para saber si se tiene que desconectar porque han pulsado el boton en el cliente
		int terminar = 0;
		while (terminar==0)
		{
			// Ahora recibimos su nombre, que dejamos en buff
			ret=read(sock_conn,buff, sizeof(buff));
			printf ("Recibido\n");
			
			// Tenemos que a?adirle la marca de fin de string 
			// para que no escriba lo que hay despues en el buffer
			buff[ret]='\0';
			
			//Escribimos el nombre en la consola
			printf ("Se ha conectado: %s\n",buff);
			
			//Obtenemos el codigo que nos indica el tipo de petición.
			char *p = strtok(buff,"/");
			int codigo = atoi(p);
			
			//Codigo 0 --> Desconexión
			if (codigo == 0){
				//Mensaje que recibimos: 0/
				terminar = 1;
			}
			
			//Codigo 1 --> Comprovación para el Login
			else if (codigo == 1){
				//Mesnaje que recibimos: 1/username/contrasenya
				char nombre[25];
				char contrasenya[20];
				
				p = strtok(NULL,"/");
				strcpy(nombre,p);
				p = strtok(NULL,"/");
				strcpy(contrasenya,p);
				
				//Consulta
				char consulta[80];
				strcpy(consulta,"SELECT contrasenya FROM jugadores WHERE nombre='");
				strcat(consulta,nombre);
				strcat(consulta,"'");
				err=mysql_query(conn,consulta);
				if (err != 0){
					printf("Error al consultar la BBDD %u %s",mysql_errno(conn),mysql_error(conn));
					strcpy(buff2, "-1");
				}
				else{
					//Recibimos el resultado de la consulta
					resultado = mysql_store_result(conn);
					row = mysql_fetch_row(resultado);
					if (row == NULL)
						strcpy(buff2,"1"); //Queremos decir que no existe este usuario (error 1)
					else
					{
						if (contrasenya == row[0])
							strcpy(buff2,"0"); //Queremos decir que todo ha ido correctamente (no error)
						else
							strcpy(buff2,"2"); //Queremos decir que el usuario no coincide con la contraseña (error 2)
					}
				}
			}
			
			//Codigo 2 --> Insert de nuevos jugadores
			else if (codigo==2){ 
				//Recivimos: 2/username/contrasenya/mail
				
				char nombre[25];
				char contrasenya[20];
				char mail[100];
				p = strtok(NULL, "/");
				strcpy(nombre, p);
				p = strtok (NULL, "/");
				strcpy (contrasenya, p);
				p = strtok(NULL,"/");
				strcpy(mail, p);
				
				//Busca si ya hay un usuario con ese nombre
				char consulta[80];
				strcpy(consulta, "SELECT nombre FROM jugadores WHERE nombre='");
				strcat(consulta, nombre);
				strcat(consulta, "'");
				err = mysql_query(conn, consulta);
				
				if (err!=0)
					strcpy(buff2, "-1");
				
				else {
					resultado = mysql_store_result(conn);
					row =  mysql_fetch_row(resultado);
					//Se registra si no hay nadie
					if (row==NULL) {
						sprintf(consulta, "INSERT INTO jugadores VALUES ('%s','%s','s')", nombre, contrasenya, mail);
						err= mysql_query(conn, consulta);
						if (err!=0)
							strcpy(buff2, "-1");						
						else
							strcpy(buff2, "0"); //Retorna 0 si se ha introducido correctamente
					}
					else
						strcpy(buff2, 1);  //Retorna 1 si ya existe
					
				}
			}
			
			
			//Codigo 3 --> Recuperar contraseña 
			else if (codigo == 3){
				//Recivimos: 3/usuario
				
				char nombre[20];
				p = strtok(NULL,"/");
				strcpy(nombre,p);
				
				char consulta[80];
				strcpy(consulta,"SELECT contrasenya FROM jugadores WHERE nombre='");
				strcat(consulta,nombre);
				strcat(consulta,"'");
				
				err=mysql_query(conn,consulta);
				if (err!=0)
					strcpy(buff2,"-1");
				else{
					resultado = mysql_store_result(conn);
					row = mysql_fetch_row(resultado);
					
					if (row == NULL)
						strcpy(buff2,"1"); //No hay ningun usuario con ese nombre
					else
						strcpy(buff2,row[0]); //No hay error, devolvemos la contraseña obtenida
				}
			}
			
			//Codigo 4 --> Obtener la partida mas larga 
			else if (codigo == 4){
				//Recivimos: 4/
				
				//No tenemos ningun input por tanto procedemos a hacer la consulta directamente
				err=mysql_query (conn, "SELECT partidas.id FROM partidas WHERE partidas.duracion = (SELECT MAX(partidas.duracion) FROM partidas)");
				if (err!=0)
					strcpy(buff2,"-1");
				else{
					resultado = mysql_store_result(conn);
					row = mysql_fetch_row(resultado);
					
					if (row == NULL)
						strcpy(buff2,"1"); //Retorna 1 si no hay jugadores en la BBDD (error 1)
					else
						strcpy(buff2,row[0]); //Si todo va bien, retorna el id de la partida
				}
			}
			
			//Codigo 5 --> Obtener jugador con más puntos
			else {
				//Recivimos: 5/
				
				//No tenemos ningun input por tanto procedemos a hacer la consulta directamente
				err=mysql_query (conn, "SELECT jugadores.nombre FROM (jugadores, registro) WHERE registro.puntos=(SELECT MAX(registro.puntos) FROM registro) AND registro.idJ=jugadores.id");
				if (err!=0)
					strcpy(buff2,"-1");
				else{
					resultado = mysql_store_result(conn);
					row = mysql_fetch_row(resultado);
					
					if (row == NULL)
						strcpy(buff2,"1"); //Retorna 1 si no hay jugadores en la BBDD (error 1)
					else
						strcpy(buff2,row[0]); //Si todo va bien, retorna el nombre del jugador
				}
			}
		
			printf("Codigo: %s , Resultado: %s",codigo,buff2); //Vemos el resultado de la accion.
		}
		
		//Cerramos conexion a la BBDD
		mysql_close(conn);
		
		// Se acabo el servicio para este cliente
		close(sock_conn);
	}

	return 0;
}

