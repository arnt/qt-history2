#ifndef HELPVIEW_H
#define HELPVIEW_H

#include <qtextbrowser.h>

class HelpView : public QTextBrowser
{
    Q_OBJECT

public:
    HelpView( QWidget *parent, const QString &dd );

    void setSource( const QString &name );
    
public slots:
    void showLink( const QString &link,  const QString &title );

signals:
    void newSource( const QString & );
    
private:
    QString docDir;

};


#endif
