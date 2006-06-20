/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qfiledialog.h>
#include <qabstractitemdelegate.h>
#include <qdirmodel.h>

//TESTED_CLASS=
//TESTED_FILES=qfiledialog.h

class tst_QFiledialog : public QObject
{
Q_OBJECT

public:
    tst_QFiledialog();
    virtual ~tst_QFiledialog();

private slots:
    void getSetCheck();
};

tst_QFiledialog::tst_QFiledialog()
{
}

tst_QFiledialog::~tst_QFiledialog()
{
}

class MyAbstractItemDelegate : public QAbstractItemDelegate
{
public:
    MyAbstractItemDelegate() : QAbstractItemDelegate() {};
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const {}
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const { return QSize(); }
};

// Testing get/set functions
void tst_QFiledialog::getSetCheck()
{
    QFileDialog obj1;
    // ViewMode QFileDialog::viewMode()
    // void QFileDialog::setViewMode(ViewMode)
    obj1.setViewMode(QFileDialog::ViewMode(QFileDialog::Detail));
    QCOMPARE(QFileDialog::ViewMode(QFileDialog::Detail), obj1.viewMode());
    obj1.setViewMode(QFileDialog::ViewMode(QFileDialog::List));
    QCOMPARE(QFileDialog::ViewMode(QFileDialog::List), obj1.viewMode());

    // FileMode QFileDialog::fileMode()
    // void QFileDialog::setFileMode(FileMode)
    obj1.setFileMode(QFileDialog::FileMode(QFileDialog::AnyFile));
    QCOMPARE(QFileDialog::FileMode(QFileDialog::AnyFile), obj1.fileMode());
    obj1.setFileMode(QFileDialog::FileMode(QFileDialog::ExistingFile));
    QCOMPARE(QFileDialog::FileMode(QFileDialog::ExistingFile), obj1.fileMode());
    obj1.setFileMode(QFileDialog::FileMode(QFileDialog::Directory));
    QCOMPARE(QFileDialog::FileMode(QFileDialog::Directory), obj1.fileMode());
    obj1.setFileMode(QFileDialog::FileMode(QFileDialog::ExistingFiles));
    QCOMPARE(QFileDialog::FileMode(QFileDialog::ExistingFiles), obj1.fileMode());
    obj1.setFileMode(QFileDialog::FileMode(QFileDialog::DirectoryOnly));
    QCOMPARE(QFileDialog::FileMode(QFileDialog::DirectoryOnly), obj1.fileMode());

    // AcceptMode QFileDialog::acceptMode()
    // void QFileDialog::setAcceptMode(AcceptMode)
    obj1.setAcceptMode(QFileDialog::AcceptMode(QFileDialog::AcceptOpen));
    QCOMPARE(QFileDialog::AcceptMode(QFileDialog::AcceptOpen), obj1.acceptMode());
    obj1.setAcceptMode(QFileDialog::AcceptMode(QFileDialog::AcceptSave));
    QCOMPARE(QFileDialog::AcceptMode(QFileDialog::AcceptSave), obj1.acceptMode());

    // bool QFileDialog::resolveSymlinks()
    // void QFileDialog::setResolveSymlinks(bool)
    obj1.setResolveSymlinks(false);
    QCOMPARE(false, obj1.resolveSymlinks());
    obj1.setResolveSymlinks(true);
    QCOMPARE(true, obj1.resolveSymlinks());

    // bool QFileDialog::confirmOverwrite()
    // void QFileDialog::setConfirmOverwrite(bool)
    obj1.setConfirmOverwrite(false);
    QCOMPARE(false, obj1.confirmOverwrite());
    obj1.setConfirmOverwrite(true);
    QCOMPARE(true, obj1.confirmOverwrite());

    // QAbstractItemDelegate * QFileDialog::itemDelegate()
    // void QFileDialog::setItemDelegate(QAbstractItemDelegate *)
    MyAbstractItemDelegate *var6 = new MyAbstractItemDelegate;
    obj1.setItemDelegate(var6);
    QCOMPARE(var6, obj1.itemDelegate());
#if QT_VERSION >= 0x040200
    // Itemviews in Qt < 4.2 have asserts for this. Qt >= 4.2 should handle this gracefully
    obj1.setItemDelegate((QAbstractItemDelegate *)0);
    QCOMPARE((QAbstractItemDelegate *)0, obj1.itemDelegate());
#endif
    delete var6;

    // QFileIconProvider * QFileDialog::iconProvider()
    // void QFileDialog::setIconProvider(QFileIconProvider *)
    QFileIconProvider *var7 = new QFileIconProvider;
    obj1.setIconProvider(var7);
    QCOMPARE(var7, obj1.iconProvider());
    obj1.setIconProvider((QFileIconProvider *)0);
    QCOMPARE((QFileIconProvider *)0, obj1.iconProvider());
    delete var7;
}

QTEST_MAIN(tst_QFiledialog)
#include "tst_qfiledialog.moc"
