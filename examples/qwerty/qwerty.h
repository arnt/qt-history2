/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWERTY_H
#define QWERTY_H

#include <qwidget.h>
#include <qmenubar.h>
#include <qmultilineedit.h>
#include <qprinter.h>

class Editor : public QWidget
{
    Q_OBJECT
public:
    Editor( QWidget *parent=0, const char *name="qwerty" );
   ~Editor();

    void load( const QString& fileName, int code=-1 );

public slots:
    void newDoc();
    void load();
    bool save();
    void print();
    void addEncoding();
    void toUpper();
    void toLower();
    void font();
protected:
    void resizeEvent( QResizeEvent * );
    void closeEvent( QCloseEvent * );

private slots:
    void saveAsEncoding( int );
    void openAsEncoding( int );
    void textChanged();

private:
    bool saveAs( const QString& fileName, int code=-1 );
    void rebuildCodecList();
    QMenuBar 	   *m;
    QMultiLineEdit *e;
#ifndef QT_NO_PRINTER
    QPrinter        printer;
#endif
    QPopupMenu	   *save_as;
    QPopupMenu	   *open_as;
    bool changed;
};

#endif // QWERTY_H
