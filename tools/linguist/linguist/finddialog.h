/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Linguist.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

    FindDialog( bool replace, QWidget *parent = 0, const char *name = 0, bool modal = FALSE );

signals:
    void findNext( const QString& text, int where, bool matchCase );
    void replace( const QString& before, const QString& after, bool matchCase, bool all );

private slots:
    void emitFindNext();
    void emitReplace();
    void emitReplaceAll();

private:
    QLineEdit *led;
    QLineEdit *red;
    QCheckBox *sourceText;
    QCheckBox *translations;
    QCheckBox *comments;
    QCheckBox *matchCase;
};

#endif
