#ifndef OPENCOMMAND_H
#define OPENCOMMAND_H

#define MYO_RDONLY 1
#define MYO_WRONLY 2
#define MYO_RDWR   4
#define MYO_CREAT  8
#define MYO_TRUNC  16
#define MYO_EXCL   32
#define MYO_EXLOCK 64

void handleOpenCommand(char* command, int socket);

#endif
