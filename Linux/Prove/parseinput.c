#include <string.h>
#include <stdio.h>


/**
 * Parsa l' input, e modifica le impostazioni di conseguenza.
 * ./Server [-p<port>] [-d<path>] [-c<numberofconnection>]
 *
 */
void parseInput(int argc, char* argv[])
{
    int i;
    for(i = 1; i < argc; i++)
    {
        printf("Parso:%s\n", argv[i]);
        switch(* (argv[i]+1))
        {
            case 'd':
                printf("La directory è: %s\n", argv[i]+2);
                break;
            case 'p':
                
                printf("La porta è: %d\n", strtol(argv[i]+2, NULL, 10));
                break;
            case 'c':
                printf("Numero di connessioni:%d\n",strtol(argv[i]+2, NULL, 10));
        }
    }
}
int main(int argc, char* argv[])
{
    parseInput(argc, argv);
    return 0;

}
