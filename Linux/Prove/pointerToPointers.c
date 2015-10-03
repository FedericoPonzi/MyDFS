

#include <stdio.h>
void cant_change(int*, int*);
int main()
{

    int c = 1;
    int d = 2;
    int e = 3;
    int * a = &c;
    int * b = &d;
    int * f = &e;
    int ** pp = &a;  // pointer to pointer 'a'

    printf("\n a's value: %p \n", a);
    printf("\n b's value: %p \n", b);
    printf("\n f's value: %p \n", f);
    printf("\n can we change a?, lets see \n");
    printf("\n a = b \n");
    a = b;
    printf("\n a's value is now: %p, same as 'b'... it seems we can, but can we do it in a function? lets see... \n", a);
    printf("\n cant_change(a, f); \n");
    cant_change(a, f);
    printf("\n a's value is now: %p, Doh! same as 'b'...  that function tricked us. \n", a);

    printf("\n NOW! lets see if a pointer to a pointer solution can help us... remember that 'pp' point to 'a' \n");
    printf("\n change(pp, f); \n");
    change(pp, f);
    printf("\n a's value is now: %p, YEAH! same as 'f'...  that function ROCKS!!!. \n", a);
    return 0;
}

void cant_change(int * x, int * z){
    x = z;
    printf("\n ----> value of 'a' is: %p inside function, same as 'f', BUT will it be the same outside of this function? lets see\n", x);
}

void change(int ** x, int * z){
    *x = z;
    printf("\n ----> value of 'a' is: %p inside function, same as 'f', BUT will it be the same outside of this function? lets see\n", *x);
}
