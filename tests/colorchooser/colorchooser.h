#include <qdialog.h>


class QWellArray;
class QColorDialogPrivate;

class ColorChooser : public QDialog {
    Q_OBJECT
public:
    ColorChooser(QWidget* parent=0, const char* name=0);
private:
     QColorDialogPrivate *d;
};
