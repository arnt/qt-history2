/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qdeveloper.h#4 $
**
** Definition of QDeveloper class
**
** Created : 980830
**
** Copyright (C) 1998-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QBUILDER_H
#define QBUILDER_H

#ifndef QT_H
#include "qmainwindow.h"
#include "qlistview.h"
#endif // QT_H

class QDeveloperPrivate;

class QDeveloperObjectItem : public QObject, public QListViewItem {
    Q_OBJECT
    QObject* object;
    QDeveloperPrivate* d;

public:
    QDeveloperObjectItem( QListView * parent, QObject* o, QDeveloperPrivate* );
    QDeveloperObjectItem( QListViewItem * parent, QObject* o, QDeveloperPrivate* );

    void setup();

    QObject* at() { return object; }
    void fillExistingTree();

protected:
    bool eventFilter(QObject*, QEvent*);

private slots:
    void objectDestroyed();
};


class Q_EXPORT QDeveloper : public QMainWindow
{
    Q_OBJECT
public:
    QDeveloper();
   ~QDeveloper();

private:
    QDeveloperPrivate* d;
    friend QApplication;
    void addTopLevelWidget(QWidget*);

private:
    void updateDetails( QObject* object, QMetaObject* cls=0 );

private slots:
    void selectObject( QListViewItem* );
    void selectClass( QListViewItem* );
    void recordUnknownTranslation( const char *, const char * );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDeveloper( const QDeveloper & );
    QDeveloper &operator=( const QDeveloper & );
#endif
};

#endif // QFILEDIALOG_H
