/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   finddialog.h
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <qdialog.h>

class QCheckBox;
class QLineEdit;

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    enum { SourceText = 0x1, Translations = 0x2, Comments = 0x4 };

    FindDialog( QWidget *parent = 0, const char *name = 0, bool modal = FALSE );

signals:
    void findNext( const QString& text, int where, bool matchCase );

private slots:
    void emitFindNext();

private:
    QLineEdit *led;
    QCheckBox *sourceText;
    QCheckBox *translations;
    QCheckBox *comments;
    QCheckBox *matchCase;
};

#endif
