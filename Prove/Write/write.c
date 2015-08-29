#include <stdio.h>
#include <string.h>


int main()
{
	/*FILE *input = fopen( "stuff.txt", "r+" );
	fseek(input, 1, SEEK_END);
	char* buffer = "92";
	fwrite(buffer, 1, strlen(buffer), input);

	fclose(input);*/
	   FILE *fp;
   int c;
	char buff[1000];
   //fp = fopen("/home/isaacisback/Programmazione/programmazionedisistema/Client/.file.txt-tlAkpe","r");
   fp = fopen("stuff.txt","r");
   while(1)
   {
      c = fread(buff, 1 , 999, fp);
      buff[c] ='\0';
      printf("letti: %d, letto '%s'\n", c, buff);
      if( feof(fp) )
      
      {
         break ;
      }
   }
   printf("buff: %s", buff);
   fclose(fp);
   return(0);
	
}
