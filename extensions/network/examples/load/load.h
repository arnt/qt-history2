#include <qwidget.h>

class QNetworkOperation;
class LoadData;

class Load : public QWidget {
    Q_OBJECT
public:
    Load(const QString& url, QWidget* parent=0, const char* name=0, WFlags f=0);
    ~Load();

protected:
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
private slots:
    void data(const QByteArray&);
private:
    LoadData* d;
};
