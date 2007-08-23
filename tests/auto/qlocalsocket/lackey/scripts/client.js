#/bin/qscript
function QVERIFY(x, socket) {
    if (!(x)) {
        throw(socket.errorString());
    }
}

var socket = new QScriptLocalSocket;
socket.peerName = "qlocalsocket_autotest";
QVERIFY(socket.waitForConnected(), socket);
print("client: connected");
socket.waitForReadyRead();
var text = socket.readLine();
var testLine = "test";
QVERIFY((text == testLine), socket);
QVERIFY((socket.errorString() == "Unknown error"), socket);
socket.close();
print("client: exiting", text);
