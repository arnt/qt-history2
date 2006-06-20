/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

int main(int /* argc */, char *argv[])
{
    printf("%s", argv[0]);
    fflush(stdout);
    
    int c;
    while ((c = fgetc(stdin)) != -1) {
        printf("%c", char(c));
        fflush(stdout);
    }
    return 0;
}
