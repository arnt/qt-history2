#include <qiconview.h>

class IconView : public QIconView
{
    Q_OBJECT

public:
    IconView( QWidget* parent = 0, const char* name = 0 );
    ~IconView();

//    QDragObject *dragObject();

public slots:
    void slotNewItem( QDropEvent *t, const QValueList<QIconDragItem>& );
};
