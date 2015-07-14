#include <stdio.h>


typedef enum { MYO_RDONLY, MYO_WRONLY, MYO_RDWR, MYO_CREAT, MYO_TRUNC, MYO_EXCL, MYO_EXLOCK} modoApertura_t;

modoApertura_t prova(int i);

int main()
{
	if(prova(0) == MYO_RDONLY)
	{
		printf("Funzionaaa :D");
	}
	return 0;

}

modoApertura_t prova(int i)
{
	
	if(i == 0)
	{
		return MYO_RDONLY;
	}
}
