#include <qdialog.h>
#include <qlistview.h>

class QWidgetStack;
class PreferenceItem;

class Preferences : public QDialog {
    Q_OBJECT
public:
    Preferences(QWidget* parent=0, const char* name=0, int f=0);
private:
    void add( PreferenceItem*, QWidget*);
    QWidgetStack* categories;
};

class PreferenceItem : public QObject, public QListViewItem {
    Q_OBJECT
public:
    PreferenceItem( QListView* view, const char* label );
    PreferenceItem( QListViewItem* group, const char* label );
    int id() const { return i; }

signals:
    void activated(int id);

protected:
    void activate();

private:
    int i;
    static int next_id;
};
