#include <QtGui>
#include <QtDebug>

class Coordinate : public QObject
{
public:
    int myX, myY;

    int x() const { return myX; };
    int y() const { return myY; };
};

QDebug operator<<(QDebug dbg, const Coordinate &c)
{
    dbg.nospace() << "(" << c.x() << ", " << c.y() << ")";
    
    return dbg.space();
}

int main(int argv, char **args)
{
    Coordinate nate;
    nate.myX = 10;
    nate.myY = 44;

    qDebug() << nate;
}
