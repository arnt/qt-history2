#include <qwidget.h>
#include <qcstring.h>
#include <qcolor.h>

class DistributionWidget : public QWidget
{
    Q_OBJECT
public:
    DistributionWidget( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    ~DistributionWidget();

    enum {
	BlockStatus_Free,
	BlockStatus_Other,
	BlockStatus_Used
    };
private:
    int totalBlocks;
    QByteArray blockStatus;
    QColor backColor;
    QColor usedColor;
    QColor otherColor;
public slots:
    void setTotalBlocks( int );
    void setBlockStatus( int, int );

public:
    virtual QSize sizeHint() const;
protected:
    virtual void paintEvent( QPaintEvent* pEv );
};
