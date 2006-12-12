#include <QApplication>
#include <stdio.h>
int main(int argc, char *argv[]) 
{
	QApplication app(argc, argv);
    if (argc > 1)
        fprintf(stderr, "%s", argv[1]);
    else
        fprintf(stderr, "Failed");
    fflush(stderr);
	return 0;
}