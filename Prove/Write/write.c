#include <stdio.h>
#include <string.h>


int main()
{
	FILE *input = fopen( "stuff.txt", "r+" );
	fseek(input, 1, SEEK_END);
	char* buffer = "92";
	fwrite(buffer, 1, strlen(buffer), input);

	fclose(input);
	   FILE *fp;
   int c;

   fp = fopen("stuff.txt","r");
   while(1)
   {
      c = fgetc(fp);
      if( feof(fp) )
      {
         break ;
      }
      printf("%d ", c);
   }
   fclose(fp);
   return(0);
	
	return 0;
}
