#include <QtGui>
#include "qcompleter.h"

#include <QtTest/QtTest>
#include <QtGui>
#include <QtDebug>
#include <QPair>
#include <QList>
#include <QPointer>

//TESTED_CLASS=QCompleter
//TESTED_FILES=gui/qcompleter.cpp gui/qcompleter.h

class CsvCompleter : public QCompleter
{
    Q_OBJECT
public:
    CsvCompleter(QObject *parent = 0) : QCompleter(parent), csv(true) { }
    
    QString pathFromIndex(const QModelIndex& sourceIndex) const; 
    
    void setCsvCompletion(bool set) { csv = set; }

protected:
    QStringList splitPath(const QString &path) const {
        return csv ? path.split(",") : QCompleter::splitPath(path);
    }

private:
    bool csv;
};

QString CsvCompleter::pathFromIndex(const QModelIndex& si) const
{
    if (!csv)
        return QCompleter::pathFromIndex(si);

    if (!si.isValid())
        return QString();

    QModelIndex idx = si;
    QStringList list;
    do {
        QString t = model()->data(idx, completionRole()).toString();
        list.prepend(t);
        QModelIndex parent = idx.parent();
        idx = parent.sibling(parent.row(), si.column());
    } while (idx.isValid());

    if (list.count() == 1)
        return list[0];
    return list.join(",");
}

class tst_QCompleter : public QObject
{
    Q_OBJECT
public:
    tst_QCompleter();
    ~tst_QCompleter();

private slots:
    void csMatchingOnCsSortedModel_data();
    void csMatchingOnCsSortedModel();
    void ciMatchingOnCiSortedModel_data();
    void ciMatchingOnCiSortedModel();

    void ciMatchingOnCsSortedModel_data();
    void ciMatchingOnCsSortedModel();
    void csMatchingOnCiSortedModel_data();
    void csMatchingOnCiSortedModel();

    void directoryModel_data();
    void directoryModel();

    void changingModel_data();
    void changingModel();

    void sortedEngineRowCount_data();
    void sortedEngineRowCount();
    void unsortedEngineRowCount_data();
    void unsortedEngineRowCount();

    void currentRow();
    void sortedEngineMapFromSource();
    void unsortedEngineMapFromSource();

    void historySearch();

    void modelDeletion();
    void setters();

    void multipleWidgets();
    void focusIn();

    void dynamicSortOrder();

private:
    void filter();
    void testRowCount();
    enum ModelType {
        CASE_SENSITIVELY_SORTED_MODEL,
        CASE_INSENSITIVELY_SORTED_MODEL,
        DIRECTORY_MODEL,
        HISTORY_MODEL
    };
    void setSourceModel(ModelType);

    CsvCompleter *completer;
    QTreeWidget *treeWidget;
    const int completionColumn;
    const int columnCount;
};

tst_QCompleter::tst_QCompleter() : completer(0), completionColumn(0), columnCount(3)
{
    treeWidget = new QTreeWidget;
    treeWidget->setColumnCount(columnCount);
}

tst_QCompleter::~tst_QCompleter()
{
    delete treeWidget;
    delete completer;
}

void tst_QCompleter::setSourceModel(ModelType type)
{   
    QString text;
    QTreeWidgetItem *parent, *child;
    treeWidget->clear();
    switch(type) {
    case CASE_SENSITIVELY_SORTED_MODEL:
        // Creates a tree model with top level items P0, P1, .., p0, p1,..
        // Each of these items parents have children (for P0 - c0P0, c1P0,...)
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 5; j++) {
                parent = new QTreeWidgetItem(treeWidget);
                text.sprintf("%c%i", i == 0 ? 'P' : 'p', j);
                parent->setText(completionColumn, text);
                for (int k = 0; k < 5; k++) {
                    child = new QTreeWidgetItem(parent);
                    QString t = QString().sprintf("c%i", k) + text;
                    child->setText(completionColumn, t);
                }
            }
        }
        completer->setModel(treeWidget->model());
        completer->setCompletionColumn(completionColumn);
        break;
    case CASE_INSENSITIVELY_SORTED_MODEL:
    case HISTORY_MODEL:
        // Creates a tree model with top level items P0, p0, P1, p1,...
        // Each of these items have children c0p0, c1p0,..
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 2; j++) {
                parent = new QTreeWidgetItem(treeWidget);
                text.sprintf("%c%i", j == 0 ? 'P' : 'p', i);
                parent->setText(completionColumn, text);
                for (int k = 0; k < 5; k++) {
                    child = new QTreeWidgetItem(parent);
                    QString t = QString().sprintf("c%i", k) + text;
                    child->setText(completionColumn, t);
                }
            }
        }
        completer->setModel(treeWidget->model());
        completer->setCompletionColumn(completionColumn);
        if (type == CASE_INSENSITIVELY_SORTED_MODEL)
            break;
        parent = new QTreeWidgetItem(treeWidget);
        parent->setText(completionColumn, QLatin1String("p3,c3p3"));
        parent = new QTreeWidgetItem(treeWidget);
        parent->setText(completionColumn, QLatin1String("p2,c4p2"));
        break;
    case DIRECTORY_MODEL:
        completer->setCsvCompletion(false);
        completer->setModel(new QDirModel(completer));
        completer->setCompletionColumn(0);
        break;
    default:
        qDebug() << "Invalid type";
    }
}

void tst_QCompleter::filter()
{
    QFETCH(QString, filterText);
    QFETCH(QString, step);
    QFETCH(QString, completion);
    QFETCH(QString, completionText);

    if (filterText.compare("FILTERING_OFF", Qt::CaseInsensitive) == 0) {
        completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
        return;
    }

    completer->setCompletionPrefix(filterText);

    for (int i = 0; i < step.length(); i++) {
        int row = completer->currentRow();
        switch (step[i].toUpper().toAscii()) {
        case 'P': --row; break;
        case 'N': ++row; break;
        case 'L': row = completer->completionCount() - 1; break;
        case 'F': row = 0; break;
        default:
            Q_ASSERT(false);
        }
        completer->setCurrentRow(row);
    }

    //QModelIndex si = completer->currentIndex();
    //QCOMPARE(completer->model()->data(si).toString(), completion);
    QCOMPARE(completer->currentCompletion(), completionText);
}

void tst_QCompleter::csMatchingOnCsSortedModel_data()
{
    delete completer;
    completer = new CsvCompleter;
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseSensitive);
    setSourceModel(CASE_SENSITIVELY_SORTED_MODEL);

    QTest::addColumn<QString>("filterText");
    QTest::addColumn<QString>("step");
    QTest::addColumn<QString>("completion");
    QTest::addColumn<QString>("completionText");

    for (int i = 0; i < 2; i++) {
         if (i == 1)
             QTest::newRow("FILTERING_OFF") << "FILTERING_OFF" << "" << "" << "";
 
         // Plain text filter
         QTest::newRow("()") << "" << "" << "P0" << "P0";
         QTest::newRow("()F") << "" << "F" << "P0" << "P0";
         QTest::newRow("()L") << "" << "L" << "p4" << "p4";
         QTest::newRow("()L") << "" << "L" << "p4" << "p4";
         QTest::newRow("()N") << "" << "N" << "P1" << "P1";
         QTest::newRow("(P)") << "P" << "" << "P0" << "P0";
         QTest::newRow("(P)F") << "P" << "" << "P0" << "P0";
         QTest::newRow("(P)L") << "P" << "L" << "P4" << "P4";
         QTest::newRow("(p)") << "p" << "" << "p0" << "p0";
         QTest::newRow("(p)N") << "p" << "N" << "p1" << "p1";
         QTest::newRow("(p)NN") << "p" << "NN" << "p2" << "p2";
         QTest::newRow("(p)NNN") << "p" << "NNN" << "p3" << "p3";
         QTest::newRow("(p)NNNN") << "p" << "NNNN" << "p4" << "p4";
         QTest::newRow("(p1)") << "p1" << "" << "p1" << "p1";
         QTest::newRow("(p11)") << "p11" << "" << "" << "";
 
         // Tree filter
         QTest::newRow("(P0,)") << "P0," << "" << "c0P0" << "P0,c0P0";
         QTest::newRow("(P0,c)") << "P0,c" << "" << "c0P0" << "P0,c0P0";
         QTest::newRow("(P0,c1)") << "P0,c1" << "" << "c1P0" << "P0,c1P0";
         QTest::newRow("(P0,c3P0)") << "P0,c3P0" << "" << "c3P0" << "P0,c3P0";
         QTest::newRow("(P3,c)F") << "P3,c" << "F" << "c0P3" << "P3,c0P3";
         QTest::newRow("(P3,c)L") << "P3,c" << "L" << "c4P3" << "P3,c4P3";
         QTest::newRow("(P3,c)N") << "P3,c" << "N" << "c1P3" << "P3,c1P3";
         QTest::newRow("(P3,c)NN") << "P3,c" << "NN" << "c2P3" << "P3,c2P3";
         QTest::newRow("(P3,,c)") << "P3,,c" << "" << "" << "";
         QTest::newRow("(P3,c0P3,)") << "P3,c0P3," << "" << "" << "";
         QTest::newRow("(P,)") << "P," << "" << "" << "";
     }
}

void tst_QCompleter::csMatchingOnCsSortedModel()
{
    filter();
}

void tst_QCompleter::ciMatchingOnCiSortedModel_data()
{
    delete completer;
    completer = new CsvCompleter;
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    setSourceModel(CASE_INSENSITIVELY_SORTED_MODEL);

    QTest::addColumn<QString>("filterText");
    QTest::addColumn<QString>("step");
    QTest::addColumn<QString>("completion");
    QTest::addColumn<QString>("completionText");

    for (int i = 0; i < 2; i++) {        
        if (i == 1)
            QTest::newRow("FILTERING_OFF") << "FILTERING_OFF" << "" << "" << "";

        // Plain text filter
        QTest::newRow("()") << "" << "" << "P0" << "P0";
        QTest::newRow("()F") << "" << "F" << "P0" << "P0";
        QTest::newRow("()L") << "" << "L" << "p4" << "p4";
        QTest::newRow("()N") << "" << "N" << "p0" << "p0";
        QTest::newRow("(P)") << "P" << "" << "P0" << "P0";
        QTest::newRow("(P)F") << "P" << "" << "P0" << "P0";
        QTest::newRow("(P)L") << "P" << "L" << "p4" << "p4";
        QTest::newRow("(p)") << "p" << "" << "P0" << "P0";
        QTest::newRow("(p)N") << "p" << "N" << "p0" << "p0";
        QTest::newRow("(p)NN") << "p" << "NN" << "P1" << "P1";
        QTest::newRow("(p)NNN") << "p" << "NNN" << "p1" << "p1";
        QTest::newRow("(p1)") << "p1" << "" << "P1" << "P1";
        QTest::newRow("(p1)N") << "p1" << "N" << "p1" << "p1";
        QTest::newRow("(p11)") << "p11" << "" << "" << "";
    
        //// Tree filter
        QTest::newRow("(p0,)") << "p0," << "" << "c0P0" << "P0,c0P0";
        QTest::newRow("(p0,c)") << "p0,c" << "" << "c0P0" << "P0,c0P0";
        QTest::newRow("(p0,c1)") << "p0,c1" << "" << "c1P0" << "P0,c1P0";
        QTest::newRow("(p0,c3P0)") << "p0,c3P0" << "" << "c3P0" << "P0,c3P0";
        QTest::newRow("(p3,c)F") << "p3,c" << "F" << "c0P3" << "P3,c0P3";
        QTest::newRow("(p3,c)L") << "p3,c" << "L" << "c4P3" << "P3,c4P3";
        QTest::newRow("(p3,c)N") << "p3,c" << "N" << "c1P3" << "P3,c1P3";
        QTest::newRow("(p3,c)NN") << "p3,c" << "NN" << "c2P3" << "P3,c2P3";
        QTest::newRow("(p3,,c)") << "p3,,c" << "" << "" << "";
        QTest::newRow("(p3,c0P3,)") << "p3,c0P3," << "" << "" << "";
        QTest::newRow("(p,)") << "p," << "" << "" << "";
    }
}

void tst_QCompleter::ciMatchingOnCiSortedModel()
{
    filter();
}

void tst_QCompleter::ciMatchingOnCsSortedModel_data()
{
    delete completer;
    completer = new CsvCompleter;
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    setSourceModel(CASE_SENSITIVELY_SORTED_MODEL);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    QTest::addColumn<QString>("filterText");
    QTest::addColumn<QString>("step");
    QTest::addColumn<QString>("completion");
    QTest::addColumn<QString>("completionText");

    for (int i = 0; i < 2; i++) {
        if (i == 1)
            QTest::newRow("FILTERING_OFF") << "FILTERING_OFF" << "" << "" << "";

        // Plain text filter
        QTest::newRow("()") << "" << "" << "P0" << "P0";
        QTest::newRow("()F") << "" << "F" << "P0" << "P0";
        QTest::newRow("()L") << "" << "L" << "p4" << "p4";
        QTest::newRow("(P)") << "P" << "" << "P0" << "P0";
        QTest::newRow("(P)F") << "P" << "" << "P0" << "P0";
        QTest::newRow("(P)L") << "P" << "L" << "p4" << "p4";
        QTest::newRow("(p)") << "p" << "" << "P0" << "P0";
        QTest::newRow("(p)N") << "p" << "N" << "P1" << "P1";
        QTest::newRow("(p)NN") << "p" << "NN" << "P2" << "P2";
        QTest::newRow("(p)NNN") << "p" << "NNN" << "P3" << "P3";
        QTest::newRow("(p1)") << "p1" << "" << "P1" << "P1";
        QTest::newRow("(p1)N") << "p1" << "N" << "p1" << "p1";
        QTest::newRow("(p11)") << "p11" << "" << "" << "";
    
        // Tree filter
        QTest::newRow("(p0,)") << "p0," << "" << "c0P0" << "P0,c0P0";
        QTest::newRow("(p0,c)") << "p0,c" << "" << "c0P0" << "P0,c0P0";
        QTest::newRow("(p0,c1)") << "p0,c1" << "" << "c1P0" << "P0,c1P0";
        QTest::newRow("(p0,c3P0)") << "p0,c3P0" << "" << "c3P0" << "P0,c3P0";
        QTest::newRow("(p3,c)F") << "p3,c" << "F" << "c0P3" << "P3,c0P3";
        QTest::newRow("(p3,c)L") << "p3,c" << "L" << "c4P3" << "P3,c4P3";
        QTest::newRow("(p3,c)N") << "p3,c" << "N" << "c1P3" << "P3,c1P3";
        QTest::newRow("(p3,c)NN") << "p3,c" << "NN" << "c2P3" << "P3,c2P3";
        QTest::newRow("(p3,,c)") << "p3,,c" << "" << "" << "";
        QTest::newRow("(p3,c0P3,)") << "p3,c0P3," << "" << "" << "";
        QTest::newRow("(p,)") << "p," << "" << "" << "";
    }
}

void tst_QCompleter::ciMatchingOnCsSortedModel()
{
    filter();
}

void tst_QCompleter::csMatchingOnCiSortedModel_data()
{
    delete completer;
    completer = new CsvCompleter;
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    setSourceModel(CASE_INSENSITIVELY_SORTED_MODEL);
    completer->setCaseSensitivity(Qt::CaseSensitive);

    QTest::addColumn<QString>("filterText");
    QTest::addColumn<QString>("step");
    QTest::addColumn<QString>("completion");
    QTest::addColumn<QString>("completionText");

    for (int i = 0; i < 2; i++) {
        if (i == 1)
            QTest::newRow("FILTERING_OFF") << "FILTERING_OFF" << "" << "" << "";

        // Plain text filter
        QTest::newRow("()") << "" << "" << "P0" << "P0";
        QTest::newRow("()F") << "" << "F" << "P0" << "P0";
        QTest::newRow("()L") << "" << "L" << "p4" << "p4";
        QTest::newRow("()N") << "" << "N" << "p0" << "p0";
        QTest::newRow("(P)") << "P" << "" << "P0" << "P0";
        QTest::newRow("(P)F") << "P" << "" << "P0" << "P0";
        QTest::newRow("(P)L") << "P" << "L" << "P4" << "P4";
        QTest::newRow("(p)") << "p" << "" << "p0" << "p0";
        QTest::newRow("(p)N") << "p" << "N" << "p1" << "p1";
        QTest::newRow("(p)NN") << "p" << "NN" << "p2" << "p2";
        QTest::newRow("(p)NNN") << "p" << "NNN" << "p3" << "p3";
        QTest::newRow("(p1)") << "p1" << "" << "p1" << "p1";
        QTest::newRow("(p11)") << "p11" << "" << "" << "";
    
        //// Tree filter
        QTest::newRow("(p0,)") << "p0," << "" << "c0p0" << "p0,c0p0";
        QTest::newRow("(p0,c)") << "p0,c" << "" << "c0p0" << "p0,c0p0";
        QTest::newRow("(p0,c1)") << "p0,c1" << "" << "c1p0" << "p0,c1p0";
        QTest::newRow("(p0,c3P0)") << "p0,c3p0" << "" << "c3p0" << "p0,c3p0";
        QTest::newRow("(p3,c)F") << "p3,c" << "F" << "c0p3" << "p3,c0p3";
        QTest::newRow("(p3,c)L") << "p3,c" << "L" << "c4p3" << "p3,c4p3";
        QTest::newRow("(p3,c)N") << "p3,c" << "N" << "c1p3" << "p3,c1p3";
        QTest::newRow("(p3,c)NN") << "p3,c" << "NN" << "c2p3" << "p3,c2p3";
        QTest::newRow("(p3,,c)") << "p3,,c" << "" << "" << "";
        QTest::newRow("(p3,c0P3,)") << "p3,c0P3," << "" << "" << "";
        QTest::newRow("(p,)") << "p," << "" << "" << "";
    
        QTest::newRow("FILTERING_OFF") << "FILTERING_OFF" << "" << "" << "";
    }
}

void tst_QCompleter::csMatchingOnCiSortedModel()
{
    filter();
}

void tst_QCompleter::directoryModel_data()
{
    delete completer;
    completer = new CsvCompleter;
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    setSourceModel(DIRECTORY_MODEL);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    QTest::addColumn<QString>("filterText");
    QTest::addColumn<QString>("step");
    QTest::addColumn<QString>("completion");
    QTest::addColumn<QString>("completionText");

    // NOTE: Add tests carefully, ensurely the paths exist on all systems
    // Output is the sourceText; currentCompletionText()

    for (int i = 0; i < 2; i++) {
        if (i == 1)
            QTest::newRow("FILTERING_OFF") << "FILTERING_OFF" << "" << "" << "";

#ifdef Q_OS_WIN
        QTest::newRow("()") << "C" << "" << "C:" << "C:";
        QTest::newRow("()") << "C:\\Program" << "" << "Program Files" << "C:\\Program Files";
#elif defined (Q_OS_MAC)
        QTest::newRow("()") << "" << "" << "/" << "/";
        QTest::newRow("(/a)") << "/a" << "" << "Applications" << "/Applications";        
        QTest::newRow("(/d)") << "/d" << "" << "Developer" << "/Developer";
#else
        QTest::newRow("()") << "" << "" << "/" << "/";
        QTest::newRow("(/h)") << "/h" << "" << "home" << "/home";
        QTest::newRow("(/et)") << "/et" << "" << "etc" << "/etc";
        QTest::newRow("(/etc/passw)") << "/etc/passw" << "" << "passwd" << "/etc/passwd";
#endif
    }
}

void tst_QCompleter::directoryModel()
{
    filter();
}

void tst_QCompleter::changingModel_data()
{
}

void tst_QCompleter::changingModel()
{
    for (int i = 0; i < 2; i++) {
        delete completer;
        completer = new CsvCompleter;
        completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
        completer->setCaseSensitivity(Qt::CaseSensitive);
        setSourceModel(CASE_SENSITIVELY_SORTED_MODEL);

        if (i == 1) {
            completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
        }

        completer->setCompletionPrefix("p");
        completer->setCurrentRow(completer->completionCount() - 1);
        QCOMPARE(completer->currentCompletion(), QString("p4"));

        // Test addition of data
        QTreeWidgetItem p5item;
        p5item.setText(completionColumn, "p5");
        treeWidget->addTopLevelItem(&p5item);
        completer->setCompletionPrefix("p5");
        QCOMPARE(completer->currentCompletion(), QString("p5"));

        // Test removal of data
        int p5index = treeWidget->indexOfTopLevelItem(&p5item);
        treeWidget->takeTopLevelItem(p5index);
        QCOMPARE(completer->currentCompletion(), QString(""));

        // Test clear
        treeWidget->clear();
        QCOMPARE(completer->currentIndex(), QModelIndex());
    }
}

void tst_QCompleter::testRowCount()
{
    QFETCH(QString, filterText);
    QFETCH(bool, hasChildren);
    QFETCH(int, rowCount);
    QFETCH(int, completionCount);

    if (filterText.compare("FILTERING_OFF", Qt::CaseInsensitive) == 0) {
        completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
        return;
    }

    completer->setCompletionPrefix(filterText);
    const QAbstractItemModel *completionModel = completer->completionModel();
    QCOMPARE(completionModel->rowCount(), rowCount);
    QCOMPARE(completionCount, completionCount);
    QCOMPARE(completionModel->hasChildren(), hasChildren);
    QCOMPARE(completionModel->columnCount(), columnCount);
}

void tst_QCompleter::sortedEngineRowCount_data()
{
    delete completer;
    completer = new CsvCompleter;
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    setSourceModel(CASE_INSENSITIVELY_SORTED_MODEL);

    QTest::addColumn<QString>("filterText");
    QTest::addColumn<bool>("hasChildren");
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("completionCount");

    QTest::newRow("whatever") << "whatever" << false << 0 << 0;
    QTest::newRow("p") << "p" << true << 10 << 10;
    QTest::newRow("p1") << "p1" << true << 2 << 2;
    QTest::newRow("P1,") << "P1," << true << 5 << 5;
    QTest::newRow("P1,c") << "P1,c" << true << 5 << 5;
    QTest::newRow("P1,cc") << "P1,cc" << false << 0 << 0;

    QTest::newRow("FILTERING_OFF") << "FILTERING_OFF" << false << 0 << 0;

    QTest::newRow("whatever(filter off)") << "whatever" << true << 10 << 0;
    QTest::newRow("p1(filter off)") << "p1" << true << 10 << 2;
    QTest::newRow("p1,(filter off)") << "p1," << true << 5 << 5;
    QTest::newRow("p1,c(filter off)") << "p1,c" << true << 5 << 5;
    QTest::newRow("P1,cc(filter off)") << "P1,cc" << true << 5 << 0;
}

void tst_QCompleter::sortedEngineRowCount()
{
    testRowCount();
}

void tst_QCompleter::unsortedEngineRowCount_data()
{
    delete completer;
    completer = new CsvCompleter;
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseSensitive);
    setSourceModel(CASE_INSENSITIVELY_SORTED_MODEL);

    QTest::addColumn<QString>("filterText");
    QTest::addColumn<bool>("hasChildren");
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("completionCount");

    QTest::newRow("whatever") << "whatever" << false << 0 << 0;
    QTest::newRow("p") << "p" << true << 5 << 5;
    QTest::newRow("p1") << "p1" << true << 1 << 1;
    QTest::newRow("P1,") << "P1," << true << 5 << 5;
    QTest::newRow("P1,c") << "P1,c" << true << 5 << 5;
    QTest::newRow("P1,cc") << "P1,cc" << false << 0 << 0;

    QTest::newRow("FILTERING_OFF") << "FILTERING_OFF" << false << 0 << 0;

    QTest::newRow("whatever(filter off)") << "whatever" << true << 10 << 0;
    QTest::newRow("p1(filter off)") << "p1" << true << 10 << 1;
    QTest::newRow("p1,(filter off)") << "p1," << true << 5 << 5;
    QTest::newRow("p1,c(filter off)") << "p1,c" << true << 5 << 5;
    QTest::newRow("P1,cc(filter off)") << "P1,cc" << true << 5 << 0;
}

void tst_QCompleter::unsortedEngineRowCount()
{
    testRowCount();
}

void tst_QCompleter::currentRow()
{
    delete completer;
    completer = new CsvCompleter;
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    setSourceModel(CASE_INSENSITIVELY_SORTED_MODEL);
 
    // blank text
    completer->setCompletionPrefix("");
    QCOMPARE(completer->currentRow(), 0);
    QVERIFY(completer->setCurrentRow(4));
    QCOMPARE(completer->currentRow(), 4);
    QVERIFY(!completer->setCurrentRow(13));
    QVERIFY(completer->setCurrentRow(4));

    // some text
     completer->setCompletionPrefix("p1");
    QCOMPARE(completer->currentRow(), 0);
    QVERIFY(completer->setCurrentRow(1));
    QCOMPARE(completer->currentRow(), 1);
    QVERIFY(!completer->setCurrentRow(2));
    QCOMPARE(completer->currentRow(), 1);

    // invalid text
    completer->setCompletionPrefix("well");
    QCOMPARE(completer->currentRow(), -1);
}

void tst_QCompleter::sortedEngineMapFromSource()
{
    delete completer;
    completer = new CsvCompleter;
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    setSourceModel(CASE_INSENSITIVELY_SORTED_MODEL);

    QModelIndex si1, si2, pi;
    QAbstractItemModel *sourceModel = completer->model();
    const QAbstractProxyModel *completionModel = 
        qobject_cast<const QAbstractProxyModel *>(completer->completionModel());

    // Fitering ON
    // empty
    si1 = sourceModel->index(4, completionColumn); // "P2"
    si2 = sourceModel->index(2, 0, si1); // "P2,c0P2"
    pi = completionModel->mapFromSource(si1);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P2"));
    pi = completionModel->mapFromSource(si2);
    QCOMPARE(pi.isValid(), false);

    // some text
    completer->setCompletionPrefix("p");
    pi = completionModel->mapFromSource(si1);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P2"));
    pi = completionModel->mapFromSource(si2);
    QCOMPARE(pi.isValid(), false);

    // more text
    completer->setCompletionPrefix("p2");
    pi = completionModel->mapFromSource(si1);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P2"));
    pi = completionModel->mapFromSource(si2);
    QCOMPARE(pi.isValid(), false);

    // invalid text
    completer->setCompletionPrefix("whatever");
    pi = completionModel->mapFromSource(si1);
    QVERIFY(!pi.isValid());

    // Fitering OFF
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    // empty
    si1 = sourceModel->index(4, completionColumn); // "P2"
    si2 = sourceModel->index(2, 0, si1); // "P2,c0P2"
    pi = completionModel->mapFromSource(si1);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P2"));
    pi = completionModel->mapFromSource(si2);
    QCOMPARE(pi.isValid(), false);

    // some text
    completer->setCompletionPrefix("p");
    pi = completionModel->mapFromSource(si1);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P2"));
    pi = completionModel->mapFromSource(si2);
    QCOMPARE(pi.isValid(), false);

    // more text
    completer->setCompletionPrefix("p2");
    pi = completionModel->mapFromSource(si1);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P2"));
    pi = completionModel->mapFromSource(si2);
    QCOMPARE(pi.isValid(), false);

    // invalid text
    completer->setCompletionPrefix("whatever");
    pi = completionModel->mapFromSource(si1);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P2"));
}

void tst_QCompleter::unsortedEngineMapFromSource()
{
    delete completer;
    completer = new CsvCompleter;
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    setSourceModel(HISTORY_MODEL); // case insensitively sorted model
    completer->setModelSorting(QCompleter::UnsortedModel);

    QModelIndex si, si2, si3, pi;
    QAbstractItemModel *sourceModel = completer->model();
    const QAbstractProxyModel *completionModel = 
        qobject_cast<const QAbstractProxyModel *>(completer->completionModel());

    si = sourceModel->index(6, completionColumn); // "P3"
    QCOMPARE(si.data().toString(), QLatin1String("P3"));
    si2 = sourceModel->index(3, completionColumn, sourceModel->index(0, completionColumn)); // "P0,c3P0"
    QCOMPARE(si2.data().toString(), QLatin1String("c3P0"));
    si3 = sourceModel->index(10, completionColumn); // "p3,c3p3" (history)
    QCOMPARE(si3.data().toString(), QLatin1String("p3,c3p3"));

    // FILTERING ON
    // empty
    pi = completionModel->mapFromSource(si);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P3"));
    pi = completionModel->mapFromSource(si2);
    QCOMPARE(pi.isValid(), false);
    pi = completionModel->mapFromSource(si3);
   QCOMPARE(completionModel->data(pi).toString(), QLatin1String("p3,c3p3"));

    // some text
    completer->setCompletionPrefix("P");
    pi = completionModel->mapFromSource(si);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P3"));
    pi = completionModel->mapFromSource(si2);
    QCOMPARE(pi.isValid(), false);
    pi = completionModel->mapFromSource(si3);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("p3,c3p3"));

    // invalid text
    completer->setCompletionPrefix("whatever");
    pi = completionModel->mapFromSource(si);
    QVERIFY(!pi.isValid());
    pi = completionModel->mapFromSource(si2);
    QVERIFY(!pi.isValid());

    // tree matching
    completer->setCompletionPrefix("P0,c");
    pi = completionModel->mapFromSource(si);
    QVERIFY(!pi.isValid());
    pi = completionModel->mapFromSource(si2);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("c3P0"));
    pi = completionModel->mapFromSource(si3);
    QCOMPARE(pi.isValid(), false);

    // more tree matching
    completer->setCompletionPrefix("p3,");
    pi = completionModel->mapFromSource(si2);
    QVERIFY(!pi.isValid());
    pi = completionModel->mapFromSource(si3);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("p3,c3p3"));

    // FILTERING OFF
    // empty
    completer->setCompletionPrefix("");
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    pi = completionModel->mapFromSource(si);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P3"));

    // some text
    completer->setCompletionPrefix("P");
    pi = completionModel->mapFromSource(si);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P3"));

    // more text
    completer->setCompletionPrefix("P3");
    pi = completionModel->mapFromSource(si);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P3"));

    // invalid text
    completer->setCompletionPrefix("whatever");
    pi = completionModel->mapFromSource(si);
    QCOMPARE(completionModel->data(pi).toString(), QLatin1String("P3"));
}

void tst_QCompleter::historySearch()
{
    delete completer;
    completer = new CsvCompleter;
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseSensitive);
    setSourceModel(HISTORY_MODEL);

    const QAbstractProxyModel *completionModel = 
      qobject_cast<const QAbstractProxyModel *>(completer->completionModel());

    // "p3,c3p3" and "p2,c4p2" are added in the tree root

    // FILTERING ON
    // empty
    completer->setCurrentRow(10);
    QCOMPARE(completer->currentCompletion(), QLatin1String("p3,c3p3"));

    // more text
    completer->setCompletionPrefix("p2");
    completer->setCurrentRow(1);
    QCOMPARE(completer->currentCompletion(), QLatin1String("p2,c4p2"));

    // comma separated text
    completer->setCompletionPrefix("p2,c4");
    completer->setCurrentRow(1);
    QCOMPARE(completionModel->rowCount(), 2);
    QCOMPARE(completer->currentCompletion(), QLatin1String("p2,c4p2"));

    // invalid text
    completer->setCompletionPrefix("whatever");
    QCOMPARE(completer->currentCompletion(), QString());

    // FILTERING OFF
    completer->setCompletionPrefix("");
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCurrentRow(10);
    QCOMPARE(completer->currentCompletion(), QLatin1String("p3,c3p3"));

    // more text
    completer->setCompletionPrefix("p2");
    completer->setCurrentRow(1);
    QCOMPARE(completer->currentCompletion(), QLatin1String("p2,c4p2"));

    // comma separated text
    completer->setCompletionPrefix("p2,c4");
    QCOMPARE(completionModel->rowCount(), 5);

    // invalid text
    completer->setCompletionPrefix("whatever");
    QCOMPARE(completer->currentCompletion(), QString());
}

void tst_QCompleter::setters()
{
    delete completer;
    completer = new CsvCompleter;
    QVERIFY(completer->popup() != 0);
    QPointer<QDirModel> dirModel = new QDirModel(completer);
    QAbstractItemModel *oldModel = completer->model();
    completer->setModel(dirModel);
    QVERIFY(completer->popup()->model() != oldModel);
    QVERIFY(completer->popup()->model() == completer->completionModel());
    completer->setPopup(new QListView);
    QVERIFY(completer->popup()->model() == completer->completionModel());
    completer->setModel(new QStringListModel(completer));
    QVERIFY(dirModel == 0); // must have been deleted

    completer->setModel(0);
    completer->setWidget(0);
}

void tst_QCompleter::modelDeletion()
{
    delete completer;
    completer = new CsvCompleter;
    QStringList list;
    list << "item1" << "item2" << "item3";
    QStringListModel *listModel = new QStringListModel(list);
    completer->setCompletionPrefix("i");
    completer->setModel(listModel);
    QVERIFY(completer->completionCount() == 3);
    QListView *view = new QListView;
    view->setModel(completer->completionModel());
    delete listModel;
    view->show();
    qApp->processEvents();
    delete view;
    QVERIFY(completer->completionCount() == 0);
    QVERIFY(completer->currentRow() == -1);
}

void tst_QCompleter::multipleWidgets()
{
    QStringList list;
    list << "item1" << "item2" << "item2";
    QCompleter completer(list);
    completer.setCompletionMode(QCompleter::InlineCompletion);

    QWidget window;
    window.show();

    QFocusEvent focusIn(QEvent::FocusIn);
    QFocusEvent focusOut(QEvent::FocusOut);

    QComboBox *comboBox = new QComboBox(&window);
    comboBox->setEditable(true);
    comboBox->setCompleter(&completer);
    comboBox->setFocus();
    comboBox->show();
    qApp->processEvents(); // focus in
    comboBox->lineEdit()->setText("it");
    QCOMPARE(comboBox->currentText(), QString("it")); // should not complete with setText
    QTest::keyPress(comboBox, 'e');
    QCOMPARE(comboBox->currentText(), QString("item1"));
    comboBox->clearEditText();
    QCOMPARE(comboBox->currentText(), QString("")); // combo box text must not change!

    QLineEdit *lineEdit = new QLineEdit(&window);
    lineEdit->setCompleter(&completer);
    lineEdit->show();
    lineEdit->setFocus();
    qApp->processEvents();
    lineEdit->setText("it");
    QCOMPARE(lineEdit->text(), QString("it")); // should not completer with setText
    QCOMPARE(comboBox->currentText(), QString("")); // combo box text must not change!
    QTest::keyPress(lineEdit, 'e');
    QCOMPARE(lineEdit->text(), QString("item1"));
    QCOMPARE(comboBox->currentText(), QString("")); // combo box text must not change!
}

void tst_QCompleter::focusIn()
{
    QStringList list;
    list << "item1" << "item2" << "item2";
    QCompleter completer(list);

    QWidget window;
    window.show();

    QComboBox *comboBox = new QComboBox(&window);
    comboBox->setEditable(true);
    comboBox->setCompleter(&completer);
    comboBox->show();
    comboBox->lineEdit()->setText("it");

    QLineEdit *lineEdit = new QLineEdit(&window);
    lineEdit->setCompleter(&completer);
    lineEdit->setText("it");
    lineEdit->show();

    QLineEdit *lineEdit2 = new QLineEdit(&window); // has no completer!
    lineEdit2->show();

    comboBox->setFocus();
    qApp->processEvents();
    QVERIFY(completer.widget() == comboBox);
    lineEdit->setFocus();
    qApp->processEvents();
    QVERIFY(completer.widget() == lineEdit);
    comboBox->setFocus();
    qApp->processEvents();
    QVERIFY(completer.widget() == comboBox);
    lineEdit2->setFocus();
    qApp->processEvents();
    QVERIFY(completer.widget() == comboBox);
}

void tst_QCompleter::dynamicSortOrder()
{
    QStandardItemModel model;
    QCompleter completer(&model);
    completer.setModelSorting(QCompleter::CaseSensitivelySortedModel);
    QStandardItem *root = model.invisibleRootItem();
    for (int i = 0; i < 20; i++) {
        root->appendRow(new QStandardItem(QString("%1").arg(i)));
    }
    root->appendRow(new QStandardItem("13"));
    root->sortChildren(0, Qt::AscendingOrder);
    completer.setCompletionPrefix("1");
    QCOMPARE(completer.completionCount(), 12);
    completer.setCompletionPrefix("13");
    QCOMPARE(completer.completionCount(), 2);
    
    root->sortChildren(0, Qt::DescendingOrder);
    completer.setCompletionPrefix("13");
    QCOMPARE(completer.completionCount(), 2);
    completer.setCompletionPrefix("1");
    QCOMPARE(completer.completionCount(), 12);
}

QTEST_MAIN(tst_QCompleter)
#include "tst_qcompleter.moc"
