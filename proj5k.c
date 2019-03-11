#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

//maks. dlugosc tekstu przekazanego w komunikacie
#define MAX 256

//struiktura komunikatu
typedef struct k
{
   long typ;	
   //tj. odbiorca, gdy =1 - odbiorca jest serwer, w innych przypadkach jest to pid procesu
   long nadawca;
   char tekst[MAX];
} komunikat;

//zmienne globalne
int id_kolejki;
komunikat kom;

/* --------------------	WATKI ------------------------ */
//watek 1 - odpowiedzialny za wysylanie komunikatu
void *wysylanie_komunikatu()
{
	printf("Klient %d wysyla komunikat o tresci: \"%s\" do serwera\n",getpid(),kom.tekst);
	if((msgsnd(id_kolejki, (komunikat *)&kom,strlen(kom.tekst)+sizeof(long),0)) == -1)
	{
       		printf("Blad wysylania komunikatu /klient: %i (%s)\n",errno,strerror(errno));
        	exit(2);
	}
pthread_exit((void *) 0);
}

//watek 2 - odpowiedzialny za odbieranie komunikatu zwrotnego
void *odbieranie_komunikatu()
{
kom.typ = getpid();
memset(kom.tekst,NULL,MAX); //wyczyszczenie tablicy znakow
if((msgrcv(id_kolejki,(komunikat *)&kom,MAX,kom.typ,IPC_NOWAIT)) == -1)  // dlF IPX_NOWAIT - proces od razu konczy dzialanie funkcji
{

	if (errno == ENOMSG) //nie ma wiadomosci zwrotnej dla klienta
	{
		pthread_exit((void *) 0);
	}
	else
	{
	printf("Blad pobrania komunikatu /klient: %i (%s)\n",errno,strerror(errno));
	exit(2);  
	}

}

printf("Klient %d odebral komunikat od serwera o tresci: %s\n",getpid(),kom.tekst);
pthread_exit((void *) 0);
}


int main() 
{
//tworzenie klucza
key_t klucz;
if(! (klucz = ftok(".",'A')))
        {
        printf("Blad tworzenia klucza /klient");
        exit(-2);
        }

//uzyskanie dostepu /stworzenie kolejki komunkatow
if((id_kolejki = msgget(klucz, 0600 | IPC_CREAT))==-1)
        {
        printf("Blad uzyskania dostepu do kolejki komunikatow /klient: %i (%s)\n", errno, strerror(errno));
        exit(-3);
        }
        else
        printf("Klient %d uzyskal dostep do kolejki komunikatow\n", getpid());

int i=0;
char wiadomosc[MAX];

//tworzenie watkow
pthread_t tid1, tid2;
if(pthread_create(&tid1, NULL,wysylanie_komunikatu, NULL))
{
        printf("Blad tworzenia watku (wysylanie_komunikatu): %i (%s)\n", errno, strerror(errno));
        exit(1);
}

if(pthread_create(&tid2, NULL,odbieranie_komunikatu, NULL))
{
        printf("Blad tworzenia watku (odbieranie_komunikatu): %i (%s)\n", errno, strerror(errno));
        exit(1);
}

//nieskonczona petla, program konczymy CTRL C
while(1)
{
	kom.typ = 1;
	kom.nadawca = getpid();

	//wprowadzanie tekstu
	printf("\nKlient %d: Podaj tekst do wyslania: ",getpid());
	i = 0;
		while(1)
		{
			wiadomosc[i] = getchar();
			if((wiadomosc[i] == '\n')||(i>=MAX-20))
			{
				wiadomosc[i] = '\0';
				break;
			}
			i++;
		}
	strcpy(kom.tekst,wiadomosc);

    //przylaczanie watkow
	if(pthread_join(tid1, NULL)) //wyslanie komunikatu
        {       
		printf("Blad przylaczenia watku (wysylanie_komunikatu):  %i (%s)\n", errno, strerror(errno));
        exit(2);
        }

    if(pthread_join(tid2, NULL)) //odebranie komunikatu
    {
        printf("Blad przylaczenia watku (odbieranie_komunikatu):  %i (%s)\n", errno, strerror(errno));
        exit(2);
    }
}

return 0;
}

