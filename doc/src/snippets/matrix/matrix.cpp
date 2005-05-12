#include <QtGui/QtGui>
#include <cmath>

class MyWidget : public QWidget
{
    void paintEvent(QPaintEvent *);
};

// PAINT
void MyWidget::paintEvent(QPaintEvent *)
{
    QPainter p;                   // our painter
    QMatrix m;                    // our transformation matrix
    m.rotate(22.5);               // rotated coordinate system
    p.begin(this);                // start painting
    p.setMatrix(m);               // use rotated coordinate system
    p.drawText(30,20, "detator"); // draw rotated text at 30,20
    p.end();                      // painting done
}


int main()
{
    double pi = 3.14;
    {
        // COMBINE
        QMatrix m;            // identity matrix
        m.translate(10, -20); // first translate (10,-20)
        m.rotate(25);         // then rotate 25 degrees
        m.scale(1.2, 0.7);    // finally scale it
    }
    {
        // OPERATIONS
        double a    = pi/180 * 25;         // convert 25 to radians
        double sina = sin(a);
        double cosa = cos(a);
        QMatrix m1(1, 0, 0, 1, 10, -20);   // translation matrix
        QMatrix m2(cosa, sina,             // rotation matrix
                    -sina, cosa, 0, 0);
        QMatrix m3(1.2, 0, 0, 0.7, 0, 0);  // scaling matrix
        QMatrix m;
        m = m3 * m2 * m1;                  // combine all transformations
    }
    return 0;
}
