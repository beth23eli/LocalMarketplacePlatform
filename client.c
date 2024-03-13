#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;
/* portul de conectare la server*/
int port;
int sd;			// descriptorul de socket
char msg[100];		// mesajul trimis

int conectare();
int inregistrare();
void iesire();
int despre();

int main (int argc, char *argv[])
{
  struct sockaddr_in server;	// structura folosita pentru conectare 

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
  {
    perror ("[client]Eroare la connect().\n");
    return errno;
  }

  while(1)
  {
    label1:
    /*procesarea comenzii*/
    bzero (msg, 100);

    printf("\nLocalMarketplacePlatform\n\n");

    printf("\nAlegeti o comanda: \n");
    printf("1. Logare\n2. Inregistrare\n3. Iesire\n4. Despre aplicatie\n");
    scanf("%s", msg);

    /* trimiterea mesajului-nr de comanda la server */
    if (write (sd, msg, 100) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }
    fflush (stdout);
    
    int i = 0;
    if(strstr(msg, "1") != NULL) i = conectare();
    else if(strstr(msg, "2") != NULL) i = inregistrare();
    else if(strstr(msg, "4") != NULL) i = despre();
    else { iesire(); break; close(sd);}
    
    if(i) //daca e logat
    {
      label2:
      //meniul intern de comenzi
      printf("\nAlegeti o comanda: \n");
      printf("1. Vizualizeaza toate produsele\n");
      printf("2. Creeaza o oferta\n");
      printf("3. Modifica o oferta\n");
      printf("4. Sterge o oferta\n");
      printf("5. Vizualizeaza lista ofertelor create de vanzator\n");
      printf("6. Vizualizeaza istoricul comenzilor realizate\n");
      printf("7. Cauta un produs\n");
      printf("8. Vizualizeaza detalii cont\n"); 
      printf("9. Delogare\n"); 
      printf("10. IESIRE\n");

      scanf("%s", msg);
      /* trimiterea mesajului-nr de comanda la server */
      if (write (sd, msg, 100) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
      }

      int nr = atoi(msg);
      bzero(msg, 0);
      switch(nr)
      {
        case 1 :
        {
          /* citirea raspunsului-produsele dat de server */
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg);

          //afisez meniul
          printf("\nAlegeti o comanda:\n 1) Adauga in cosul de comenzi.\n 2) Meniul principal\n");
          scanf("%s", msg);
          /* trimiterea mesajului-nr de comanda la server */
          if (write (sd, msg, 100) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }

          int nr = atoi(msg);
          if(nr == 1)
          {
            /* citirea raspunsului dat de server */
            if (read (sd, msg, 100) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
            printf ("%s\n", msg);

            scanf("%s", msg);
            /* trimiterea mesajului-nr ofertei la server */
            if (write (sd, msg, 100) <= 0)
            {
              perror ("[client]Eroare la write() spre server.\n");
              return errno;
            }

            /* citirea raspunsului dat(daca confirma comanda) de server */
            if (read (sd, msg, 100) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
            printf ("%s\n", msg);

            if(strstr(msg, "propria") != NULL || strstr(msg, "Nu") != NULL) {goto label2;}

            //citesc y sau N daca confirma comanda
            scanf("%s", msg);
            /* trimiterea mesajului la server */
            if (write (sd, msg, 100) <= 0)
            {
              perror ("[client]Eroare la write() spre server.\n");
              return errno;
            }

            /* citirea raspunsului dat de server */
            if (read (sd, msg, 100) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
            printf ("%s\n", msg);

            goto label2;
          }
          else
          {
            goto label2;
          }

          goto label2;
        }
        case 2 :
        {
          char text1[100];
          char text2[100];
          char text3[100];

          /*CITESC TITLUL, DESCRIEREA, PRETUL OFERTEI*/
          /* citirea raspunsului-mesajul pt titlu dat de server */
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg);
          //citesc titlul ofertei
          read (0, text1, 100);
          /* trimiterea mesajului-titlului la server */
          if (write (sd, text1, 100) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }


          /* citirea raspunsului-mesajul pt descriere dat de server */
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg);
          //citesc descrierea ofertei
          read (0, text2, 100);
          /* trimiterea mesajului-descrierii la server */
          if (write (sd, text2, 100) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }

          /* citirea raspunsului-mesajul pt pret dat de server */
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg);
          //citesc pretul ofertei
          read (0, text3, 100);
          /* trimiterea mesajului-pretului la server */
          if (write (sd, text3, 100) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }
          
          //citesc mesajul-de creare a ofertei
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg);
          
          goto label2;
        }
        case 3 :
        {
          char modifier[10];
          //citesc mesajul - toate ofertele utilizatorului
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("Alegeti id_ofertei pe care doriti sa o modificati:\n%s\n", msg);
          strcpy(msg, ""); 
          read(0, msg, 100);
          msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul

          printf ("Alegeti ce anume doriti sa modificati - titlu/descriere/pret:\n");
          read(0, modifier, 10);
          modifier[strcspn(modifier, "\n")] = '\0'; // Elimin newline-ul

          strcat(msg, " ");
          strcat(msg, modifier);

          //transmit id_oferta + ce anume sa modific
          if (write (sd, msg, 100) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }

          //citesc mesajul utilizatorului
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg);

          if(strstr(msg, "gresita") != NULL) goto label2;

          //citesc componenta aleasa modificata
          strcpy(msg, "");
          read(0, msg, 100);
          msg[strcspn(msg, "\n")] = '\0';

          //transmit 
          if (write (sd, msg, 100) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }

          //citesc mesajul de actualizare utilizatorului
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg);

          goto label2;
        }
        case 4 :
        {
          char id[5];
          //citesc mesajul - toate ofertele utilizatorului
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg);

          if(strstr(msg, "nici") != NULL) goto label2;

          //citesc id ofertei pe care userul doreste sa o stearga
          printf("Alegeti id-ul ofertei pe care doriti sa o stergeti: \n");
          read(0, id, 5);

          /* trimiterea mesajului-id-ul ofertei la server */
          if (write (sd, id, 5) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }

          //citesc mesajul de stergere a ofertei utilizatorului
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg);

          goto label2;
        }
        case 5 :
        {
          //citesc mesajul - toate ofertele utilizatorului
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg);

          goto label2;
        }
        case 6 :
        {
          //citesc mesajul - istoricul comenzilor realizate utilizatorului
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          if(strstr(msg, "Nu") != NULL) {printf("%s\n", msg); goto label2;}
          printf ("Acestea sunt comenzile realizate:\n");
          printf("%s\n", msg);

          goto label2;
        }
        case 7 :
        {
          /* citirea raspunsului dat de server */
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg);

          //citesc numele produsului cautat
          strcpy(msg, "");
          read(0, msg, 100);
          msg[strcspn(msg, "\n")] = '\0'; // elimină caracterul newline

          /* trimiterea mesajului-produsul cautat la server */
          if (write (sd, msg, 100) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }

          /* citirea raspunsului dat de server */
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("\n%s\n", msg);

          goto label2;
        }
        case 8 :
        {
          char inf[300];
          printf("\nDetalii cont:\n");

          //citesc mesajul - inf despre cont
          if (read (sd, inf, 300) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", inf);

          //meniu de comenzi
          printf("\n1) Adauga o adresa de email\n2) Adauga adresa de livrare\n3) Adauga un nr. de telefon\n4) Inapoi\n");
          scanf("%s", msg);
          /* trimiterea mesajului-nr de comanda la server */
          if (write (sd, msg, 100) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }

          int nr = atoi(msg);
          if(nr == 1)
          {
            //citesc mesajul pt email utilizatorului
            if (read (sd, msg, 100) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
            printf ("%s\n", msg);
            sleep(2);
            if(strstr(msg, "deja") != NULL) goto label2;

            read (0, msg, 100);
            msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
            /* trimiterea email-ului la server */
            if (write (sd, msg, 100) <= 0)
            {
              perror ("[client]Eroare la write() spre server.\n");
              return errno;
            }

            //citesc mesajul de adaugare a email-ului
            if (read (sd, msg, 100) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
            printf ("%s\n", msg);
          }
          else if(nr == 2)
          {
            //citesc mesajul pt adresa utilizatorului
            if (read (sd, msg, 100) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
            printf ("%s\n", msg);
            sleep(2);
            if(strstr(msg, "deja") != NULL) goto label2;

            read (0, msg, 100);
            msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
            /* trimiterea adresei la server */
            if (write (sd, msg, 100) <= 0)
            {
              perror ("[client]Eroare la write() spre server.\n");
              return errno;
            }

            //citesc mesajul de adaugare a adresei
            if (read (sd, msg, 100) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
            printf ("%s\n", msg);
          }
          else if(nr == 3)
          {
            //citesc mesajul pt telefon utilizatorului
            if (read (sd, msg, 100) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
            printf ("%s\n", msg);
            sleep(2);
            if(strstr(msg, "deja") != NULL) goto label2;

            read (0, msg, 100);
            msg[strcspn(msg, "\n")] = '\0'; // Elimin newline-ul
            /* trimiterea telefonului la server */
            if (write (sd, msg, 100) <= 0)
            {                                                
              perror ("[client]Eroare la write() spre server.\n");
              return errno;
            }

            //citesc mesajul ca s-a adaugat telefonul
            if (read (sd, msg, 100) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
            printf ("%s\n", msg);
          }
          else
          {
            goto label2;
          }

          goto label2;
        }
        case 9 :
        {
          //citesc mesajul de delogare
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          printf ("%s\n", msg); 

          goto label1;

          break;
        }
        case 10 :
        {
          //citesc mesajul de iesire din aplicatie
          if (read (sd, msg, 100) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          sleep(1);
          printf ("%s\n", msg);

          sleep(3);
          close(sd);
        }
      }

      strcpy(msg, "1");
    } 
    //daca se intoarce cu statusul 0 -> apare din noul meniul de intrare
    if(atoi(msg) == 0)
    {
      goto label1;
    }
    else break;
  }

  /* inchidem conexiunea, am terminat */
  close (sd);

  return 0;
}

int conectare()
{
  char username[20];
  char parola[20];
  
  /* citirea raspunsului - mesajul de introducere username dat de server (apel blocant pina cind serverul raspunde) */
  if (read (sd, msg, 100) < 0)
  {
    perror ("[client]Eroare la read() de la server.\n");
    return errno;
  }
  /* afisam mesajul primit */
  printf ("%s\n", msg);

  read (0, username, 10);
  /* trimiterea username-ului la server */
  if (write (sd, username, 10) <= 0)
  {
    perror ("[client]Eroare la write() spre server.\n");
    return errno;
  }

  /* citirea raspunsului dat de server-introducerea parolei (apel blocant pina cind serverul raspunde) */
  if (read (sd, msg, 100) < 0)
  {
    perror ("[client]Eroare la read() de la server.\n");
    return errno;
  }
  printf("%s\n", msg);

  read (0, parola, 10);
  /* trimiterea parolei la server */
  if (write (sd, parola, 20) <= 0)
  {
    perror ("[client]Eroare la write() spre server.\n");
    return errno;
  }

  /* citirea raspunsului dat de server (apel blocant pina cind serverul raspunde) */
  if (read (sd, msg, 100) < 0)
  {
    perror ("[client]Eroare la read() de la server.\n");
    return errno;
  }
  printf("%s\n", msg);

  if(strstr(msg, "Nu") != NULL) return 0;
  else return 1;
}

int despre()
{
  char about[600];
  /* citirea raspunsului dat de server (apel blocant pina cind serverul raspunde) */
  if (read (sd, about, 600) < 0)
  {
    perror ("[client]Eroare la read() de la server.\n");
  }
  printf("%s\n", about);
  strcpy(msg, "0");
  sleep(2);

  return 0;
}

int inregistrare()
{
  char username[10];
  char parola[20];

    /* citirea raspunsului - mesajul de conectare/inregistrare dat de server (apel blocant pina cind serverul raspunde) */
    if (read (sd, msg, 100) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
    /* afisam mesajul primit */
    printf ("%s\n", msg);

  read (0, username, 10);

  /* trimiterea username-ului la server */
  if (write (sd, username, 10) <= 0)
  {
    perror ("[client]Eroare la write() spre server.\n");
    return errno;
  }

  /* citirea raspunsului dat de server-introducerea parolei (apel blocant pina cind serverul raspunde) */
  if (read (sd, msg, 100) < 0)
  {
    perror ("[client]Eroare la read() de la server.\n");
    return errno;
  }
  printf("%s\n", msg);

  read (0, parola, 20);
  /* trimiterea parolei la server */
  if (write (sd, parola, 20) <= 0)
  {
    perror ("[client]Eroare la write() spre server.\n");
    return errno;
  }

  /* citirea raspunsului dat de server (apel blocant pina cind serverul raspunde) */
  if (read (sd, msg, 100) < 0)
  {
    perror ("[client]Eroare la read() de la server.\n");
    return errno;
  }
  printf("%s\n", msg);

  if(strstr(msg, "Exista") != NULL) return 0;
  else return 1;
}

void iesire()
{
  /* citirea raspunsului dat de server (apel blocant pina cind serverul raspunde) */
  if (read (sd, msg, 100) < 0)
  {
    perror ("[client]Eroare la read() de la server.\n");
  }
  printf("Aplicatia se va inchide in cateva secunde...\n"); sleep(3);
  printf("%s\n", msg);
}