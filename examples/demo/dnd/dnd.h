#include "dndbase.h"

class DnDDemo : public DnDDemoBase
{
    Q_OBJECT

public:
    DnDDemo( QWidget* parent = 0, const char* name = 0 );
    ~DnDDemo();

protected:
    QPixmap loadPixmap( const QString& name );
};
