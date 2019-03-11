#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

//maks. dlugosc tekstu przekazanego w komunikacie
#define MAX 256

//funkcja obslugujaca sygnal SIGINT przekazany do programu
void obsluga_sigint(int sig_n);

//struiktura komunikatu
typedef struct k 
{
   long typ; 
   long nadawca;
   char tekst[MAX];
} komunikat;


int main()
{

//tworzenie klucza
key_t klucz;
if(! (klucz = ftok(".",'A')))
        {
        printf("Blad tworzenia klucza /serwer");
        exit(-2);
        }

//uzyskanie dostepu /stworzenie kolejki komunikatow
int id_kolejki;
if((id_kolejki = msgget(klucz, 0600 | IPC_CREAT))==-1)
	{
	printf("Blad uzyskania dostepu do kolejki komunikatow /serwer: %i (%s)\n", errno, strerror(errno));
	exit(-3);
	}	

//obsluga sygnalu SIGINT - przerwanie programu
signal(SIGINT,obsluga_sigint);
printf("^C konczy prace serwera \n\n");

//nieskonczona petla, CTRL+C konczymy dzialanie serwera
komunikat kom;

while(1)
{
	printf("\nSerwer oczekuje na komunikaty\n");
	kom.typ = 1;
	memset(kom.tekst,NULL,MAX); //wyczyszczenie tablicy znakow

	//pobieranie wiadomosci
	if((msgrcv(id_kolejki,(komunikat *)&kom,MAX+sizeof(long),kom.typ,0)) == -1)
	{
		printf("Blad pobrania komunikatu /serwer: %i (%s)\n",errno,strerror(errno));
		return -1;	
	}

	printf("Serwer odebral komunikat od procesu: %d, o tresci: %s\n", kom.nadawca, kom.tekst);

	//zmiana na wielkie litery tekstu
	int rozmiar = strlen(kom.tekst), i;
	for(i=0; i<rozmiar; i++)
	{
		kom.tekst[i] = toupper(kom.tekst[i]);
	}

	//wysylanie odpowiedzi do klienta
	kom.typ = kom.nadawca;
	printf("Serwer wysyla komunikat do: %ld, o tresci: %s\n", kom.nadawca, kom.tekst);
	if((msgsnd(id_kolejki,(komunikat *)&kom,strlen(kom.tekst)+sizeof(kom.nadawca),0))==-1)
	{
		printf("Blad wysylania komunikatu /serwer: %i (%s)\n",errno,strerror(errno));
		return -1;
	}

}

return 0;
}

//funckja obslugujaca sygnal SIGINT
void obsluga_sigint(int sig_n)
{
	key_t klucz;
	if(! (klucz = ftok(".",'A')))
        {
        	printf("Blad tworzenia klucza /serwer");
        	exit(-2);
        }
	
	int id_kolejki;
	if(sig_n==SIGINT){
		printf(" Zakonczono prace serwera\n");
		
		int id_kolejki;
		if((id_kolejki =msgget(klucz, 0600 | IPC_CREAT))==-1)
        	{
        		printf("Blad uzyskania dostepu do kolejki komunikatow /serwer: %i (%s)\n", errno, strerror(errno));
        		exit(-3);
        	}

		if((msgctl(id_kolejki, IPC_RMID,0)) == -1){
			printf("Blad usuwania kolejki komunikatow /serwer: %i (%s)\n", errno, strerror(errno));

			exit(-1);
		}
		else
		printf("Usunieta kolejke komunikatow o ID: %d\n", id_kolejki);
	
	exit(0);
	}
}
