#include <qdialog.h>


class QWellArray;
class QColorDialogPrivate;

class QColorDialog : public QDialog {
    Q_OBJECT
public:
    QColorDialog( QWidget* parent=0, const char* name=0, bool modal=FALSE );
    QRgb selectedColor() const;
    
    static QRgb getColor(QWidget *parent=0, const char* name=0 );
private:
     QColorDialogPrivate *d;
};
