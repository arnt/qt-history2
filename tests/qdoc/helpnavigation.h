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
class QProgressBar;
class QComboBox;
class QPushButton;

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

    HelpNavigation( QWidget *parent, const QString &dd );
    void setViewMode( ViewMode m );
    ViewMode viewMode() const;
    QString titleOfLink( const QString &link );

    bool eventFilter( QObject *, QEvent * );

    void loadIndexFile( QProgressBar *bar );
    void setupContentsView( QProgressBar *bar );

    void addBookmark( const QString &title, const QString &link );
    void removeBookmark();
    void saveBookmarks();
    void loadBookmarks();

signals:
    void showLink( const QString &s, const QString& t );
    void moveFocusToBrowser();
    void preparePorgress( int );
    void incProcess();
    void finishProgress();
    void tabChanged();
    
private slots:
    void searchInIndexLine( const QString &s );
    void showTopic( QListBoxItem * );
    void setIndexTopic( QListBoxItem * );
    void showContents( QListViewItem * );
    void startSearch();

private:
    QListViewItem *doSearch( const QString &fn, const QStringList &query, QListViewItem *after );

    QTabWidget *tabWidget;
    QLineEdit *indexEdit;
    QListBox *indexList;
    QMap<QString, QString> titleMap;
    QListView *contentsView, *bookmarkList, *searchList;
    QWidget *contentsTab, *indexTab, *bookmarkTab, *searchTab;
    QString docDir;
    QComboBox *searchCombo;
    QPushButton *searchButton;
    bool inSearch;

};

#endif
