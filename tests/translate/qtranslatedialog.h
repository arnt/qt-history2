/****************************************************************************
** $Id: //depot/qt/main/tests/translate/qtranslatedialog.h#1 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1999 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QTRANSLATEDIALOG_H
#define QTRANSLATEDIALOG_H

#include "qdialog.h"
class QListView;
class QListViewItem;
class QLineEdit;

class Q_EXPORT QTranslateDialog : public QDialog
{
    Q_OBJECT
public:
    QTranslateDialog( QWidget * parent=0, const char* name = 0 );
    ~QTranslateDialog();
public slots:    
    void add( const char*, const char *);
    void addTranslation( const char*, const char *, const char *);
    void save();
private slots:
    void currentItemSet( QListViewItem * );
private:
    QListView *lv;
    QLineEdit *ed;
    QListViewItem *currentItem;
};

#endif
