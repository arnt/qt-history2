/****************************************************************************
** $Id: //depot/qt/main/examples/notepad/notepad.h#4 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qmainwindow.h>

class QComboBox;
class QMultiLineEdit;

class Note
{
public:
    Note( const QString &theTitle, const QString &theNote=QString::null );
    ~Note();

    const QString &note() { return mNote; }
    const QString &title() { return mTitle; }

    void setNote( const QString &n ) { mNote = n; }
    void setTitle( const QString &t ) { mTitle = t; }

    bool load();
    bool save();

private:
    QString mTitle;
    QString mNote;
};

class NotePad : public QMainWindow
{
    Q_OBJECT
public:
    NotePad( QWidget *parent=0, const char *name=0 );
    ~NotePad();

private:
    void loadTitles();

private slots:
    void newNote();
    void deleteNote();
    void loadNote( int );

private:
    QStringList notes;
    Note *currentNote;
    QComboBox *noteList;
    QMultiLineEdit *edit;
};


