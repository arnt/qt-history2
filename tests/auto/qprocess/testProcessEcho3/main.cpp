/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <stdio.h>

int main()
{
    int c;
    for (;;) {
        c = fgetc(stdin);
        if (c == '\0')
            break;
        if (c != -1) {
            fputc(c, stdout);
            fputc(c, stderr);
            fflush(stdout);
            fflush(stderr);
        }
    }

    return 0;
}
