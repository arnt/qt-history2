#include <qdialog.h>


class QWellArray;
class QColorShower;

class ColorChooser : public QDialog {
    Q_OBJECT
public:
    ColorChooser(QWidget* parent=0, const char* name=0, bool modal=FALSE);
public slots:
    void addCustom();
private:
    QWellArray *custom;
    QWellArray *standard;
    QColorShower *cs;
    int nCust;
};
