#include <qmainwindow.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qtextbrowser.h>

class ChoiceItem;
class QListViewItem;
class QListView;
class QLabel;

class Info : public QTextBrowser {
    Q_OBJECT
public:
    Info( QWidget* Q_PARENT, const char* Q_NAME );

signals:
    void idClicked(const QString& name);

public slots:
    void setSource(const QString& name);
};

class Main : public QMainWindow {
    Q_OBJECT
public:
    Main();
    void loadFeatures(const QString& filename);
    void loadConfig(const QString& filename);

private:
    void createItem(const QString& ch);
    QMap<QString,QString> label;
    QMap<QString,QString> documentation;
    QMap<QString,QStringList> links;
    QMap<QString,QStringList> dependencies;
    QMap<QString,QStringList> rdependencies;
    QMap<QString,QString> section;
    QMap<QString,ChoiceItem*> item;
    QMap<QString,QListViewItem*> sectionitem;
    QStringList choices;
    QListView* lv;
    QTextBrowser* info;

private slots:
    void updateAvailability(QListViewItem* i);
    void showInfo(QListViewItem* i);
    void selectId(const QString&);
    void open();
    void save();
    void testAll();
};
