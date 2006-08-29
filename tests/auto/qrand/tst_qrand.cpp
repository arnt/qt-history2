#include <QtTest/QtTest>

class tst_QRand: public QObject
{
    Q_OBJECT
private slots:
    void testqrand();
};

void tst_QRand::testqrand()
{
    const int numTestValues = 100;

    int generatedNumbers[numTestValues];
    bool generatesSameSequence = true;

    // test without calling srand() first
    // should give same sequence as with srand(1)

    for (int i=0; i<numTestValues; ++i)
        generatedNumbers[i] = qrand();

    qsrand(1);
    for (int i=0; i<numTestValues; ++i)
        if (generatedNumbers[i] != qrand())
            generatesSameSequence = false;
    
    QVERIFY(generatesSameSequence);

    for (unsigned int seed=1; seed < 10; seed+=100) {

        qsrand(seed);
        for (int i=0; i<numTestValues; ++i)
            generatedNumbers[i] = qrand();

        qsrand(seed);
        generatesSameSequence = true;
        for (int i=0; i<numTestValues; ++i)
            if (generatedNumbers[i] != qrand())
                generatesSameSequence = false;

        QVERIFY(generatesSameSequence);
    }
}

QTEST_MAIN(tst_QRand)
#include "tst_qrand.moc"
