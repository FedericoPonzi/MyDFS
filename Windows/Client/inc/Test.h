#ifndef TEST
#define TEST

int testOpen(char* filename, char* indirizzo, int const debug);
int testSimpleRead(char* filename, char* indirizzo, int const debug);
int testRead(char* filename, char* indirizzo, int const debug);
int testWrite(char* filename, char* indirizzo, int const debug);
void testInvalidazioneCache(char* filename, char* indirizzo, int const debug);
int testHeartBeating(char* filename, char* indirizzo, int const debug);
void testOpenErrors(char* filename, char* indirizzo, int debug);

#endif
