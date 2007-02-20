#include <QtTest/QtTest>
#include <qxmlstream.h>
class tst_QXmlStream: public QObject
{
    Q_OBJECT
private slots:
    void testWellFormed();
    void testWellFormed_data();
    void testWellFormed_incrementally();
    void testWellFormed_incrementally_data();
    void testNotWellFormed();
    void testNotWellFormed_data();
    void testNotWellFormed_incrementally();
    void testNotWellFormed_incrementally_data();

    void testReader();
    void testReader_data();
};


void tst_QXmlStream::testWellFormed_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("canonical");
    QDir dir;
    dir.cd("xmltest/valid/sa/");
    foreach(QString filename , dir.entryList(QStringList() << "*.xml")) {
        QString canonicalname = "out/" + filename;
        QTest::newRow(dir.filePath(filename).toLatin1().data()) << dir.filePath(filename) << dir.filePath(canonicalname);
    }
}

void tst_QXmlStream::testWellFormed_incrementally_data()
{
    testWellFormed_data();
}

void tst_QXmlStream::testNotWellFormed_data()
{
    QTest::addColumn<QString>("xml");
    QDir dir;
    dir.cd("xmltest/not-wf/sa/");
    foreach(QString filename , dir.entryList(QStringList() << "*.xml")) {
        QTest::newRow(dir.filePath(filename).toLatin1().data()) << dir.filePath(filename);
    }
}

void tst_QXmlStream::testNotWellFormed_incrementally_data()
{
    testNotWellFormed_data();
}

static int best(int a, int b) {
    if (a < 0)
        return b;
    if (b < 0)
        return a;
    return qMin(a, b);
}

static int best(int a, int b, int c) {
    if (a < 0)
        return best(b, c);
    if (b < 0)
        return best(a, c);
    if (c < 0)
        return best(a, b);
    return qMin(qMin(a, b), c);
}

QByteArray makeCanonical(const QString &filename, bool testIncremental = false)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QXmlStreamReader reader;

    QByteArray buffer;
    int bufferPos = 0;

    if (!testIncremental) {
        reader.setDevice(&file);
    } else {
        buffer = file.readAll();
    }

    QByteArray outarray;
    QXmlStreamWriter writer(&outarray);

    forever {
        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.isDTD()) {
                if (!reader.notationDeclarations().isEmpty()) {
                    QString dtd;
                    dtd += "<!DOCTYPE doc [\n";
                    foreach (QXmlStreamNotationDeclaration notation, reader.notationDeclarations()) {
                        dtd += "<!NOTATION ";
                        dtd += notation.name().toString();
                        if (notation.publicId().isEmpty()) {
                            dtd += " SYSTEM \'";
                            dtd += notation.systemId().toString();
                            dtd += "\'";
                        } else {
                            dtd += " PUBLIC \'";
                            dtd += notation.publicId().toString();
                            dtd += "\'";
                            if (!notation.systemId().isEmpty() ) {
                                dtd += " \'";
                                dtd += notation.systemId().toString();
                                dtd += "\'";
                            }
                        }
                        dtd += ">\n";
                    }

                    dtd += "]>\n";
                    writer.writeDTD(dtd);
                }
            }else if (reader.isStartElement()) {
                writer.writeStartElement(reader.namespaceUri().toString(), reader.name().toString());

                QMap<QString, QXmlStreamAttribute> sortedAttributes;
                foreach(QXmlStreamAttribute attribute, reader.attributes())
                    sortedAttributes.insert(attribute.name().toString(), attribute);
                foreach(QXmlStreamAttribute attribute, sortedAttributes.values())
                    writer.writeAttribute(attribute);
            } else if (reader.isCharacters()) {
                // make canonical

                QString text = reader.text().toString();
                int i = 0;
                int p = 0;
                while ((i = best(text.indexOf(QLatin1Char(10), p),
                                 text.indexOf(QLatin1Char(13), p),
                                 text.indexOf(QLatin1Char(9), p))) >= 0) {
                    writer.writeCharacters(text.mid(p, i - p));
                    writer.writeEntityReference(QString("#%1").arg(text.at(i).unicode()));
                    p = i + 1;
                }
                writer.writeCharacters(text.mid(p));
            } else if (reader.isStartDocument() || reader.isEndDocument() || reader.isComment()){
                // canonical does not want any of those
            } else if (reader.isProcessingInstruction() && reader.processingInstructionData().isEmpty()) {
                // for some reason canonical wants a space
                writer.writeProcessingInstruction(reader.processingInstructionTarget().toString(), QLatin1String(""));
            } else if (!reader.error()){
                writer.writeCurrentToken(reader);
            }
        }
        if (testIncremental && bufferPos < buffer.size()) {
            reader.addData(QByteArray(buffer.data() + (bufferPos++), 1));
        } else {
            break;
        }
    }

    if (reader.error()) {
        outarray += "ERROR:";
        outarray += reader.errorString().toLatin1();
    }

    return outarray;
}

bool verifyWellFormed(const QString &filename, bool testIncremental = false)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QXmlStreamReader reader;

    QByteArray buffer;
    int bufferPos = 0;

    if (!testIncremental) {
        reader.setDevice(&file);
    } else {
        buffer = file.readAll();
    }

    QByteArray outarray;
    QXmlStreamWriter writer(&outarray);

    forever {
        while (!reader.atEnd()) {
            reader.readNext();
        }
        if (testIncremental && bufferPos < buffer.size()) {
            reader.addData(QByteArray(buffer.data() + (bufferPos++), 1));
        } else {
            break;
        }
    }

    return reader.error() == QXmlStreamReader::NoError;
}



void tst_QXmlStream::testWellFormed()
{
    QFETCH(QString, xml);
    QFETCH(QString, canonical);
    QByteArray result = makeCanonical(xml);
    QFile file(canonical);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString reference = QString::fromUtf8(file.readAll());
    QString qxmlstream = QString::fromUtf8(result);
    QEXPECT_FAIL("xmltest/valid/sa/012.xml", "The namespace specification requires this test to fail", Continue);
    QEXPECT_FAIL("xmltest/valid/sa/110.xml", "We are in sync with xmllint, the spec seems fuzzy about this one", Continue);
    QCOMPARE(qxmlstream, reference);
}

void tst_QXmlStream::testWellFormed_incrementally()
{
    QFETCH(QString, xml);
    QFETCH(QString, canonical);
    QByteArray result = makeCanonical(xml, true);
    QFile file(canonical);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString reference = QString::fromUtf8(file.readAll());
    QString qxmlstream = QString::fromUtf8(result);
    QEXPECT_FAIL("xmltest/valid/sa/012.xml", "The namespace specification requires this test to fail", Continue);
    QEXPECT_FAIL("xmltest/valid/sa/052.xml", "Bug in our utf8 codec?", Continue);
    QEXPECT_FAIL("xmltest/valid/sa/110.xml", "We are in sync with xmllint, the spec seems fuzzy about this one", Continue);
    QCOMPARE(qxmlstream, reference);
}

void tst_QXmlStream::testNotWellFormed()
{
    QFETCH(QString, xml);
    bool wellFormed = verifyWellFormed(xml);
    QEXPECT_FAIL("xmltest/not-wf/sa/168.xml", "Bug in our utf8 codec?", Continue);
    QEXPECT_FAIL("xmltest/not-wf/sa/169.xml", "Bug in our utf8 codec?", Continue);
    QEXPECT_FAIL("xmltest/not-wf/sa/170.xml", "Bug in our utf8 codec?", Continue);

    QCOMPARE(wellFormed, false);
}

void tst_QXmlStream::testNotWellFormed_incrementally()
{
    QFETCH(QString, xml);
    bool wellFormed = verifyWellFormed(xml, true);
    QEXPECT_FAIL("xmltest/not-wf/sa/168.xml", "Bug in our utf8 codec?", Continue);
    QEXPECT_FAIL("xmltest/not-wf/sa/169.xml", "Bug in our utf8 codec?", Continue);
    QEXPECT_FAIL("xmltest/not-wf/sa/170.xml", "Bug in our utf8 codec?", Continue);
    QCOMPARE(wellFormed, false);
}


QByteArray readFile(const QString &filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QXmlStreamReader reader;

    reader.setDevice(&file);
    QByteArray outarray;
    QTextStream writer(&outarray);

    while (!reader.atEnd()) {
        reader.readNext();
        writer << reader.tokenString() << "(";
        if (reader.isWhitespace())
            writer << " whitespace";
        if (reader.isCDATA())
            writer << " CDATA";
        if (reader.isStartDocument() && reader.isStandaloneDocument())
            writer << " standalone";
        if (!reader.text().isEmpty())
            writer << " text=\"" << reader.text().toString() << "\"";
        if (!reader.processingInstructionTarget().isEmpty())
            writer << " processingInstructionTarget=\"" << reader.processingInstructionTarget().toString() << "\"";
        if (!reader.processingInstructionData().isEmpty())
            writer << " processingInstructionData=\"" << reader.processingInstructionData().toString() << "\"";
        if (!reader.name().isEmpty())
            writer << " name=\"" << reader.name().toString() << "\"";
        if (!reader.namespaceUri().isEmpty())
            writer << " namespaceUri=\"" << reader.namespaceUri().toString() << "\"";
        if (!reader.qualifiedName().isEmpty())
            writer << " qualifiedName=\"" << reader.qualifiedName().toString() << "\"";
        if (reader.attributes().size()) {
            foreach(QXmlStreamAttribute attribute, reader.attributes()) {
                writer << endl << "    Attribute(";
                if (!attribute.name().isEmpty())
                    writer << " name=\"" << attribute.name().toString() << "\"";
                if (!attribute.namespaceUri().isEmpty())
                    writer << " namespaceUri=\"" << attribute.namespaceUri().toString() << "\"";
                if (!attribute.qualifiedName().isEmpty())
                    writer << " qualifiedName=\"" << attribute.qualifiedName().toString() << "\"";
                if (!attribute.value().isEmpty())
                    writer << " value=\"" << attribute.value().toString() << "\"";
                writer << " )" << endl;
            }
        }
        writer << " )" << endl;
    }
    if (reader.error())
        writer << "ERROR: " << reader.errorString() << endl;
    return outarray;
}

void tst_QXmlStream::testReader()
{
    QFETCH(QString, xml);
    QFETCH(QString, ref);
    QFile file(ref);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString reference = QString::fromUtf8(file.readAll());
    QString qxmlstream = QString::fromUtf8(readFile(xml));
    QCOMPARE(qxmlstream, reference);
}

void tst_QXmlStream::testReader_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("ref");
    QDir dir;
    dir.cd("data/");
    foreach(QString filename , dir.entryList(QStringList() << "*.xml")) {
        QString reference =  QFileInfo(filename).baseName() + ".ref";
        QTest::newRow(dir.filePath(filename).toLatin1().data()) << dir.filePath(filename) << dir.filePath(reference);
    }
}


QTEST_MAIN(tst_QXmlStream)
#include "tst_qxmlstream.moc"
