/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: stdinprocess <all|line <0|1>>\n");
        printf("echos all its input to its output.\n");
        return 1;
    }
    
    QFile file;

    if (strcmp(argv[1], "all") == 0) {
        file.open(stdin, QFile::ReadWrite);
        printf("%s", file.readAll().constData());
    } else if (strcmp(argv[1], "line") == 0) {
        if (strcmp(argv[2], "0") == 0) {
            file.open(stdin, QFile::ReadWrite);
        } else {
            file.open(0, QFile::ReadWrite);
        }

        char line[1024];
        while (file.readLine(line, sizeof(line)) > 0) {
            printf("%s", line);
            fflush(stdout);
        }
    }
    return 0;
}
