#include "dataparser.h"
#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>

#include <QtDebug>

static QList<qreal> parseNumbersList(QByteArray::const_iterator &itr)
{
    QList<qreal> points;
    QByteArray temp;
    while ((*itr) == ' ')
        ++itr;
    while (((*itr) >= '0' && (*itr) <= '9') ||
           (*itr) == '-' || (*itr) == '+') {
        temp = QByteArray();

        if ((*itr) == '-')
            temp += *itr++;
        else if ((*itr) == '+')
            temp += *itr++;
        while ((*itr) >= '0' && (*itr) <= '9')
            temp += *itr++;
        if ((*itr) == '.')
            temp += *itr++;
        while ((*itr) >= '0' && (*itr) <= '9')
            temp += *itr++;
        if (( *itr) == 'e') {
            temp += *itr++;
            if ((*itr) == '-' ||
                (*itr) == '+')
                temp += *itr++;
        }
        while ((*itr) >= '0' && (*itr) <= '9')
            temp += *itr++;
        while ((*itr) == ' ')
            ++itr;
        if ((*itr) == ',')
            ++itr;
        points.append(temp.toDouble());
        //eat the rest of space
        while ((*itr) == ' ')
            ++itr;
    }

    return points;
}

static QList<QPointF> parsePoints(const QByteArray &line)
{
    QList<QPointF> res;

    QByteArray::const_iterator it = line.constBegin();
    if (*it == ',')
        ++it;
    
    QList<qreal> nums = parseNumbersList(it);
    QList<qreal>::const_iterator nitr;
    for (nitr = nums.begin(); nitr != nums.end(); ++nitr) {
        qreal x = *nitr; ++nitr;
        Q_ASSERT(nitr != nums.end());
        qreal y = *nitr;
        res.append(QPointF(x, y));
    }

    return res;
}

QList< QVector<QPointF> > parseData(const QByteArray &contents)
{
    QList<QByteArray> lines = contents.split('\n');
    QList<QByteArray>::const_iterator itr;

    QList< QVector<QPointF> > res;
    QVector<QPointF> current;

    for (itr = lines.begin(); itr != lines.end(); ++itr) {
        QByteArray line = (*itr).trimmed();
        if (line.isEmpty() || line.startsWith('/')) {
            if (!current.isEmpty()) {
                res.append(current);
                current = QVector<QPointF>();
            }
            continue;
        } else {
            QList<QPointF> lst = parsePoints(line);
            current << lst.toVector();
        }
    }
    return res;
}

QList< QVector<QPointF> > parseFile(const QString &fileName)
{
    QList< QVector<QPointF> > res;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug()<<"couldn't open "<<fileName;
        return res;
    }

    QVector<QPointF> current;

    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('/')) {
            if (!current.isEmpty()) {
                res.append(current);
                current = QVector<QPointF>();
            }
            continue;
        } else {
            QList<QPointF> lst = parsePoints(line);
            current << lst.toVector();
        }
    }

    return res;
}
