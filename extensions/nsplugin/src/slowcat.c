#include <stdio.h>

static x=0;

main()
{
    int ch;
    while ( (ch=getchar()) != EOF ) {
	putchar(ch);
	fflush(stdout);

	if (x++%10==0) usleep(1000);
    }
}
