

#include <qdialog.h>

class QLabel;
class QSlider;

class QVFbRateDialog : public QDialog
{
    Q_OBJECT
public:
    QVFbRateDialog( int value, QWidget *parent=0, const char *name=0,
		    bool modal=FALSE );

signals:
    void updateRate( int r );

protected slots:
    void rateChanged( int r );
    void cancel();

private:
    QLabel *rateLabel;
    QSlider *rateSlider;
    int oldRate;
};


