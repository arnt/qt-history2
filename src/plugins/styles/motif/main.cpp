#include <qstyleplugin.h>
#include <qmotifstyle.h>

class Qt::MotifStyle : public QStylePlugin
{
public:
    Qt::MotifStyle();

    QStringList keys() const;
    QStyle *create(const QString&);
};

Qt::MotifStyle::MotifStyle()
: QStylePlugin()
{
}

QStringList Qt::MotifStyle::keys() const
{
    QStringList list;
    list << "Motif";
    return list;
}

QStyle* Qt::MotifStyle::create(const QString& s)
{
    if (s.toLower() == "motif")
        return new QMotifStyle();

    return 0;
}

Q_EXPORT_PLUGIN(Qt::MotifStyle)

