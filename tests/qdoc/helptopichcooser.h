#ifndef HELPTOPICCHOOSER_H
#define HELPTOPICCHOOSER_H

#include <qdialog.h>

class QListBox;

class HelpTopicChooser : public QDialog
{
    Q_OBJECT
    
public:
    HelpTopicChooser( QWidget *parent, const QStringList &lnkNames,
		      const QStringList &lnks, const QString &title );
    
    QString link() const;
    
    static QString getLink( QWidget *parent, const QStringList &lnkNames,
			    const QStringList &lnks, const QString &title );
    
private:
    QString theLink;
    QListBox *linkList;
    QStringList links, linkNames;
    
};
    
#endif
