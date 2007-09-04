#/bin/qscript
function QVERIFY(x, socket) {
    if (!(x)) {
        throw(socket.errorString());
    }
}

var socket = new QScriptLocalSocket;
var tries = 0;
do {
    socket.peerName = "qlocalsocket_autotest";
    if ((socket.errorString() != "QLocalSocket::connectToName: Invalid name")
        || (socket.errorString() != "QLocalSocket::connectToName: Connection Refused"))
        break;
    socket.sleep(1);
    ++tries;
} while ((socket.errorString() == "QLocalSocket::connectToName: Invalid name"
        || (socket.errorString() == "QlocalSocket::connectToName: ConnectionRefused"))
        && tries < 5000);
socket.waitForConnected(), socket;
//print("client: connected");
socket.waitForReadyRead();
var text = socket.readLine();
var testLine = "test";
QVERIFY((text == testLine), socket);
QVERIFY((socket.errorString() == "Unknown error"), socket);
socket.close();
//print("client: exiting", text);
