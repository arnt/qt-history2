#include <QApplication>
#include <QBuffer>
#include <QPalette>

static void fifo_snippet()
{
    QBuffer buffer;
    char ch;

    buffer.open(QBuffer::ReadWrite);
    buffer.write("Qt rocks!");
    buffer.getChar(&ch);  // ch == 'Q'
    buffer.getChar(&ch);  // ch == 't'
    buffer.getChar(&ch);  // ch == ' '
    buffer.getChar(&ch);  // ch == 'r'
}

static void write_datastream_snippets()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    QDataStream out(&buffer);
    out << QApplication::palette();
}

static void read_datastream_snippets()
{
    QByteArray byteArray;

    QPalette palette;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::ReadOnly);

    QDataStream in(&buffer);
    in >> palette;
}

static void bytearray_ptr_ctor_snippet()
{
    QByteArray byteArray("abc");
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    buffer.seek(3);
    buffer.write("def", 3);
    buffer.close();
    // byteArray == "abcdef"
}

static void setBuffer_snippet()
{
    QByteArray byteArray("abc");
    QBuffer buffer;
    buffer.setBuffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    buffer.seek(3);
    buffer.write("def", 3);
    buffer.close();
    // byteArray == "abcdef"
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    fifo_snippet();
    bytearray_ptr_ctor_snippet();
    write_datastream_snippets();
    read_datastream_snippets();
    setBuffer_snippet();
    return 0;
}
