#include <qsocket.h>

main(int argc, char** argv)
{
    QSocket socket;
    socket.connectToHost("localhost",25);
    char helo[] = "HELO gooble\r\n";
    socket.writeBlock(helo,sizeof(helo));
    socket.flush(); // necessary, to permit QSocket to buffer more
    while ( !socket.bytesAvailable() ) {
        qDebug("No reply yet...");
        sleep(1);
    }
    qDebug("Got reply, %d bytes.", socket.bytesAvailable() );
}
