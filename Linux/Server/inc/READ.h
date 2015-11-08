#ifndef READCOMMAND_H
#define READCOMMAND_H
#define MYSEEK_SET 0
#define MYSEEK_CUR 1
#define MYSEEK_END 2

void handleREADCommand(char* command, int socket);

#endif
