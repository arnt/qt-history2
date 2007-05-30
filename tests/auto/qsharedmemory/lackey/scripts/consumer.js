function QVERIFY(x, debugInfo) {
    if (!(x)) {
        print(debugInfo);
        throw(debugInfo + " " + x);
    }
}

var consumer = new ScriptSharedMemory;
consumer.setKey("market");

//print("consumer starting");
var tries = 0;;
while(!consumer.attach()) {
    if (tries == 5000) {
        print("consumer exiting, waiting too long");
        return;
    }
    ++tries;
}
//print("consumer attached");


QVERIFY(consumer.lock());
consumer.set(1, consumer.get(1) + 1);
QVERIFY(consumer.unlock());

var i = 0;
while(true) {
    QVERIFY(consumer.lock(), "lock");
    if (consumer.get(0) == 'Q') {
        consumer.set(0, ++i);
        //print ("consumer sets" + i);
    }
    if (consumer.get(0) == 'E') {
        QVERIFY(consumer.unlock(), "unlock");
        break;
    }
    QVERIFY(consumer.unlock(), "unlock");
}

QVERIFY(consumer.lock());
consumer.set(1, consumer.get(1) - 1);
QVERIFY(consumer.unlock());

//print("consumer detaching");
QVERIFY(consumer.detach());
