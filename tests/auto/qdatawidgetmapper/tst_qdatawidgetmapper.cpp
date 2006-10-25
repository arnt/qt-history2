#include <QtGui/QtGui>
#include <QtTest/QtTest>

class tst_QDataWidgetMapper: public QObject
{
    Q_OBJECT
private slots:
    void setModel();
    void navigate();
    void addMapping();
    void currentIndexChanged();
    void changingValues();
    void setData();
    void mappedWidgetAt();

    void comboBox();
};

static QStandardItemModel *testModel(QObject *parent = 0)
{
    QStandardItemModel *model = new QStandardItemModel(10, 10, parent);

    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col)
            model->setData(model->index(row, col), QString("item %1 %2").arg(row).arg(col));
    }

    return model;
}

void tst_QDataWidgetMapper::setModel()
{
    QDataWidgetMapper mapper;

    QCOMPARE(mapper.model(), (QAbstractItemModel *)0);

    { // let the model go out of scope first
        QStandardItemModel model;
        mapper.setModel(&model);
        QCOMPARE(mapper.model(), &model);
    }

    QCOMPARE(mapper.model(), (QAbstractItemModel *)0);

    { // let the mapper go out of scope first
        QStandardItemModel model2;
        QDataWidgetMapper mapper2;
        mapper2.setModel(&model2);
    }
}

void tst_QDataWidgetMapper::navigate()
{
    QDataWidgetMapper mapper;
    QAbstractItemModel *model = testModel(&mapper);
    mapper.setModel(model);

    QLineEdit edit1;
    QLineEdit edit2;
    QLineEdit edit3;

    mapper.addMapping(&edit1, 0);
    mapper.addMapping(&edit2, 1);
    mapper.addMapping(&edit3, 2);

    mapper.toFirst();
    QCOMPARE(edit1.text(), QString("item 0 0"));
    QCOMPARE(edit2.text(), QString("item 0 1"));
    QCOMPARE(edit3.text(), QString("item 0 2"));

    mapper.toPrevious(); // should do nothing
    QCOMPARE(edit1.text(), QString("item 0 0"));
    QCOMPARE(edit2.text(), QString("item 0 1"));
    QCOMPARE(edit3.text(), QString("item 0 2"));

    mapper.toNext();
    QCOMPARE(edit1.text(), QString("item 1 0"));
    QCOMPARE(edit2.text(), QString("item 1 1"));
    QCOMPARE(edit3.text(), QString("item 1 2"));

    mapper.toLast();
    QCOMPARE(edit1.text(), QString("item 9 0"));
    QCOMPARE(edit2.text(), QString("item 9 1"));
    QCOMPARE(edit3.text(), QString("item 9 2"));

    mapper.toNext(); // should do nothing
    QCOMPARE(edit1.text(), QString("item 9 0"));
    QCOMPARE(edit2.text(), QString("item 9 1"));
    QCOMPARE(edit3.text(), QString("item 9 2"));

    mapper.setCurrentIndex(4);
    QCOMPARE(edit1.text(), QString("item 4 0"));
    QCOMPARE(edit2.text(), QString("item 4 1"));
    QCOMPARE(edit3.text(), QString("item 4 2"));

    mapper.setCurrentIndex(-1); // should do nothing
    QCOMPARE(edit1.text(), QString("item 4 0"));
    QCOMPARE(edit2.text(), QString("item 4 1"));
    QCOMPARE(edit3.text(), QString("item 4 2"));

    mapper.setCurrentIndex(10); // should do nothing
    QCOMPARE(edit1.text(), QString("item 4 0"));
    QCOMPARE(edit2.text(), QString("item 4 1"));
    QCOMPARE(edit3.text(), QString("item 4 2"));

    mapper.setCurrentModelIndex(QModelIndex()); // should do nothing
    QCOMPARE(edit1.text(), QString("item 4 0"));
    QCOMPARE(edit2.text(), QString("item 4 1"));
    QCOMPARE(edit3.text(), QString("item 4 2"));

    mapper.setCurrentModelIndex(model->index(6, 0));
    QCOMPARE(edit1.text(), QString("item 6 0"));
    QCOMPARE(edit2.text(), QString("item 6 1"));
    QCOMPARE(edit3.text(), QString("item 6 2"));

    /* now try vertical navigation */

    mapper.setOrientation(Qt::Vertical);

    mapper.addMapping(&edit1, 0);
    mapper.addMapping(&edit2, 1);
    mapper.addMapping(&edit3, 2);

    mapper.toFirst();
    QCOMPARE(edit1.text(), QString("item 0 0"));
    QCOMPARE(edit2.text(), QString("item 1 0"));
    QCOMPARE(edit3.text(), QString("item 2 0"));

    mapper.toPrevious(); // should do nothing
    QCOMPARE(edit1.text(), QString("item 0 0"));
    QCOMPARE(edit2.text(), QString("item 1 0"));
    QCOMPARE(edit3.text(), QString("item 2 0"));

    mapper.toNext();
    QCOMPARE(edit1.text(), QString("item 0 1"));
    QCOMPARE(edit2.text(), QString("item 1 1"));
    QCOMPARE(edit3.text(), QString("item 2 1"));

    mapper.toLast();
    QCOMPARE(edit1.text(), QString("item 0 9"));
    QCOMPARE(edit2.text(), QString("item 1 9"));
    QCOMPARE(edit3.text(), QString("item 2 9"));

    mapper.toNext(); // should do nothing
    QCOMPARE(edit1.text(), QString("item 0 9"));
    QCOMPARE(edit2.text(), QString("item 1 9"));
    QCOMPARE(edit3.text(), QString("item 2 9"));

    mapper.setCurrentIndex(4);
    QCOMPARE(edit1.text(), QString("item 0 4"));
    QCOMPARE(edit2.text(), QString("item 1 4"));
    QCOMPARE(edit3.text(), QString("item 2 4"));

    mapper.setCurrentIndex(-1); // should do nothing
    QCOMPARE(edit1.text(), QString("item 0 4"));
    QCOMPARE(edit2.text(), QString("item 1 4"));
    QCOMPARE(edit3.text(), QString("item 2 4"));

    mapper.setCurrentIndex(10); // should do nothing
    QCOMPARE(edit1.text(), QString("item 0 4"));
    QCOMPARE(edit2.text(), QString("item 1 4"));
    QCOMPARE(edit3.text(), QString("item 2 4"));

    mapper.setCurrentModelIndex(QModelIndex()); // should do nothing
    QCOMPARE(edit1.text(), QString("item 0 4"));
    QCOMPARE(edit2.text(), QString("item 1 4"));
    QCOMPARE(edit3.text(), QString("item 2 4"));

    mapper.setCurrentModelIndex(model->index(0, 6));
    QCOMPARE(edit1.text(), QString("item 0 6"));
    QCOMPARE(edit2.text(), QString("item 1 6"));
    QCOMPARE(edit3.text(), QString("item 2 6"));
}

void tst_QDataWidgetMapper::addMapping()
{
    QDataWidgetMapper mapper;
    QAbstractItemModel *model = testModel(&mapper);
    mapper.setModel(model);

    QLineEdit edit1;
    mapper.addMapping(&edit1, 0);
    mapper.toFirst();
    QCOMPARE(edit1.text(), QString("item 0 0"));

    mapper.addMapping(&edit1, 1);
    mapper.toFirst();
    QCOMPARE(edit1.text(), QString("item 0 1"));

    QCOMPARE(mapper.mappedSection(&edit1), 1);

    edit1.clear();
    mapper.removeMapping(&edit1);
    mapper.toFirst();
    QCOMPARE(edit1.text(), QString());

    {
        QLineEdit edit2;
        mapper.addMapping(&edit2, 2);
        mapper.toFirst();
        QCOMPARE(edit2.text(), QString("item 0 2"));
    } // let the edit go out of scope

    QCOMPARE(mapper.mappedWidgetAt(2), (QWidget *)0);
    mapper.toLast();
}

void tst_QDataWidgetMapper::currentIndexChanged()
{
    QDataWidgetMapper mapper;
    QAbstractItemModel *model = testModel(&mapper);
    mapper.setModel(model);

    QSignalSpy spy(&mapper, SIGNAL(currentIndexChanged(int)));

    mapper.toFirst();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 0);

    mapper.toNext();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 1);

    mapper.setCurrentIndex(7);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 7);

    mapper.setCurrentIndex(-1);
    QCOMPARE(spy.count(), 0);

    mapper.setCurrentIndex(42);
    QCOMPARE(spy.count(), 0);
}

void tst_QDataWidgetMapper::changingValues()
{
    QDataWidgetMapper mapper;
    QAbstractItemModel *model = testModel(&mapper);
    mapper.setModel(model);

    QLineEdit edit1;
    mapper.addMapping(&edit1, 0);
    mapper.toFirst();
    QCOMPARE(edit1.text(), QString("item 0 0"));

    model->setData(model->index(0, 0), QString("changed"));
    QCOMPARE(edit1.text(), QString("changed"));
}

void tst_QDataWidgetMapper::setData()
{
    QDataWidgetMapper mapper;
    QAbstractItemModel *model = testModel(&mapper);
    mapper.setModel(model);

    QLineEdit edit1;
    QLineEdit edit2;

    mapper.addMapping(&edit1, 0);
    mapper.addMapping(&edit2, 1);
    mapper.toFirst();
    QCOMPARE(edit1.text(), QString("item 0 0"));
    QCOMPARE(edit2.text(), QString("item 0 1"));

    edit1.setText("new text");

    mapper.submit();
    QCOMPARE(model->data(model->index(0, 0)).toString(), QString("new text"));
}

// a combo box is evil since it is either read/only or read/write
void tst_QDataWidgetMapper::comboBox()
{
    QDataWidgetMapper mapper;
    QAbstractItemModel *model = testModel(&mapper);
    mapper.setModel(model);
    mapper.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

    QComboBox readOnlyBox;
    readOnlyBox.setEditable(false);
    readOnlyBox.addItem("item -1 -1");
    readOnlyBox.addItem("item 0 0");
    readOnlyBox.addItem("item 0 x");

    QComboBox readWriteBox;
    readWriteBox.setEditable(true);

    // populat the combo boxes with data
    mapper.addMapping(&readOnlyBox, 0);
    mapper.addMapping(&readWriteBox, 1);
    mapper.toFirst();

    QEXPECT_FAIL("", "See task 125493", Abort);
    QCOMPARE(readOnlyBox.currentText(), QString("item 0 0"));
    QCOMPARE(readWriteBox.currentText(), QString("item 0 1"));

    // set some new values on the boxes
    readOnlyBox.setCurrentIndex(1);
    readWriteBox.setEditText("item 0 y");

    mapper.submit();

    // make sure the new values are in the model
    QCOMPARE(model->data(model->index(0, 0)).toString(), QString("1"));
    QCOMPARE(model->data(model->index(0, 1)).toString(), QString("item 0 y"));
}

void tst_QDataWidgetMapper::mappedWidgetAt()
{
    QDataWidgetMapper mapper;
    QAbstractItemModel *model = testModel(&mapper);
    mapper.setModel(model);

    QLineEdit lineEdit1;
    QLineEdit lineEdit2;

    mapper.addMapping(&lineEdit1, 1);
    mapper.addMapping(&lineEdit2, 2);

    QCOMPARE(mapper.mappedWidgetAt(1), &lineEdit1);
    QCOMPARE(mapper.mappedWidgetAt(2), &lineEdit2);

    mapper.addMapping(&lineEdit2, 4242);

    QCOMPARE(mapper.mappedWidgetAt(2), (QWidget*)0);
    QCOMPARE(mapper.mappedWidgetAt(4242), &lineEdit2);
}

QTEST_MAIN(tst_QDataWidgetMapper)
#include "tst_qdatawidgetmapper.moc"
