#ifndef HELPVIEW_H
#define HELPVIEW_H

#include <qtextbrowser.h>

class HelpView : public QTextBrowser
{
    Q_OBJECT
    
public:
    HelpView( QWidget *parent, const QString &dd );
    
public slots:
    void showLink( const QString &link );

private:
    QString docDir;
    
};


#endif
