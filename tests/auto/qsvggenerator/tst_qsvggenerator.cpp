/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>

#include <qapplication.h>
#include <qdebug.h>
#include <qpainter.h>
#include <qsvggenerator.h>


//TESTED_CLASS=
//TESTED_FILES=qsvggenerator.h

class tst_QSvgGenerator : public QObject
{
Q_OBJECT

public:
    tst_QSvgGenerator();
    virtual ~tst_QSvgGenerator();

private slots:
    void construction();
    void fileName();
    void outputDevice();
    void size();
};

tst_QSvgGenerator::tst_QSvgGenerator()
{
}

tst_QSvgGenerator::~tst_QSvgGenerator()
{
}

void tst_QSvgGenerator::construction()
{
    QSvgGenerator generator;
    QCOMPARE(generator.fileName(), QString());
    QCOMPARE(generator.outputDevice(), (QIODevice *)0);
    QCOMPARE(generator.resolution(), 72);
    QCOMPARE(generator.size(), QSize(100, 100));
}

static void removeAttribute(const QDomNode &node, const QString &attribute)
{
    if (node.isNull())
        return;

    node.toElement().removeAttribute(attribute);

    removeAttribute(node.firstChild(), attribute);
    removeAttribute(node.nextSibling(), attribute);
}

static void compareWithoutFontInfo(const QByteArray &source, const QByteArray &reference)
{
    QDomDocument sourceDoc;
    sourceDoc.setContent(source);

    QDomDocument referenceDoc;
    referenceDoc.setContent(reference);

    QList<QString> fontAttributes;
    fontAttributes << "font-family" << "font-size" << "font-weight" << "font-style";

    foreach (QString attribute, fontAttributes) {
        removeAttribute(sourceDoc, attribute);
        removeAttribute(referenceDoc, attribute);
    }

    QCOMPARE(sourceDoc.toByteArray(), referenceDoc.toByteArray());
}

void tst_QSvgGenerator::fileName()
{
    QString fileName = "fileName_output.svg";
    QFile::remove(fileName);
    
    QSvgGenerator generator;
    generator.setFileName(fileName);
    QCOMPARE(generator.fileName(), fileName);

    QPainter painter(&generator);
    painter.fillRect(0, 0, 100, 100, Qt::red);
    painter.end();

    QVERIFY(QFile::exists(fileName));

    QFile file(fileName);
    QVERIFY(file.open(QIODevice::ReadOnly));

    QFile referenceFile("referenceSvgs/" + fileName);
    QVERIFY(referenceFile.open(QIODevice::ReadOnly));

    compareWithoutFontInfo(file.readAll(), referenceFile.readAll());
}

void tst_QSvgGenerator::outputDevice()
{
    QString fileName = "outputDevice_output.svg";
    QFile::remove(fileName);
    
    QFile file(fileName);

    {
        // Device is not open
        QSvgGenerator generator;
        generator.setOutputDevice(&file);
        QCOMPARE(generator.outputDevice(), (QIODevice *)&file);

        QPainter painter;
        QVERIFY(painter.begin(&generator));
        QCOMPARE(file.openMode(), QIODevice::OpenMode(QIODevice::Text | QIODevice::WriteOnly));
        file.close();
    }
    {
        // Device is not open, WriteOnly
        file.open(QIODevice::WriteOnly);
        
        QSvgGenerator generator;
        generator.setOutputDevice(&file);
        QCOMPARE(generator.outputDevice(), (QIODevice *)&file);

        QPainter painter;
        QVERIFY(painter.begin(&generator));
        QCOMPARE(file.openMode(), QIODevice::OpenMode(QIODevice::WriteOnly));
        file.close();
    }
    {
        // Device is not open, ReadOnly
        file.open(QIODevice::ReadOnly);
        
        QSvgGenerator generator;
        generator.setOutputDevice(&file);
        QCOMPARE(generator.outputDevice(), (QIODevice *)&file);

        QPainter painter;
        QTest::ignoreMessage(QtWarningMsg, "QSvgPaintEngine::begin(), could not write to read-only output device: 'Unknown error'");
        QVERIFY(!painter.begin(&generator));
        QCOMPARE(file.openMode(), QIODevice::OpenMode(QIODevice::ReadOnly));
        file.close();
    }
}

void tst_QSvgGenerator::size()
{
    QString fileName = "size_output.svg";
    QFile::remove(fileName);
    
    QSvgGenerator generator;
    generator.setSize(QSize(320, 240));
    generator.setFileName(fileName);
    QCOMPARE(generator.fileName(), fileName);

    QPainter painter(&generator);
    painter.fillRect(0, 0, 100, 100, Qt::red);
    painter.end();

    QVERIFY(QFile::exists(fileName));

    QFile file(fileName);
    QVERIFY(file.open(QIODevice::ReadOnly));

    QFile referenceFile("referenceSvgs/" + fileName);
    QVERIFY(referenceFile.open(QIODevice::ReadOnly));

    compareWithoutFontInfo(file.readAll(), referenceFile.readAll());
}

QTEST_MAIN(tst_QSvgGenerator)
#include "tst_qsvggenerator.moc"
