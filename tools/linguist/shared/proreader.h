#ifndef PROREADER_H
#define PROREADER_H

#include <QtCore/QStack>
#include <QtCore/QList>
#include <QtCore/QTextStream>

class ProBlock;
class ProItem;
class ProFile;

class ProReader
{
public:
    ProReader();
    ProFile *read(const QString &fileName);
    void setEnableBackSlashFixing(bool enable);

protected:
    ProFile *read(QIODevice *device, const QString &name = QLatin1String("device"));

    void writeItem(const QList<ProItem *> &items, int index, QTextStream &out, QString indent);
    ProBlock *currentBlock();
    void updateItem();
    bool parseline(QByteArray line);
    void insertVariable();
    void insertOperator(const char op);
    void insertComment(const QByteArray &comment);
    void enterScope(bool multiLine);
    void leaveScope();
    void finalizeBlock();
    void cleanup();

private:
    QStack<ProBlock *> m_blockstack;
    ProBlock *m_block;

    ProItem *m_commentItem;
    QByteArray m_proitem;
    QByteArray m_pendingComment;
    bool    m_fixBackSlashes;
#ifdef PROPARSER_STORE_LINENUMBERS
# define ASSIGN_LINENUMBER(item) item->setLineNumber(m_currentLineNumber)
    int m_currentLineNumber;
#else 
# define ASSIGN_LINENUMBER(item)
#endif
};

#endif //PROREADER_H
