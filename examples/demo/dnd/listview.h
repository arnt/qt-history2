#include <qlistview.h>

class ListView : public QListView
{
    Q_OBJECT

public:
    ListView( QWidget* parent = 0, const char* name = 0 );
    ~ListView();
};
