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
    char buffer[1024];
    for (;;) {
        size_t num = fread(buffer, 1, sizeof(buffer), stdin);
        if (num <= 0)
            break;
        fwrite(buffer, num, 1, stdout);
        fflush(stdout);
    }

    return 0;
}
