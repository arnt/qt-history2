#include <qtextcodec.h>

int main()
{
  QString input = "abcdefg";
  QTextCodec* codec;
  QTextCodec* except = QTextCodec::codecForName("ISO-10646-UCS-2");
  for (int i = 0; (codec = QTextCodec::codecForIndex(i)); i++) {
    if (codec == except)
      continue;
    
    int len = input.length();
    QCString result = codec->fromUnicode(input, len);
    if (result.length() == result.size() - 1) {
      qDebug("%s: OK", codec->name());
    } else {
      qDebug("%s: NG", codec->name());
    }
  }
}
