/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtranslatordialog.h#1 $
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
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
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

//this file wil be renamed, and probably put in its own file
class QMessageParser
{
public:
    QMessageParser();

    virtual ~QMessageParser();
    //void parse( QTextStream *input, QString name, QString scope = "PO" );
    void parse( const QString &filename, QString scope = "PO" );
protected:
    virtual void add( const char*, const char*, const char* );
private:
    enum State { Initial, AfterKey, Error };

    void parse();

    const char *stateStr( State );
    const char *tokenStr( int token );
    void error( int token, QString );

    State state;
    QMessageLexer *lex;
    QString fileName;
    QString scope;
    QString key;
    QString trans;

};




#endif // QTRANSLATORDIALOG_H
