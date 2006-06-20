/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <stdio.h>
#include <string.h>

int main()
{
    char buf[32];
    memset(buf, 0, sizeof(buf));

    char *cptr = buf;
    int c;
    while (cptr != buf + 31 && (c = fgetc(stdin)) != EOF)
        *cptr++ = (char) c;

    printf("%s", buf);
    return 0;
}
