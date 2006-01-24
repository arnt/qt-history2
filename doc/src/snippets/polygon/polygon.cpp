#include <QtGui>

int main()
{
    {
        // STREAM
        QPolygon polygon;
        polygon << QPoint(10, 20) << QPoint(20, 30);
    }

    {
        // STREAMF
        QPolygonF polygon;
        polygon << QPointF(10.4, 20.5) << QPointF(20.2, 30.2);
    }

    {
        // SETPOINTS
        static const int points[] = { 10, 20, 30, 40 };
        QPolygon polygon;
        polygon.setPoints(2, points);
    }

    {
        // SETPOINTS2
        QPolygon polygon;
        polygon.setPoints(2, 10, 20, 30, 40);
    }

    {
        // PUTPOINTS
        QPolygon polygon(1);
        polygon[0] = QPoint(4, 5);
        polygon.putPoints(1, 2, 6,7, 8,9);
    }

    {
        // PUTPOINTS2
        QPolygon polygon(3);
        polygon.putPoints(0, 3, 4,5, 0,0, 8,9);
        polygon.putPoints(1, 1, 6,7);
    }

    {
        // PUTPOINTS3
        QPolygon polygon1;
        polygon1.putPoints(0, 3, 1,2, 0,0, 5,6);
        // polygon1 is now the three-point polygon(1,2, 0,0, 5,6);

        QPolygon polygon2;
        polygon2.putPoints(0, 3, 4,4, 5,5, 6,6);
        // polygon2 is now (4,4, 5,5, 6,6);

        polygon1.putPoints(2, 3, polygon2);
        // polygon1 is now the five-point polygon(1,2, 0,0, 4,4, 5,5, 6,6);
    }
    return 0;
}
