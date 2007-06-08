/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qapplication.h>
#include <QtCore/QSet>
#include <QtCore/QTranslator>
#include <private/qthread_p.h>
#include <QtGui/QInputDialog>
#include <QtGui/QColorDialog>
#include <QtGui/QFileDialog>


//TESTED_CLASS=
//TESTED_FILES=

class tst_languageChange : public QObject
{
    Q_OBJECT
public:
    tst_languageChange();

public slots:
    void initTestCase();
    void cleanupTestCase();
private slots:
    void retranslatability_data();
    void retranslatability();

};


tst_languageChange::tst_languageChange()

{
}

void tst_languageChange::initTestCase()
{
}

void tst_languageChange::cleanupTestCase()
{
}
/**
 * Records all calls to translate()
 */
class TransformTranslator : public QTranslator
{
    Q_OBJECT
public:
    TransformTranslator() : QTranslator() {}
    TransformTranslator(QObject *parent) : QTranslator(parent) {}
    virtual QString translate(const char *context, const char *sourceText, const char *comment = 0) const
    {
        QByteArray total(context);
        total.append("::");
        total.append(sourceText);
        if (comment) {
            total.append("::");
            total.append(comment);
        }
        m_translations.insert(total);
        QString res;
        for (int i = 0; i < int(qstrlen(sourceText)); ++i) {
            QChar ch = QLatin1Char(sourceText[i]);
            if (ch.isLower()) {
                res.append(ch.toUpper());
            } else if (ch.isUpper()) {
                res.append(ch.toLower());
            } else {
                res.append(ch);
            }
        }
        return res;
    }

    virtual bool isEmpty() const { return false; }
 
public slots:
    void install() {
        QCoreApplication::installTranslator(this);
        QTest::qWait(1500);
        //### is there any better way to close a Qt dialog?
        QThreadData *data = QThreadData::current(); 
        if (!data->eventLoops.isEmpty()) 
            data->eventLoops.top()->quit();
    }
public:
    mutable QSet<QByteArray> m_translations;
};

enum DialogType {
    InputDialog = 1,
    ColorDialog,
    FileDialog
};

typedef QSet<QByteArray> TranslationSet;
Q_DECLARE_METATYPE(TranslationSet)

void tst_languageChange::retranslatability_data()
{
    QTest::addColumn<int>("dialogType");
    QTest::addColumn<TranslationSet >("expected");

    //next we fill it with data
    QTest::newRow( "QInputDialog" )  
        << int(InputDialog) << (QSet<QByteArray>()  
                    << "QDialogButtonBox::OK" 
                    << "QDialogButtonBox::Cancel");

    QTest::newRow( "QColorDialog" )
        << int(ColorDialog) << (QSet<QByteArray>()  
                    << "QColorDialog::Cancel"
                    << "QColorDialog::&Sat:"
                    << "QColorDialog::&Add to Custom Colors"
                    << "QColorDialog::&Green:"
                    << "QColorDialog::&Red:"
                    << "QColorDialog::Bl&ue:"
                    << "QColorDialog::A&lpha channel:"
                    << "QColorDialog::&Basic colors"
                    << "QColorDialog::&Custom colors"
                    << "QColorDialog::&Val:"
                    << "QColorDialog::&Define Custom Colors >>"
                    << "QColorDialog::OK"
                    << "QColorDialog::Hu&e:");

    QTest::newRow( "QFileDialog" )
        << int(FileDialog) << (QSet<QByteArray>()
                    << "QFileDialog::All Files (*)"
                    << "QFileDialog::Back"
                    << "QFileDialog::Create New Folder"
                    << "QFileDialog::Detail View"
                    << "QFileDialog::Drive"
                    << "QFileDialog::File"
                    << "QFileDialog::File Folder::Match Windows Explorer"
                    << "QFileDialog::Files of type:"
                    << "QFileDialog::Forward"
                    << "QFileDialog::List View"
                    << "QFileDialog::Look in:"
                    << "QFileDialog::Open"
                    << "QFileDialog::Parent Directory"
                    << "QFileDialog::Show "
                    << "QFileDialog::Show &hidden files"
                    << "QFileDialog::&Delete"
                    << "QFileDialog::&New Folder"
                    << "QFileDialog::&Rename"
                    << "QFileSystemModel::Date Modified"
                    << "QFileSystemModel::My Computer"
                    << "QFileSystemModel::Size"
                    << "QFileSystemModel::Type::All other platforms"
                    << "QFileSystemModel::%1 KB"
                    << "QDialogButtonBox::Cancel"
                    << "QDialogButtonBox::Open"
                    << "QFileDialog::File &name");
}

void tst_languageChange::retranslatability()
{
    QFETCH( int, dialogType);
    QFETCH( TranslationSet, expected);

    // This will always be queried for when a language changes
    expected.insert("QApplication::QT_LAYOUT_DIRECTION::Translate this string to the string 'LTR' in left-to-right "
                       "languages or to 'RTL' in right-to-left languages (such as Hebrew and Arabic) to "
                       "get proper widget layout.");
    TransformTranslator translator;
    QTimer::singleShot(500, &translator, SLOT(install()));
    switch (dialogType) {
    case InputDialog:
        (void)QInputDialog::getInteger(0, QLatin1String("title"), QLatin1String("label"));
        break;
    case ColorDialog:
        (void)QColorDialog::getColor();
        break;
    case FileDialog: {
        QFileDialog dlg;
        dlg.setFileMode(QFileDialog::ExistingFiles);
        dlg.exec();
        break; }
    }
#if 0
    QList<QByteArray> list = translator.m_translations.toList();
    qSort(list);
    qDebug() << list;
#endif
    // see if all of our *expected* translations was translated. 
    // (There might be more, but thats not that bad)
    expected.subtract(translator.m_translations);
    if (!expected.isEmpty())
        qDebug() << expected;

    QCOMPARE(expected.isEmpty(),true);
}

QTEST_MAIN(tst_languageChange)
#include "tst_languagechange.moc"
