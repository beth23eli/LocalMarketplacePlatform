#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h> 
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>

/* portul folosit */
#define PORT 2024
/* codul de eroare returnat de anumite apeluri */
extern int errno;

int conectare(sqlite3 *db, int client, char *msgrasp, char* parola);
int inregistrare();
int meniu_intern(int client, int nr, sqlite3* db);

int log_status = 0;
char msgrasp[100]=" "; //mesaj de raspuns pentru client
char msg[100];		//mesajul primit de la client
char user[20] = ""; //username
int id_user;

int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    int sd;			//descriptorul de socket

    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror ("[server]Eroare la socket().\n");
    	return errno;
    }

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
    	perror ("[server]Eroare la bind().\n");
    	return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 1) == -1)
    {
    	perror ("[server]Eroare la listen().\n");
    	return errno;
    }
	
	//ma conectez la baza de date
	sqlite3 *db;
	int rc;

	rc = sqlite3_open("marketplace.db", &db);
	if(rc) 
	{
		fprintf(stderr, "Baza de date nu a putut fi deschisa: %s\n", sqlite3_errmsg(db));
		return(0);
	} 
	else fprintf(stderr, "Baza de date a fost deschisa cu succes!\n");
			

    /* servim in mod concurent clientii... */
    while (1)
    {
    	int client;
    	socklen_t length = sizeof(from);

    	printf ("[server]Asteptam la portul %d...\n",PORT);
    	fflush (stdout); //golim bufferul

    	/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    	client = accept (sd, (struct sockaddr *) &from, &length);

    	/* eroare la acceptarea conexiunii de la un client */
    	if (client < 0)
    	{
    		perror ("[server]Eroare la accept().\n");
    		continue;
    	}

    	int pid;
    	if ((pid = fork()) == -1) 
		{
    		close(client);
    		continue;
    	} 
		else if (pid > 0) // parinte
		{ 
    		close(client);
    		while(waitpid(-1,NULL,WNOHANG));
    		continue;
    	} 
		else if (pid == 0) // copil
		{
			label1: 
    		close(sd);

			bzero (msg, 100);
			printf ("[server]Asteptam mesajul...\n");
			fflush (stdout);

			// citirea mesajului - nr comenzii din meniul de intrare
			if (read (client, msg, 100) <= 0)
			{
				perror ("[server]Eroare la read() de la client.\n");
				close (client);	// inchidem conexiunea cu clientul 
				continue;		// continuam sa ascultam 
			}
			msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
			printf ("[server]Mesajul a fost receptionat...%s\n", msg); 

			char parola[20];
			char nume[20];
			if(strstr(msg, "1") != NULL) //conectare
			{
				bzero(msg, 100);
				strcpy(msgrasp, "Introduceti username-ul pentru logare:");
				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					continue;		/* continuam sa ascultam */
				}
				else printf ("[server]Mesajul a fost transmis cu succes.\n");

				// citirea username-ului trimis de client
				bzero(msg, 100);
				if (read (client, msg, 100) <= 0)
				{
					perror ("[server]Eroare la read() de la client.\n");
					close (client);	// inchidem conexiunea cu clientul 
					continue;		// continuam sa ascultam 
				}
				printf ("[server]Mesajul a fost receptionat...%s\n", msg); 
				msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
				strcpy(nume, msg);

				bzero(msg, 100);
				strcpy(msgrasp, "Introduceti parola pentru logare:");
				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					continue;		/* continuam sa ascultam */
				}
				else printf ("[server]Mesajul a fost transmis cu succes.\n");

				// citirea parolei trimis de client
				bzero(msg, 100);
				if (read (client, msg, 100) <= 0)
				{
					perror ("[server]Eroare la read() de la client.\n");
					close (client);	// inchidem conexiunea cu clientul 
					continue;		// continuam sa ascultam 
				}
				printf ("[server]Mesajul a fost receptionat...%s\n", msg);
				msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
				strcpy(parola, msg);

				if (conectare(db, client, nume, parola)) //daca se conecteaza
				{
					log_status = 1;
				}
				else //daca introduce un username ce nu e inregistrat
				{
					log_status = 0;
					goto label1;
				}
			}
			else if(strstr(msg, "2") != NULL) //inregistrare
			{
				bzero(msg, 100);
				strcpy(msgrasp, "Introduceti username-ul pentru inregistrare:");

				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					continue;		/* continuam sa ascultam */
				}
				else printf ("[server]Mesajul a fost transmis cu succes.\n");

				// citirea mesajului - username ul introdus
				bzero(msg, 100);
				if (read (client, msg, 100) <= 0)
				{
					perror ("[server]Eroare la read() de la client.\n");
					close (client);	// inchidem conexiunea cu clientul 
					continue;		// continuam sa ascultam 
				}
				printf ("[server]Mesajul a fost receptionat...%s\n", msg); 
				msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
				strcpy(nume, msg);

				/* returnam mesajul clientului */
				bzero(msg, 100);
				strcpy(msgrasp, "Introduceti parola pentru inregistrare:");
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					continue;		/* continuam sa ascultam */
				}
				else printf ("[server]Mesajul a fost transmis cu succes.\n");

				// citirea parolei trimis de client
				bzero(msg, 100);
				if (read (client, msg, 100) <= 0)
				{
					perror ("[server]Eroare la read() de la client.\n");
					close (client);	// inchidem conexiunea cu clientul 
					continue;		// continuam sa ascultam 
				}
				printf ("[server]Mesajul a fost receptionat...%s\n", msg);
				msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
				strcpy(parola, msg);

				if (inregistrare(db, client, nume, parola)) //daca se inregistreaza
				{
					log_status = 1;
				}
				else //daca introduce un username ce nu e inregistrat
				{
					log_status = 0;
					goto label1;
				}				
			}
			else if(strstr(msg, "4") != NULL)
			{
				char about[600];
				strcpy(about, "\nBun venit pe LocalMarketplacePlatform! Aici, cumpărăturile locale devin o experiență simplă și plăcută.\n\n");
				strcat(about, "Cum sa utilizati:\n \nÎnregistrare/Logare: Creați un cont sau autentificați-vă cu un nume de utilizator unic.\n");
				strcat(about, "Vizualizare Oferte: Explorați și descoperiți o gamă diversă de produse.\n");
				strcat(about, "Gestionare Oferte: Adăugați, modificați sau ștergeți oferte cu ușurință.\n");
				strcat(about, "Căutare Produse: Găsiți rapid produsele dorite folosind funcția de căutare.\n");
				strcat(about, "Realizare de Comenzi Rapid: Alegeti ofertele preferate si realizati o comanda.\n");

				/* returnam mesajul clientului */
				if (write (client, about, 600) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					continue;		/* continuam sa ascultam */
				}	
				goto label1;
			}
			else // a 3 comanda
			{
				strcpy(msgrasp, "Ati parasit aplicatia cu succes!");
				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					continue;		/* continuam sa ascultam */
				}	
				break;
			}

			//extrag id_user folosind username-ul
			char sql[100];

			sprintf(sql, "SELECT id_user FROM useri WHERE username='%s';", user);
			char **result;
			int rows, columns;

			rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
			if (rc != SQLITE_OK) 
			{
				fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return rc;
			}

			// Verificăm dacă avem cel puțin o rând în rezultate
			if (rows > 0) {
				// Valoarea id_user este stocată în prima celulă (0, 0) a tabelului result
				id_user = atoi(result[1]);

				printf("ID_user: %d\n", id_user);
			} else {
				printf("Utilizatorul nu a fost găsit.\n");
			}
			// Eliberăm memoria rezultatelor
			sqlite3_free_table(result);

			///dupa conectare sau inregistrare cu un nou username, clientul va fi conectat --- de aici apare meniul intern de comenzi
			if(log_status)
			{
				label2:
				strcpy(msg, "");
				//citesc nr comenzii din al doilea meniu
				if (read (client, msg, 100) <= 0)
				{
					perror ("[server]Eroare la read() de la client.\n");
					close (client);	// inchidem conexiunea cu clientul 
					continue;		// continuam sa ascultam 
				}
				msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
				printf ("[server]Mesajul a fost receptionat...%s\n", msg); 
				printf("%s\n", msg);

				//intru in meniu cu comanda transmisa
				int nr = atoi(msg);
				int rez = meniu_intern(client, nr, db);

				if(rez) goto label2; //apare iarasi meniul intern
				else if(rez == 2) {break; } //iesire
				else goto label1; //apare meniul de la intrare
			}
			else //nu e logat --- trebuie sa afisez din nou meniul de conectare
			{
				bzero(msg, 100);
				if (read (client, msg, 100) <= 0)
				{
					perror ("[server]Eroare la read() de la client.\n");
					close (client);	// inchidem conexiunea cu clientul 
					continue;		// continuam sa ascultam 
				}
				msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
				printf ("[server]Mesajul a fost receptionat...%s\n", msg); 

				/* returnam mesajul - log_statusul clientului */
				sprintf(msgrasp, "%d", log_status);
				if (write (client, msgrasp, 100) <= 0) 
				{
					perror ("[server]Eroare la write() catre client.\n");
					return 0;		/* continuam sa ascultam */
				}

				goto label1;
			}

    		/* am terminat cu acest client, inchidem conexiunea */
    		close (client);
    		exit(0);
    	}
		//inchid baza de date
		sqlite3_close(db);	
    } 
} 

int inregistrare(sqlite3 *db, int client, char *msgrasp, char *parola)
{
	int rc;
	char msg[100] = "";

	strcpy(user, msgrasp); //username-ul

	char sql[100];
    sprintf(sql, "SELECT * FROM useri WHERE username='%s' AND password='%s';", user, parola);
	char **result;
	int rows, columns;

	rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return rc;
	}

	if(rows > 0) //daca username-ul este gasit in tabela cu useri
	{
		bzero(msg, 100);
		strcpy(msgrasp, "Exista deja un cont cu acest username!");

		/* returnam mesajul clientului */
		if (write (client, msgrasp, 100) <= 0)
		{
			perror ("[server]Eroare la write() catre client.\n");
			return 0;
		}
		// Eliberați memoria rezultatelor
		sqlite3_free_table(result);

		return 0;		
	}
	else //userul nu e gasit, il adaug in tabela useri
	{
		sprintf(sql, "INSERT INTO useri (username, password) VALUES ('%s', '%s');", user, parola);

		rc = sqlite3_exec(db, sql, 0, 0, 0);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "Eroare la inserare: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			return rc;
		}

		/* returnam mesajul clientului */
		strcpy(msgrasp, "V-ati inregistrat cu succes!");
		if (write (client, msgrasp, 100) <= 0)
		{
			perror ("[server]Eroare la write() catre client.\n");
			return 0;		/* continuam sa ascultam */
		}
		printf("Inserare reusita!\n");

		// Eliberați memoria rezultatelor
		sqlite3_free_table(result);

		return 1;
	}
}

int conectare(sqlite3 *db, int client, char *msgrasp, char *parola)
{
	int rc;
	char sql[100];
	char msg[100] = "";
	
	strcpy(user, msgrasp); //username-ul

    sprintf(sql, "SELECT * FROM useri WHERE username='%s' AND password='%s';", user, parola);
	char **result;
	int rows, columns;

	rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return rc;
	}

	if(rows > 0) //daca username-ul si parola sunt gasite in tabela cu useri
	{
		log_status = 1;
		
		bzero(msg, 100);
		strcpy(msgrasp, "V-ati conectat cu succes!");

		/* returnam mesajul clientului */
		if (write (client, msgrasp, 100) <= 0)
		{
			perror ("[server]Eroare la write() catre client.\n");
			return 0;	
		}
		// Eliberați memoria rezultatelor
		sqlite3_free_table(result);

		return 1;
	}
	else //userul si parola nu sunt gasite
	{
		bzero(msg, 100);
        strcpy(msgrasp, "Nu sunteti inregistrat cu acest username sau parola gresita!");

		/* returnam mesajul clientului */
		if (write (client, msgrasp, 100) <= 0)
		{
			perror ("[server]Eroare la write() catre client.\n");
			return 0;		/* continuam sa ascultam */
		}
		// Eliberați memoria rezultatelor
		sqlite3_free_table(result);

		return 0;
	}
}

int meniu_intern(int client, int nr, sqlite3* db)
{
	printf("cmd_nr este %d\n", nr);
	switch(nr)
	{
		case 1 : 
		{
			int rc;
			char sql[100];

			sprintf(sql, "SELECT id_oferta, titlu, descriere, pret FROM oferte;");
			char **result;
			int rows, columns;

			rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
			if (rc != SQLITE_OK) {
				fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return rc;
			}

			/* returnam ofertele disponibile clientului */
			strcpy(msgrasp, "");
			int index;
			for (int i = 1; i <= rows; i++) {
				for (int j = 0; j < columns; j++) {
					index = i * columns + j;
					strcat(msgrasp, result[index]);
					if (j < columns - 1) {
						strcat(msgrasp, " "); // Adaugă un spațiu între id_oferta și titlu_oferta, doar dacă nu suntem la ultima coloană
					}
				}
				strcat(msgrasp, "\n"); // Adaugă un caracter newline între fiecare pereche id_oferta și titlu_oferta
			}
			if (write (client, msgrasp, 100) <= 0)
			{
				perror ("[server]Eroare la write() catre client.\n");
			}

			//citesc nr comenzii ofertei introdus
			strcpy(msg, "");
			if (read (client, msg, 100) <= 0)
			{
				perror ("[server]Eroare la read() de la client.\n");
				close (client);	// inchidem conexiunea cu clientul 
			}
			msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
			printf ("[server]Mesajul a fost receptionat...%s\n", msg); 

			int nr = atoi(msg); //nr comenzii
			switch(nr)
			{
				case 1 : //comanda de a adauga in cosul de comenzi
				{
					// Verificăm dacă există oferte disponibile înainte de a cere introducerea id-ului
					char count_sql[100];
					sprintf(count_sql, "SELECT COUNT(*) FROM oferte;");
					char **count_result;
					int count_rows, count_columns;

					int count_rc = sqlite3_get_table(db, count_sql, &count_result, &count_rows, &count_columns, 0);

					if (count_rc != SQLITE_OK)
					{
						fprintf(stderr, "Eroare la execuția interogării SQL pentru numărul de oferte: %s\n", sqlite3_errmsg(db));
						sqlite3_close(db);
						return count_rc;
					}

					int offer_count = atoi(count_result[1]); // Assuming the count is in the second column
					sqlite3_free_table(count_result);

					if (offer_count > 0) //daca sunt oferte pe platforma
					{
						/* returnam mesajul clientului */
						strcpy(msgrasp, "Alegeti id-ul ofertei din lista de mai sus ce doriti sa o comandati:");
						if (write (client, msgrasp, 100) <= 0)
						{
							perror ("[server]Eroare la write() catre client.\n");
							return 0;		/* continuam sa ascultam */
						}

						//citesc id-ul ofertei introdus
						strcpy(msg, "");
						if (read (client, msg, 100) <= 0)
						{
							perror ("[server]Eroare la read() de la client.\n");
							close (client);	// inchidem conexiunea cu clientul 
						}
						msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
						printf ("[server]Mesajul a fost receptionat...%s\n", msg); 

						// Verificăm dacă id-ul introdus există în tabelul "oferte"
						char sql[200];
						sprintf(sql, "SELECT * FROM oferte WHERE id_oferta='%s';", msg);
						char **result;
						int rows, columns;
						int rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);

						if (rc != SQLITE_OK)
						{
							fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
							sqlite3_close(db);
							return rc;
						}

						if (rows > 0) //daca se gaseste oferta cu id-ul introdus
						{
							// Detaliile ofertei
							char id_oferta[10];
							char titlu[100];
							char descriere[200];
							char pret[20];
							char user_id[10];

							int index = columns; // prima linie a rezultatelor
							strcpy(id_oferta, result[index]);
							strcpy(titlu, result[index + 2]);
							strcpy(descriere, result[index + 3]);
							strcpy(pret, result[index + 4]);

							//verific daca userul nu e acelasi cu cel care a creat-o
							//inainte sa cumpere
							sprintf(sql, "SELECT id_user FROM oferte WHERE id_oferta='%s';", id_oferta);
							char **userResult;
							int userRows, userColumns;
							rc = sqlite3_get_table(db, sql, &userResult, &userRows, &userColumns, 0);

							if (rc != SQLITE_OK)
							{
								fprintf(stderr, "Eroare la execuția interogării SQL pentru user_id: %s\n", sqlite3_errmsg(db));
								sqlite3_close(db);
								return rc;
							}
							strcpy(user_id, userResult[userColumns]);
							sqlite3_free_table(userResult);

							char us[10]; sprintf(us, "%d", id_user);
							if (strcmp(user_id, us) != 0)
							{
								// Returnăm mesajul clientului
								strcpy(msgrasp, "Confirmati comanda? (Y/N)");
								if (write(client, msgrasp, 100) <= 0)
								{
									perror("[server]Eroare la write() catre client.\n");
									return 0;
								}

								// Așteaptă mesajul de confirmare de la client
								if (read(client, msg, 100) <= 0)
								{
									perror("[server]Eroare la read() de la client.\n");
									return 0;
								}
								msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
								printf ("[server]Mesajul a fost receptionat...%s\n", msg); 

								if(strstr(msg, "Y") != NULL || strstr(msg, "y") != NULL)
								{
									// Returnăm mesajul clientului
									strcpy(msgrasp, "Comanda a fost realizata!");
									if (write(client, msgrasp, 100) <= 0)
									{
										perror("[server]Eroare la write() catre client.\n");
										return 0;
									}
									
									// Adaugăm oferta în tabelul "cos"
									char insert_sql[500];
									sprintf(insert_sql, "INSERT INTO cos (id_user, id_oferta, titlu, descriere, pret) VALUES (%d, '%s', '%s', '%s', '%s');",
											id_user, id_oferta, titlu, descriere, pret);

									rc = sqlite3_exec(db, insert_sql, 0, 0, 0);

									if (rc != SQLITE_OK)
									{
										fprintf(stderr, "Eroare la adăugarea ofertei în coș: %s\n", sqlite3_errmsg(db));
										sqlite3_close(db);
										return rc;
									}
									printf("Oferta a fost adaugata in cos cu succes.\n");

									// Ștergem oferta din tabela "oferte" după adăugare în coș, pentru ca a fost comandata si nu mai este disponibila pt alti clienti
									char delete_sql[200];
									sprintf(delete_sql, "DELETE FROM oferte WHERE id_oferta='%s';", id_oferta);

									rc = sqlite3_exec(db, delete_sql, 0, 0, 0);

									if (rc != SQLITE_OK)
									{
										fprintf(stderr, "Eroare la ștergerea ofertei din tabela oferte: %s\n", sqlite3_errmsg(db));
										sqlite3_close(db);
										return rc;
									}
									printf("Oferta a fost ștearsa din tabela oferte cu succes.\n");
								}
								else
								{
									// Returnăm mesajul clientului
									strcpy(msgrasp, "Comanda nu a fost realizata!");
									if (write(client, msgrasp, 100) <= 0)
									{
										perror("[server]Eroare la write() catre client.\n");
										return 0;
									}
								}	
							}
							else
							{
								// ID-ul utilizatorului coincide cu cel care a creat oferta
								strcpy(msgrasp, "Nu puteți cumpăra propria ofertă.");
								if (write(client, msgrasp, 100) <= 0)
								{
									perror("[server]Eroare la write() către client.\n");
									return 0;
								}
							}	

							return 1;
						}
						else //nu exista oferta cu id-ul introdus
						{
							strcpy(msgrasp, "Nu exista o oferta cu id-ul ales.");
							if (write(client, msgrasp, 100) <= 0)
							{
								perror("[server]Eroare la write() catre client.\n");
								return 0;
							}
							return 1;
						}

						// Eliberăm memoria rezultatelor
						sqlite3_free_table(result);


						return 1;
					}
					else
					{
						// Nu există oferte
						strcpy(msgrasp, "Nu exista oferte disponibile.");
						if (write(client, msgrasp, 100) <= 0)
						{
							perror("[server]Eroare la write() catre client.\n");
							return 0;
						}
					}
				}
				case 2 : //comanda de intoarcere inapoi la meniul intern
				{
					return 1;
				}
			}
				// Eliberați memoria rezultatelor
				sqlite3_free_table(result);

			return 1;
		}
		case 2 : 
		{
			char titlu[100];
			char descriere[100];
			char pret[100];

			/* returnam mesajul clientului */
			strcpy(msgrasp, "Introduceti titlul ofertei:");
			if (write (client, msgrasp, 100) <= 0)
			{
				perror ("[server]Eroare la write() catre client.\n");
			}

			//citesc titlul ales
			if (read (client, titlu, 100) <= 0)
			{
				perror ("[server]Eroare la read() de la client.\n");
				close (client);	// inchidem conexiunea cu clientul 
			}
			msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
			printf ("[server]Mesajul a fost receptionat...%s\n", titlu); 

			/* returnam mesajul clientului */
			strcpy(msgrasp, "Introduceti descrierea ofertei:");
			if (write (client, msgrasp, 100) <= 0)
			{
				perror ("[server]Eroare la write() catre client.\n");
			}

			//citesc descrierea introdusa
			if (read (client, descriere, 100) <= 0)
			{
				perror ("[server]Eroare la read() de la client.\n");
				close (client);	// inchidem conexiunea cu clientul 
			}
			msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
			printf ("[server]Mesajul a fost receptionat...%s\n", descriere); 

			/* returnam pretul ofertei clientului */
			strcpy(msgrasp, "Introduceti pretul ofertei:");
			if (write (client, msgrasp, 100) <= 0)
			{
				perror ("[server]Eroare la write() catre client.\n");
			}

			//citesc pretul introdus
			if (read (client, pret, 100) <= 0)
			{
				perror ("[server]Eroare la read() de la client.\n");
				close (client);	// inchidem conexiunea cu clientul 
			}
			msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
			printf ("[server]Mesajul a fost receptionat...%s\n", pret); 

			/*Introduc datele in tabela oferte*/
			int rc;
			char sql[400];

			sprintf(sql, "INSERT INTO oferte (id_user, titlu, descriere, pret) VALUES ('%d', '%s', '%s', '%s');", id_user, titlu, descriere, pret);
			rc = sqlite3_exec(db, sql, 0, 0, 0);
			if (rc != SQLITE_OK) {
				fprintf(stderr, "Eroare la inserare: %s\n", sqlite3_errmsg(db));
			}

			//trimit mesajul de creare oferta cu succes
			strcpy(msgrasp, "Ati creat cu succes noua oferta!");
			if (write (client, msgrasp, 100) <= 0)
			{
				perror ("[server]Eroare la write() catre client.\n");
			}

			return 1;
		}
		case 3 : 
		{
			int rc;
			char sql[100];

			sprintf(sql, "SELECT id_oferta, titlu, descriere, pret FROM oferte WHERE id_user='%d';", id_user); 
			char **result;
			int rows, columns;

			rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
			if (rc != SQLITE_OK) {
				fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return rc;
			}

			if(rows > 0) //daca id_user este gasit in tabela cu oferte
			{	
				strcpy(msgrasp, "");
				int index;
				for (int i = 1; i <= rows; i++) {
					for (int j = 0; j < columns; j++) {
						index = i * columns + j;
						strcat(msgrasp, result[index]);
						if (j < columns - 1) {
							strcat(msgrasp, " "); // Adaugă un spațiu între id_oferta și titlu_oferta, doar dacă nu suntem la ultima coloană
						}
					}
					strcat(msgrasp, "\n"); // Adaugă un caracter newline între fiecare pereche id_oferta și titlu_oferta
				}

				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					return 0;	
				}

				//citesc mesajul introdus
				if (read (client, msg, 100) <= 0)
				{
					perror ("[server]Eroare la read() de la client.\n");
					close (client);	// inchidem conexiunea cu clientul 
				}
				msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
				printf ("[server]Mesajul a fost receptionat...%s\n", msg); 

				char modifier[150]; strcpy(modifier, msg);
				char *token = strtok(msg, " ");
				char id[10];
				if (token != NULL)
				{
					strncpy(id, token, sizeof(id) - 1);
					id[sizeof(id) - 1] = '\0'; 
				}

				// Procesăm răspunsul și realizăm modificarea corespunzătoare
				if (strstr(modifier, "titlu") != NULL)
				{
					/* returnam mesajul clientului */
					strcpy(msgrasp, "Introduceti noul titlu:");
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
						return 0;	
					}

					// Așteptăm noul titlu de la client
					strcpy(msg, "");
					if (read (client, msg, 100) <= 0)
					{
						perror ("[server]Eroare la read() de la client.\n");
						close (client);	// inchidem conexiunea cu clientul 
					}
					msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
					printf ("[server]Mesajul a fost receptionat...%s\n", msg); 

					// Actualizăm coloana "titlu" în baza de date
					char update_sql[180];
					sprintf(update_sql, "UPDATE oferte SET titlu='%s' WHERE id_oferta='%s';", msg, id);

					rc = sqlite3_exec(db, update_sql, 0, 0, 0);
					if (rc != SQLITE_OK)
					{
						fprintf(stderr, "Eroare la actualizarea titlului: %s\n", sqlite3_errmsg(db));
						sqlite3_close(db);
						return rc;
					}
					else
					{
						printf("Titlul a fost actualizat cu succes.\n");
						strcpy(msgrasp, "Titlul a fost actualizat cu succes.");

						// returnăm mesajul clientului
						if (write(client, msgrasp, 100) <= 0)
						{
							perror("[server]Eroare la write() catre client.\n");
							return 0;
						}
					}
				}
				else if (strstr(modifier, "descriere") != NULL)
				{
					/* returnam mesajul clientului */
					strcpy(msgrasp, "Introduceti noua descriere:");
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
						return 0;	
					}

					// Așteptăm noua descriere de la client
					strcpy(msg, "");
					if (read (client, msg, 100) <= 0)
					{
						perror ("[server]Eroare la read() de la client.\n");
						close (client);	// inchidem conexiunea cu clientul 
					}
					msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
					printf ("[server]Mesajul a fost receptionat...%s\n", msg); 

					// Actualizăm coloana "descriere" în baza de date
					char update_sql[180];
					sprintf(update_sql, "UPDATE oferte SET descriere='%s' WHERE id_oferta='%s';", msg, id);

					rc = sqlite3_exec(db, update_sql, 0, 0, 0);
					if (rc != SQLITE_OK)
					{
						fprintf(stderr, "Eroare la actualizarea descrierii: %s\n", sqlite3_errmsg(db));
						sqlite3_close(db);
						return rc;
					}
					else
					{
						printf("Descrierea a fost actualizat cu succes.\n");
						strcpy(msgrasp, "Descrierea a fost actualizat cu succes.");

						// returnăm mesajul clientului
						if (write(client, msgrasp, 100) <= 0)
						{
							perror("[server]Eroare la write() catre client.\n");
							return 0;
						}
					}
				}
				else if (strstr(modifier, "pret") != NULL)
				{
					/* returnam mesajul clientului */
					strcpy(msgrasp, "Introduceti noul pret:");
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
						return 0;	
					}

					// Așteptăm noul preț de la client
					strcpy(msg, "");
					if (read (client, msg, 100) <= 0)
					{
						perror ("[server]Eroare la read() de la client.\n");
						close (client);	// inchidem conexiunea cu clientul 
					}
					msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
					printf ("[server]Mesajul a fost receptionat...%s\n", msg); 

					// Actualizăm coloana "pret" în baza de date
					printf("id_oferta este %s\n", id);
					char update_sql[180];
					sprintf(update_sql, "UPDATE oferte SET pret='%s' WHERE id_oferta='%s';", msg, id);

					rc = sqlite3_exec(db, update_sql, 0, 0, 0);
					if (rc != SQLITE_OK)
					{
						fprintf(stderr, "Eroare la actualizarea pretului: %s\n", sqlite3_errmsg(db));
						sqlite3_close(db);
						return rc;
					}
					else
					{
						printf("Pretul a fost actualizat cu succes.\n");
						strcpy(msgrasp, "Pretul a fost actualizat cu succes.");

						// returnăm mesajul clientului
						if (write(client, msgrasp, 100) <= 0)
						{
							perror("[server]Eroare la write() catre client.\n");
							return 0;
						}
					}
				}	
				else 
				{
					/* returnam mesajul clientului */
					strcpy(msgrasp, "Comanda gresita! Ati fost intors la meniul principal.");
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
						return 0;	
					}
					return 1;
				}				

				// Eliberați memoria rezultatelor
				sqlite3_free_table(result);

			}
			else //id_user nu e gasit in tabela cu oferte
			{
				bzero(msg, 100);
				strcpy(msgrasp, "Nu aveti nicio oferta creata");

				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					return 0;		/* continuam sa ascultam */
				}
				// Eliberați memoria rezultatelor
				sqlite3_free_table(result);

				return 1;
			}
			return 1;
		}				
		case 4 : 
		{
			int rc;
			char sql[150];

			sprintf(sql, "SELECT id_oferta, titlu, descriere, pret FROM oferte WHERE id_user='%d';", id_user); 
			char **result;
			int rows, columns;

			rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
			if (rc != SQLITE_OK) {
				fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return rc;
			}

			if(rows > 0) //daca id_user este gasit in tabela cu oferte
			{	
				strcpy(msgrasp, "");
				int index;
				for (int i = 1; i <= rows; i++) {
					for (int j = 0; j < columns; j++) {
						index = i * columns + j;
						strcat(msgrasp, result[index]);
						if (j < columns - 1) {
							strcat(msgrasp, " "); // Adaugă un spațiu între id_oferta și titlu_oferta, doar dacă nu suntem la ultima coloană
						}
					}
					strcat(msgrasp, "\n"); // Adaugă un caracter newline între fiecare pereche id_oferta și titlu_oferta
				}


				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					return 0;	
				}
			}
			else //id_user nu e gasit in tabela cu oferte
			{
				bzero(msg, 100);
				strcpy(msgrasp, "Nu aveti nici o oferta creata");

				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					return 0;		/* continuam sa ascultam */
				}
				// Eliberați memoria rezultatelor
				sqlite3_free_table(result);

				return 1;
			}

			//citesc mesajul-id-ul ofertei introdus
			strcpy(msg, "");
			if (read (client, msg, 100) <= 0)
			{
				perror ("[server]Eroare la read() de la client.\n");
				close (client);	// inchidem conexiunea cu clientul 
			}
			msg[strcspn(msg, "\n")] = '\0'; 
			printf ("[server]Mesajul a fost receptionat...%s\n", msg); 

			printf("id este %s\n", msg);

			// Verificarea dacă id-ul există în tabela de oferte
			sprintf(sql, "SELECT id_oferta FROM oferte WHERE id_oferta='%s';", msg);
			rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);

			if (rc != SQLITE_OK) {
				fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return rc;
			}

			if (rows == 0) { // id-ul nu există în tabela de oferte
				bzero(msg, 100);
				strcpy(msgrasp, "ID de oferta introdus nu există!");

				// returnăm mesajul clientului
				if (write(client, msgrasp, 100) <= 0) {
					perror("[server]Eroare la write() către client.\n");
					sqlite3_free_table(result);
					return 0;
				}

				// Eliberați memoria rezultatelor
				sqlite3_free_table(result);
				return 1;
			}

			// Ștergerea înregistrării cu id_oferta specificat
			sprintf(sql, "DELETE FROM oferte WHERE id_oferta='%s';", msg);
			rc = sqlite3_exec(db, sql, 0, 0, 0);

			if (rc != SQLITE_OK)
			{
				fprintf(stderr, "Eroare la ștergerea înregistrării: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return rc;
			}
			else
			{
				printf("Înregistrarea cu id_oferta %s a fost ștearsă cu succes.\n", msg);
				strcpy(msgrasp, "Oferta aleasa a fost ștearsă cu succes.");

				// returnăm mesajul clientului
				if (write(client, msgrasp, 100) <= 0)
				{
					perror("[server]Eroare la write() către client.\n");
					return 0;
				}
			}


			return 1;
		}
		case 5 : 
		{
			int rc;
			char sql[100];

			sprintf(sql, "SELECT id_oferta, titlu, descriere, pret FROM oferte WHERE id_user='%d';", id_user); 
			char **result;
			int rows, columns;

			rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
			if (rc != SQLITE_OK) {
				fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return rc;
			}

			if(rows > 0) //daca id_user este gasit in tabela cu oferte
			{	
				strcpy(msgrasp, "");
				int index;
				for (int i = 1; i <= rows; i++) {
					for (int j = 0; j < columns; j++) {
						index = i * columns + j;
						strcat(msgrasp, result[index]);
						if (j < columns - 1) {
							strcat(msgrasp, " "); // Adaugă un spațiu între id_oferta și titlu_oferta, doar dacă nu suntem la ultima coloană
						}
					}
					strcat(msgrasp, "\n"); // Adaugă un caracter newline între fiecare pereche id_oferta și titlu_oferta
				}


				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					return 0;	
				}

				return 1;
			}
			else //id_user nu e gasit in tabela cu oferte
			{
				bzero(msg, 100);
				strcpy(msgrasp, "Nu aveti nicio oferta creata.");

				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					return 0;		/* continuam sa ascultam */
				}
				// Eliberați memoria rezultatelor
				sqlite3_free_table(result);
			}
			return 1;
		}
		case 6 : 
		{
			int rc;
			char sql[100];

			sprintf(sql, "SELECT titlu, descriere, pret FROM cos WHERE id_user='%d';", id_user); 
			char **result;
			int rows, columns;

			rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
			if (rc != SQLITE_OK) {
				fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return rc;
			}

			if(rows > 0) //daca id_user este gasit in tabela cu oferte
			{	
				strcpy(msgrasp, "");
				int index;
				for (int i = 1; i <= rows; i++) {
					for (int j = 0; j < columns; j++) {
						index = i * columns + j;
						strcat(msgrasp, result[index]);
						if (j < columns - 1) {
							strcat(msgrasp, " "); // Adaugă un spațiu între id_oferta și titlu_oferta, doar dacă nu suntem la ultima coloană
						}
					}
					strcat(msgrasp, "\n"); // Adaugă un caracter newline între fiecare pereche id_oferta și titlu_oferta
				}


				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					return 0;	
				}

				return 1;
			}
			else //id_user nu e gasit in tabela cu oferte
			{
				bzero(msg, 100);
				strcpy(msgrasp, "Nu ati realizat niciodata o comanda!");

				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					return 0;		/* continuam sa ascultam */
				}
				// Eliberați memoria rezultatelor
				sqlite3_free_table(result);
			}
			return 1;
		}
		case 7:
		{
			/* returnam mesajul clientului */
			strcpy(msgrasp, "Introduceti produsul cautat:");
			if (write(client, msgrasp, 100) <= 0)
			{
				perror("[server]Eroare la write() catre client.\n");
			}

			// Așteaptă numele produsului cautat de la client
			if (read(client, msg, 100) <= 0)
			{
				perror("[server]Eroare la read() de la client.\n");
				return 0;
			}
			msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
			printf("[server]Mesajul a fost receptionat...%s\n", msg);

			// Construiește și execută interogarea SQL
			char sql[450];

			char keyword1[50], keyword2[50], keyword3[50];
			int keywordsCount = sscanf(msg, "%s %s %s", keyword1, keyword2, keyword3);

			// Construiește și execută interogarea SQL pentru expresia cu unul, două sau trei cuvinte
			if (keywordsCount > 0)
			{
				// Un singur cuvant sau primul cuvant din expresie
				sprintf(sql, "SELECT id_oferta, titlu, descriere, pret FROM oferte WHERE LOWER(titlu) LIKE LOWER('%%%s%%') OR LOWER(descriere) LIKE LOWER('%%%s%%')", keyword1, keyword1);
			}
			if (keywordsCount > 1)
			{
				// Al doilea cuvant din expresie
				sprintf(sql + strlen(sql), " UNION SELECT id_oferta, titlu, descriere, pret FROM oferte WHERE LOWER(titlu) LIKE LOWER('%%%s%%') OR LOWER(descriere) LIKE LOWER('%%%s%%')", keyword2, keyword2);
			}
			if (keywordsCount == 3)
			{
				// Al treilea cuvant din expresie
				sprintf(sql + strlen(sql), " UNION SELECT id_oferta, titlu, descriere, pret FROM oferte WHERE LOWER(titlu) LIKE LOWER('%%%s%%') OR LOWER(descriere) LIKE LOWER('%%%s%%')", keyword3, keyword3);
			}

			int rc;
			char **result;
			int rows, columns;

			rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);

			if (rc != SQLITE_OK)
			{
				fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return rc;
			}

			// Procesează rezultatele și trimite-le clientului
			if (rows > 0)
			{
				strcpy(msgrasp, "");
				int index;
				for (int i = 1; i <= rows; i++)
				{
					for (int j = 0; j < columns; j++)
					{
						index = i * columns + j;
						strcat(msgrasp, result[index]);
						if (j < columns - 1)
						{
							strcat(msgrasp, " ");
						}
					}
					strcat(msgrasp, "\n");
				}

				// returnăm rezultatele către client
				if (write(client, msgrasp, 100) <= 0)
				{
					perror("[server]Eroare la write() catre client.\n");
					return 0;
				}
			}
			else
			{
				strcpy(msgrasp, "Nu s-au gasit rezultate pentru produsul cautat.");
				if (write(client, msgrasp, 100) <= 0)
				{
					perror("[server]Eroare la write() catre client.\n");
					return 0;
				}
			}

			// Eliberează memoria rezultatelor
			sqlite3_free_table(result);

			return 1;
		}
		case 8 : 
		{
			char pass_chr[20];
			char parola[20];
			char email[40];
			char adresa[50];
			char telefon[15];
			char str_ras[300];

			//extrag parola folosind username-ul
			char sql[100];
			int rc;

			sprintf(sql, "SELECT password, email, adresa, telefon FROM useri WHERE username='%s';", user);
			char **result;
			int rows, columns;

			rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
			if (rc != SQLITE_OK) {
				fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return rc;
			}
			// Verificăm dacă avem cel puțin un rând în rezultate
			if (rows > 0) {
				strcpy(parola,result[(rows + 1) * columns - 4]);

				// Verificăm și tratăm cazurile în care email, adresa sau telefon sunt goale sau nule
				if (result[(rows + 1) * columns - 3] != NULL && strlen(result[(rows + 1) * columns - 3]) > 0) {
					strcpy(email, result[(rows + 1) * columns - 3]);
				} else {
					strcpy(email, " ");
				}

    			if (result[(rows + 1) * columns - 2] != NULL && strlen(result[(rows + 1) * columns - 2]) > 0) {
            		strcpy(adresa, result[(rows + 1) * columns - 2]);
				} else {
					strcpy(adresa, " ");
				}
    			
				if (result[(rows + 1) * columns - 1] != NULL && strlen(result[(rows + 1) * columns - 1]) > 0) {
					strcpy(telefon, result[(rows + 1) * columns - 1]);
				} else {
					strcpy(telefon, " ");
				}
			} else {
				printf("Utilizatorul nu a fost găsit.\n");
			}
			// Eliberăm memoria rezultatelor
			sqlite3_free_table(result);

			parola[strcspn(parola, "\n")] = '\0'; // Elimin newline-ul
			int i;
			for(i = 0; i < strlen(parola); i++)
				pass_chr[i] = '*';
			pass_chr[i] = '\0';

			printf("hello\n");
			strcpy(str_ras, "Username: ");
			strcat(str_ras, user);
			strcat(str_ras, "\nParola: ");
			strcat(str_ras, pass_chr);
			strcat(str_ras, "\nEmail: ");
			strcat(str_ras, email);
			strcat(str_ras, "\nAdresa: ");
			strcat(str_ras, adresa);
			strcat(str_ras, "\nContact: ");
			strcat(str_ras, telefon);
			printf("%s\n", str_ras);

			/* returnam inf. disponibile clientului */
			if (write (client, str_ras, 300) <= 0)
			{
				perror ("[server]Eroare la write() catre client.\n");
			}

			// Așteaptă nr.comenzii de la client
			if (read(client, msg, 100) <= 0)
			{
				perror("[server]Eroare la read() de la client.\n");
				return 0;
			}
			msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
			printf ("[server]Mesajul a fost receptionat...%s\n", msg); 

			int nr = atoi(msg); //nr comenzii
			switch(nr)
			{
				case 1 :
				{
					char email[30];

					/* Verifică dacă utilizatorul are deja un email înregistrat */
					sprintf(sql, "SELECT email FROM useri WHERE username='%s';", user);
					rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
					if (rc != SQLITE_OK)
					{
						fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
						sqlite3_close(db);
						return rc;
					}

					if (rows > 0 && result[1] != NULL)
					{
						// Utilizatorul are deja un email înregistrat
						strcpy(email, result[1]);  // Adresa de email este în prima celulă a rezultatelor
						sqlite3_free_table(result);

						strcpy(msgrasp, "Aveti deja un email inregistrat! ");
						if (write(client, msgrasp, 100) <= 0)
						{
							perror("[server]Eroare la write() catre client.\n");
						}

						return 1;
					}

					strcpy(msgrasp, "Introduceti adresa de e-mail");
					/* returnam mesajul clientului */
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
					}

					// Așteaptă email-ul de la client
					if (read(client, msg, 100) <= 0)
					{
						perror("[server]Eroare la read() de la client.\n");
						return 0;
					}
					msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
					strcpy(email, msg);
					printf ("[server]Mesajul a fost receptionat...%s\n", email); 

					/*Introduc emailul in tabela useri*/
					int rc;
					char sql[400];

					sprintf(sql, "UPDATE useri SET email = '%s' WHERE username = '%s';", email, user);

					rc = sqlite3_exec(db, sql, 0, 0, 0);
					if (rc != SQLITE_OK) {
						fprintf(stderr, "Eroare la inserare: %s\n", sqlite3_errmsg(db));
					}

					// Eliberați memoria rezultatelor
					sqlite3_free_table(result);

					//trimit mesajul de adaugare a email-ului
					strcpy(msgrasp, "Email-ul introdus a fost salvat!");
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
					}

					return 1;
				}
				case 2 :
				{
					char adresa[50];

					/* Verificăm dacă utilizatorul are deja o adresa înregistrata */
					sprintf(sql, "SELECT adresa FROM useri WHERE username='%s';", user);
					rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
					if (rc != SQLITE_OK)
					{
						fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
						sqlite3_close(db);
						return rc;
					}
					
					if (rows > 0 && result[1] != NULL)
					{
						// Utilizatorul are deja un email înregistrat
						printf("aiciii\n");
						strcpy(adresa, result[1]);  
						sqlite3_free_table(result);

						strcpy(msgrasp, "Aveti deja o adresa inregistrata! ");
						if (write(client, msgrasp, 100) <= 0)
						{
							perror("[server]Eroare la write() catre client.\n");
						}

						return 1;
					}

					strcpy(msgrasp, "Introduceti adresa de livrare:");
					/* returnam mesajul clientului */
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
					}

					// Așteaptă adresa de la client
					if (read(client, msg, 100) <= 0)
					{
						perror("[server]Eroare la read() de la client.\n");
						return 0;
					}
					msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
					strcpy(adresa, msg);
					printf ("[server]Mesajul a fost receptionat...%s\n", adresa); 

					/*Introduc adresa in tabela useri*/
					int rc;
					char sql[400];

					sprintf(sql, "UPDATE useri SET adresa = '%s' WHERE username = '%s';", adresa, user);

					rc = sqlite3_exec(db, sql, 0, 0, 0);
					if (rc != SQLITE_OK) {
						fprintf(stderr, "Eroare la inserare: %s\n", sqlite3_errmsg(db));
					}

					// Eliberați memoria rezultatelor
					sqlite3_free_table(result);	

					//trimit mesajul de adaugare a ofertei
					strcpy(msgrasp, "Adresa introdusa a fost salvata!");
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
					}

					return 1;
				}
				case 3 :
				{
					char telefon[15];

					/* Verifică dacă utilizatorul are deja un telefon înregistrat */
					sprintf(sql, "SELECT telefon FROM useri WHERE username='%s';", user);
					rc = sqlite3_get_table(db, sql, &result, &rows, &columns, 0);
					if (rc != SQLITE_OK)
					{
						fprintf(stderr, "Eroare la execuția interogării SQL: %s\n", sqlite3_errmsg(db));
						sqlite3_close(db);
						return rc;
					}
					
					if (rows > 0 && result[1] != NULL)
					{
						// Utilizatorul are deja un telefon înregistrat
						strcpy(telefon, result[1]);  
						sqlite3_free_table(result);

						strcpy(msgrasp, "Aveti deja un nr. de telefon inregistrat! ");
						if (write(client, msgrasp, 100) <= 0)
						{
							perror("[server]Eroare la write() catre client.\n");
						}

						return 1;
					}

					strcpy(msgrasp, "Introduceti nr de telefon:");
					/* returnam mesajul clientului */
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
					}

					// Așteaptă email-ul de la client
					if (read(client, msg, 100) <= 0)
					{
						perror("[server]Eroare la read() de la client.\n");
						return 0;
					}
					msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
					strcpy(telefon, msg);
					printf ("[server]Mesajul a fost receptionat...%s\n", telefon); 

					/*Introduc telefonul in tabela useri*/
					int rc;
					char sql[400];

					sprintf(sql, "UPDATE useri SET telefon = '%s' WHERE username = '%s';", telefon, user);

					rc = sqlite3_exec(db, sql, 0, 0, 0);
					if (rc != SQLITE_OK) {
						fprintf(stderr, "Eroare la inserare: %s\n", sqlite3_errmsg(db));
					}

					// Eliberați memoria rezultatelor
					sqlite3_free_table(result);

					//trimit mesajul de adaugare a telefonului
					strcpy(msgrasp, "Nr. de telefon introdus a fost salvat!");
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
					}
					
					return 1;
				}
				case 4 :
				{
					return 1;
				}
			}


			return 1;
		}
		case 9 : 
		{
			/* returnam mesajul clientului */
			strcpy(msgrasp, "V-ati delogat cu succes!");
			if (write (client, msgrasp, 100) <= 0)
			{
				perror ("[server]Eroare la write() catre client.\n");
			}
			printf("[server]Mesajul a fost transmis...%s\n", msgrasp);

			return 0;
		}
		case 10 : 
		{
			/* returnam mesajul clientului */
			strcpy(msgrasp, "In cateva secunde aplicatia se va inchide si veti fi delogat din cont...");
			if (write (client, msgrasp, 100) <= 0)
			{
				perror ("[server]Eroare la write() catre client.\n");
			}
			exit(0);
			return 2;
		}
		default : return 0;
	}
}