#include <qtextview.h>
#include <qlistview.h>

class ItemTextView : public QTextView
{
    Q_OBJECT

public:
    ItemTextView( QWidget * parent=0, const char * name=0 );
    ~ItemTextView();

    QSize sizeHint() const;
    void setIndex( int i );

public slots:
    void change( QListViewItem* );

private:
    int index;
};
