/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of pen input character set maintenance.
** Probably a developer-only tool.
**
** Created : 20000414
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qwidget.h>
#include <qdialog.h>
#include "qimpenchar.h"

class QComboBox;
class QListBox;
class QLineEdit;
class QIMPenWidget;

class QIMPenInputCharDlg : public QDialog
{
    Q_OBJECT
public:
    QIMPenInputCharDlg( QWidget *parent = 0, const char *name = 0,
                    bool modal = FALSE, int WFlags = 0 );

    unsigned int unicode() const { return uni; }

protected:
    void addSpecial( QComboBox * );

protected slots:
    void setSpecial( int sp );
    void setCharacter( const QString &string );

protected:
    uint uni;
};

class QIMPenCreateCharSet : public QWidget
{
    Q_OBJECT
public:
    QIMPenCreateCharSet();
    ~QIMPenCreateCharSet();

protected:
    void fillList();

protected slots:
    void load();
    void save();
    void up();
    void down();
    void add();
    void remove();
    void clear();
    void newStroke( QIMPenStroke *pc );
    void charSelected( int idx );

protected:
    QString filename;
    QLineEdit *title;
    QLineEdit *desc;
    QListBox *charList;
    QIMPenWidget *pw;
    QIMPenChar *penChar;
    QIMPenChar *editChar;
    QIMPenCharSet *charSet;
};

