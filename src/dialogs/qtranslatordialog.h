/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtranslatordialog.h#4 $
**
** Definition of QTranslatorDialog class
**
** Created : 990115
**
** Copyright (C) 1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTRANSLATORDIALOG_H
#define QTRANSLATORDIALOG_H

#ifndef QT_H
#include "qdialog.h"
class QListView;
class QListViewItem;
#endif // QT_H

class QTranslatorEdit;
class QMessageLexer;

class Q_EXPORT QTranslatorDialog : public QDialog
{
    Q_OBJECT
public:
    QTranslatorDialog( QWidget * parent=0, const char* name = 0 );
    ~QTranslatorDialog();
public slots:
    void add( const char*, const char *);
    void addTranslation( const char*, const char *, const char *);
    void save();
protected:
    void hide();
    void show();
private slots:
    void currentItemSet( QListViewItem * );
    void updateEd();
    void textChanged();
private:
    QListView *lv;
    QTranslatorEdit *ed;
    QListViewItem *currentItem;
    bool showing;
    bool edited;
};


class Q_EXPORT QAppTranslator : public QTranslatorDialog
{
    Q_OBJECT
public:
    QAppTranslator( QWidget * parent=0, const char* name = 0 );
    ~QAppTranslator();
};

#endif // QTRANSLATORDIALOG_H
