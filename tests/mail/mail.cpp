#include <qsocket.h>

main(int argc, char** argv)
{
    QSocket socket;
    socket.connectToHost("localhost",25);
    char helo[] = "HELO gooble\n";
    socket.writeBlock(helo,sizeof(helo));
    socket.flush(); // shouldn't be needed, and doesn't help
    while (!socket.bytesAvailable()) {
        qDebug("No reply yet...");
        sleep(1);
    }
    qDebug("Got reply.");
}
