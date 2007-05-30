function QVERIFY(x, debugInfo) {
    if (!(x)) {
        print(debugInfo);
        throw(debugInfo);
    }
}

var producer = new ScriptSharedMemory;
producer.setKey("market");

var size = 1024;
if (!producer.create(size)) {
    QVERIFY(producer.error() == 4, "create");
    QVERIFY(producer.attach());
}
//print ("producer created and attached");
QVERIFY(producer.lock(), "lock");
producer.set(1, 1);
QVERIFY(producer.unlock(), "unlock");

var i = 0;
while(i < 5) {
    QVERIFY(producer.lock(), "lock");
    if (producer.get(0) == 'Q') {
        QVERIFY(producer.unlock(), "unlock");
        continue;
    }
    //print("producer: " + i);
    ++i;
    for (var j = 0; j < size; ++j)
        producer.set(j, 'Q');
    QVERIFY(producer.unlock(), "unlock");
}
QVERIFY(producer.lock());
producer.set(0, 'E');
QVERIFY(producer.unlock());

while (true) {
    QVERIFY(producer.lock());
    if (producer.get(1 == 1)) {
        QVERIFY(producer.unlock());
        break;
    }
    QVERIFY(producer.unlock());
}
//print ("producer done");
