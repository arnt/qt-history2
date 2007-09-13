#ifndef QBIC_H
#define QBIC_H

#include "QtCore/qhash.h"
#include "QtCore/qlist.h"
#include "QtCore/qpair.h"
#include "QtCore/qregexp.h"
#include "QtCore/qstring.h"
#include "QtCore/qstringlist.h"

QT_DECLARE_CLASS(QByteArray)

class QBic
{
public:
    struct Info
    {
        QHash<QString, int> classSizes;
        QHash<QString, QStringList> classVTables;
    };

    struct VTableDiff
    {
        QList<QPair<QString, QString> > reimpMethods;
        QList<QPair<QString, QString> > modifiedVTables;
        QStringList addedVTables;
        QStringList removedVTables;
    };

    struct SizeDiff
    {
        QStringList mismatch;
        QStringList added;
        QStringList removed;
    };

    void addBlacklistedClass(const QString &wildcard);
    bool isBlacklisted(const QString &className) const;

    Info parseOutput(const QByteArray &ba) const;
    Info parseFile(const QString &fileName) const;

    VTableDiff diffVTables(const Info &oldLib, const Info &newLib) const;
    SizeDiff diffSizes(const Info &oldLib, const Info &newLib) const;

private:
    QList<QRegExp> blackList;
};

#endif
