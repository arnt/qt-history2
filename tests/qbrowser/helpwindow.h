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

#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qtextbrowser.h>
#include <qmainwindow.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdir.h>
#include <qurloperator.h>

class QComboBox;
class QPopupMenu;
class QUrlOperator;

class HelpWindow : public QMainWindow
{
    Q_OBJECT
public:
    HelpWindow( const QString& home_,  const QString& path, QWidget* parent = 0, const char *name=0 );
    ~HelpWindow();

private slots:
    void setBackwardAvailable( bool );
    void setForwardAvailable( bool );

    void textChanged();
    void about();
    void aboutQt();
    void openFile();
    void newWindow();

    void pathSelected( const QString & );
    void histChosen( int );
    void bookmChosen( int );
    void addBookmark();

    void newData( const QByteArray &data, QNetworkOperation *op );

private:
    bool eventFilter( QObject * o, QEvent * e );
    void readHistory();
    void readBookmarks();

    QTextBrowser* browser;
    QComboBox *pathCombo;
    int backwardId, forwardId;
    QString selectedURL;
    QDir path;
    QStringList fileList, history, bookmarks;
    QMap<int, QString> mHistory, mBookmarks;
    QPopupMenu *hist, *bookm;
    QUrlOperator url;
    QCString pageData;

};





#endif

