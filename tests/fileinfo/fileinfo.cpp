#include <qfileinfo.h>
#include <qstring.h>
#include <ostream.h>

// Test conditions:

// Under Windows:

// - create directory "D:\abc"                 md D:\abc
// - make that your current working directory  cd D:\abc
// - create directory "C:\tmp"                 md C:\tmp
// - make that your current working directory  cd C:\tmp
// - make drive C: your current drive          C:

// Under Linux:

// just do it!

char *p[][6] = {
    //   Input             dirPath()  dirPath(true)  fileName()  filePath()        absFilePath()
#ifdef Q_OS_LINUX
    { "/xyz/sam.c",     "/xyz",    "/xyz",        "sam.c",    "/xyz/sam.c",     "/xyz/sam.c"   },
    { "/sam.c",         "/",       "/",           "sam.c",    "/sam.c",         "/sam.c"       },
#else
    { "D:\\abc\\sam.c", "D:/abc",  "D:/abc",      "sam.c",    "D:/abc/sam.c",   "D:/abc/sam.c" },
    { "D:/abc/sam.c",   "D:/abc",  "D:/abc",      "sam.c",    "D:/abc/sam.c",   "D:/abc/sam.c" },
    { "D:sam.c",        "D:",      "D:/abc",      "sam.c",    "D:sam.c",        "D:/abc/sam.c" },
    { "D:\\sam.c",      "D:/",     "D:/",         "sam.c",    "D:/sam.c",       "D:/sam.c"     },
    { "\\xyz\\sam.c",   "/xyz",    "C:/xyz",      "sam.c",    "/xyz/sam.c",     "C:/xyz/sam.c" },
    { "sam.c",          ".",       "C:/tmp",      "sam.c",    "sam.c",          "C:/tmp/sam.c" },
    { "\\sam.c",        "/",       "C:/",         "sam.c",    "/sam.c",         "C:/sam.c"     },
#endif
    { NULL }
};

#define TEST(fn, input, expected)\
{\
QFileInfo fi(input);\
if (fi.fn != expected)\
cout << "QFileInfo(" << input << ")."#fn" = '" << fi.fn << "'  expected: '" << expected << "'" << endl;\
}

int main(void)
{
    for (char **s = &p[0][0]; *s; s += 6) {
	TEST(dirPath(),     *s, *(s + 1));
	TEST(dirPath(TRUE), *s, *(s + 2));
	TEST(fileName(),    *s, *(s + 3));
	TEST(filePath(),    *s, *(s + 4));
	TEST(absFilePath(), *s, *(s + 5));
    }
    cout << "Fileinfo: done." << endl;
    return 0;
}

