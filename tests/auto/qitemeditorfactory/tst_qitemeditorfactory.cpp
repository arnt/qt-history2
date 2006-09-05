#include <QtGui/QtGui>
#include <QtTest/QtTest>

class tst_QItemEditorFactory: public QObject
{
    Q_OBJECT
private slots:
    void createEditor();
    void createCustomEditor();
};

void tst_QItemEditorFactory::createEditor()
{
    const QItemEditorFactory *factory = QItemEditorFactory::defaultFactory();

    QWidget parent;

    QWidget *w = factory->createEditor(QVariant::String, &parent);
    QCOMPARE(w->metaObject()->className(), "QExpandingLineEdit");
}

void tst_QItemEditorFactory::createCustomEditor()
{
#if QT_VERSION < 0x040200
    QSKIP("Needs Qt >= 4.2", SkipAll);
#else
    QItemEditorFactory editorFactory;

    QItemEditorCreatorBase *creator = new QStandardItemEditorCreator<QDoubleSpinBox>();
    editorFactory.registerEditor(QVariant::Rect, creator);

    QWidget parent;

    QWidget *w = editorFactory.createEditor(QVariant::Rect, &parent);
    QCOMPARE(w->metaObject()->className(), "QDoubleSpinBox");
    QCOMPARE(w->metaObject()->userProperty().type(), QVariant::Double);

    delete creator;
#endif
}

QTEST_MAIN(tst_QItemEditorFactory)
#include "tst_qitemeditorfactory.moc"

