/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <string.h>
#include <qstring.h>
#include <qdebug.h>

#include "qlocalsocket.h"

#define SOCK_PATH "echo_socket"

int main(void)
{
    QLocalSocket socket;
    socket.connectToName(SOCK_PATH);
    socket.open(QIODevice::ReadWrite);

    printf("Connected.\n");
    char str[100];
    while(printf("> "), fgets(str, 100, stdin), !feof(stdin)) {
        if (socket.write(str, strlen(str)) == -1) {
            perror("send");
            return EXIT_FAILURE;
        }

        int t;
        if ((t = socket.read(str, 100)) > 0) {
            str[t] = '\0';
            printf("echo> %s", str);
        } else {
            if (t < 0)
                perror("recv");
            else
                printf("Server closed connection.\n");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
