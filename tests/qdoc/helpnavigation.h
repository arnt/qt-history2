#ifndef HELPNAVIGATION_H
#define HELPNAVIGATION_H

#include <qwidget.h>
#include <qstringlist.h>
#include <qlistbox.h>
#include <qmap.h>
#include <qlistview.h>

class QTabWidget;
class QListBox;
class QLineEdit;
class QListView;

class HelpNavigationListItem : public QListBoxText
{
public:
    HelpNavigationListItem( QListBox *ls, const QString &txt );

    void addLink( const QString &link );
    QStringList links() const { return linkList; }

private:
    QStringList linkList;

};

class HelpNavigationContentsItem : public QListViewItem
{
public:
    HelpNavigationContentsItem( QListView *v, QListViewItem *after );
    HelpNavigationContentsItem( QListViewItem *v, QListViewItem *after );

    void setLink( const QString &lnk );
    QString link() const;

private:
    QString theLink;

};

class HelpNavigation : public QWidget
{
    Q_OBJECT

public:
    enum ViewMode {
	Contents,
	Index,
	Search,
	Bookmarks
    };

    HelpNavigation( QWidget *parent, const QString &indexFile,
		    const QString &titleFile, const char *name = 0 );

    void setViewMode( ViewMode m );
    QString titleOfLink( const QString &link );

    bool eventFilter( QObject *, QEvent * );

signals:
    void showLink( const QString &s, const QString& t );
    void moveFocusToBrowser();
    
private slots:
    void searchInIndexLine( const QString &s );
    void showTopic( QListBoxItem * );
    void setIndexTopic( QListBoxItem * );
    void showContents( QListViewItem * );
    
private:
    void loadIndexFile( const QString &indexFile, const QString &titleFile );
    void setupContentsView( const QString &titleFile );

    QTabWidget *tabWidget;
    QLineEdit *indexEdit;
    QListBox *indexList, *bookmarkList;
    QMap<QString, QString> titleMap;
    QListView *contentsView;
    QWidget *contentsTab, *indexTab, *bookmarkTab;

};

#endif
