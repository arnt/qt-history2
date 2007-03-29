/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <metatranslator.h>

#include <QFile>
#include <QRegExp>
#include <QString>
#include <QStack>
#include <QTextCodec>
#include <QStack>
#include <QDebug>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

enum { Tok_Eof, Tok_class, Tok_return, Tok_tr,
       Tok_translate, Tok_Ident, Tok_Package,
       Tok_Comment, Tok_String, Tok_Colon, Tok_Dot,
       Tok_LeftBrace, Tok_RightBrace, Tok_LeftParen,
       Tok_RightParen, Tok_Comma, Tok_Semicolon, Tok_Integer };

class Scope
{
    public:
        QString name;
        enum Type {Clazz, Function, Other} type;
        int line;
       
        Scope(QString name, Type type, int line) :
                name(name),
                type(type),
                line(line)
        {}
        
        ~Scope()
        {}
};

/*
  The tokenizer maintains the following global variables. The names
  should be self-explanatory.
*/

static QByteArray yyFileName;
static int yyCh;
static char yyIdent[128];
static size_t yyIdentLen;
static char yyComment[65536];
static size_t yyCommentLen;

static char yyString[65536];
static size_t yyStringLen;
static qlonglong yyInteger;
static int yyParenDepth;
static int yyLineNo;
static int yyCurLineNo;
static int yyParenLineNo;
static int yyTok;
static QTextCodec *yyCodecForTr = 0;
static QTextCodec *yyCodecForSource = 0;

// the file to read from (if reading from a file)
static FILE *yyInFile;

// the string to read from and current position in the string (otherwise)
static QString yyInStr;
static int yyInPos;

static int (*getChar)();

static bool yyParsingUtf8;

// The parser maintains the following global variables.
static QString yyPackage;
static QStack<Scope*> yyScope;
static QString yyDefaultContext;

static int getCharFromFile()
{
    // ### TODO. what about encoding UTF-16?
    int c = getc( yyInFile );
    if ( c == '\n' )
        yyCurLineNo++;
    return c;
}

static void startTokenizer( const char *fileName, int (*getCharFunc)(), QTextCodec *codecForTr, QTextCodec *codecForSource )
{
    yyInPos = 0;
    getChar = getCharFunc;
    
    yyFileName = fileName;
    yyPackage = "";

    yyScope.clear();    
    
    yyTok = -1;
    
    yyParenDepth = 0;
    yyCurLineNo = 1;

    yyParenLineNo = 1;
    yyCh = getChar();
    yyCodecForTr = codecForTr;
    if (!yyCodecForTr)
        yyCodecForTr = QTextCodec::codecForName("ISO-8859-1");
    Q_ASSERT(yyCodecForTr);
    yyCodecForSource = codecForSource;
    
    yyParsingUtf8 = false;
}

static int getToken()
{
    const char tab[] = "abfnrtv";
    const char backTab[] = "\a\b\f\n\r\t\v";
    uint n;
    bool quiet;
    
    yyIdentLen = 0;
    yyCommentLen = 0;
    yyStringLen = 0;
    
    while ( yyCh != EOF ) {
        yyLineNo = yyCurLineNo;
        
        if ( isalpha(yyCh) || yyCh == '_' ) {
            do {
                if ( yyIdentLen < sizeof(yyIdent) - 1 )
                    yyIdent[yyIdentLen++] = (char) yyCh;
                yyCh = getChar();
            } while ( isalnum(yyCh) || yyCh == '_' );
            yyIdent[yyIdentLen] = '\0';
            
            if(yyTok != Tok_Dot)
            {
                switch ( yyIdent[0] ) {
                    case 'r':
                        if ( strcmp(yyIdent + 1, "eturn") == 0 )
                            return Tok_return;
                        break;
                     case 'c':
                            if ( strcmp(yyIdent + 1, "lass") == 0 )
                        return Tok_class;
                    break;
                }
            }
            switch ( yyIdent[0] ) {
            case 'T':
                // TR() for when all else fails
                if ( qstricmp(yyIdent + 1, "R") == 0 ) {
                    yyParsingUtf8 = false;
                    return Tok_tr;
                }    
                break;
            case 'p':
                if( strcmp(yyIdent +1, "ackage") == 0 )
                    return Tok_Package;
                break;
            case 't':
                if ( strcmp(yyIdent + 1, "r") == 0 ) {
                    yyParsingUtf8 = false;
                    return Tok_tr;
                } else if ( qstrcmp(yyIdent + 1, "ranslate") == 0 ) {
                    yyParsingUtf8 = false;
                    return Tok_translate;
                }
            }
            return Tok_Ident;
        } else {
            switch ( yyCh ) {
                
            case '/':
                yyCh = getChar();
                if ( yyCh == '/' ) {
                    do {
                        yyCh = getChar();
                        yyComment[yyCommentLen++] = (char) yyCh;
                    } while ( yyCh != EOF && yyCh != '\n' );
                    yyComment[yyCommentLen] = '\0';
                    return Tok_Comment;
                } else if ( yyCh == '*' ) {
                    bool metAster = false;
                    bool metAsterSlash = false;
                    
                    while ( !metAsterSlash ) {
                        yyCh = getChar();
                        if ( yyCh == EOF ) {
                            fprintf( stderr,
                                     "%s: Unterminated C++ comment starting at"
                                     " line %d\n",
                                     (const char *) yyFileName, yyLineNo );
                            yyComment[yyCommentLen] = '\0';
                            return Tok_Comment;
                        }
                        if ( yyCommentLen < sizeof(yyComment) - 1 )
                            yyComment[yyCommentLen++] = (char) yyCh;
                        
                        if ( yyCh == '*' )
                            metAster = true;
                        else if ( metAster && yyCh == '/' )
                            metAsterSlash = true;
                        else
                            metAster = false;
                    }
                    yyCh = getChar();
                    yyCommentLen -= 2;
                    yyComment[yyCommentLen] = '\0';
                    return Tok_Comment;
                }
                break;
            case '"':
                yyCh = getChar();
                quiet = false;
                
                while ( yyCh != EOF && yyCh != '\n' && yyCh != '"' ) {
                    if ( yyCh == '\\' ) {
                        yyCh = getChar();
                        
                        if ( yyCh == '\n' ) {
                            yyCh = getChar();
                        } else if ( yyCh == 'x' ) {
                            QByteArray hex = "0";
                            
                            yyCh = getChar();
                            while ( isxdigit(yyCh) ) {
                                hex += (char) yyCh;
                                yyCh = getChar();
                            }
#if defined(_MSC_VER) && _MSC_VER >= 1400
                            sscanf_s( hex, "%x", &n );
#else
                            sscanf( hex, "%x", &n );
#endif
                            if ( yyStringLen < sizeof(yyString) - 1 )
                                yyString[yyStringLen++] = (char) n;
                        } else if ( yyCh >= '0' && yyCh < '8' ) {    //maybe remove
                            QByteArray oct = "";
                            int n = 0;
                            
                            do {
                                oct += (char) yyCh;
                                ++n;
                                yyCh = getChar();
                            } while ( yyCh >= '0' && yyCh < '8' && n < 3 );
#if defined(_MSC_VER) && _MSC_VER >= 1400
                            sscanf_s( oct, "%o", &n );
#else
                            sscanf( oct, "%o", &n );
#endif
                            if ( yyStringLen < sizeof(yyString) - 1 )
                                yyString[yyStringLen++] = (char) n;
                        } else {
                            const char *p = strchr( tab, yyCh );
                            if ( yyStringLen < sizeof(yyString) - 1 )
                                yyString[yyStringLen++] = ( p == 0 ) ?
                                                          (char) yyCh : backTab[p - tab];
                            yyCh = getChar();
                        }
                    } else {
                        if (!yyCodecForSource) {
                            if ( yyParsingUtf8 && yyCh >= 0x80 && !quiet) {
                                qWarning( "%s:%d: Non-ASCII character detected in trUtf8 string",
                                          (const char *) yyFileName, yyLineNo );
                                quiet = true;
                            }
                            // common case: optimized
                            if ( yyStringLen < sizeof(yyString) - 1 )
                                yyString[yyStringLen++] = (char) yyCh;
                            yyCh = getChar();
                        } else {
                            QByteArray originalBytes;
                            while ( yyCh != EOF && yyCh != '\n' && yyCh != '"' && yyCh != '\\' ) {
                                if ( yyParsingUtf8 && yyCh >= 0x80 && !quiet) {
                                    qWarning( "%s:%d: Non-ASCII character detected in trUtf8 string",
                                              (const char *) yyFileName, yyLineNo );
                                    quiet = true;
                                }
                                originalBytes += (char)yyCh;
                                yyCh = getChar();
                            }
                            
                            QString unicodeStr = yyCodecForSource->toUnicode(originalBytes);
                            QByteArray convertedBytes;
                            
                            if (!yyCodecForTr->canEncode(unicodeStr) && !quiet) {
                                qWarning( "%s:%d: Cannot convert C++ string from %s to %s",
                                          (const char *) yyFileName, yyLineNo, yyCodecForSource->name().constData(),
                                          yyCodecForTr->name().constData() );
                                quiet = true;
                            }
                            convertedBytes = yyCodecForTr->fromUnicode(unicodeStr);
                            
                            size_t len = qMin((size_t)convertedBytes.size(), sizeof(yyString) - yyStringLen - 1);
                            memcpy(yyString + yyStringLen, convertedBytes.constData(), len);
                            yyStringLen += len;
                        }
                    }
                }
                yyString[yyStringLen] = '\0';
                
                if ( yyCh != '"' )
                    qWarning( "%s:%d: Unterminated string",
                              (const char *) yyFileName, yyLineNo );
                
                if ( yyCh == EOF ) {
                    return Tok_Eof;
                } else {
                    yyCh = getChar();
                    return Tok_String;
                }
                break;
            case ':':
                yyCh = getChar();
                return Tok_Colon;
            case '\'':
                yyCh = getChar();
                if ( yyCh == '\\' )
                    yyCh = getChar();
                
                do {
                    yyCh = getChar();
                } while ( yyCh != EOF && yyCh != '\'' );
                yyCh = getChar();
                break;
            case '{':
                yyCh = getChar();
                return Tok_LeftBrace;
            case '}':
                yyCh = getChar();
                return Tok_RightBrace;
            case '(':
                if (yyParenDepth == 0)
                    yyParenLineNo = yyCurLineNo;
                yyParenDepth++;
                yyCh = getChar();
                return Tok_LeftParen;
            case ')':
                if (yyParenDepth == 0)
                    yyParenLineNo = yyCurLineNo;
                yyParenDepth--;
                yyCh = getChar();
                return Tok_RightParen;
            case ',':
                yyCh = getChar();
                return Tok_Comma;
            case '.':
                yyCh = getChar();
                return Tok_Dot;
            case ';':
                yyCh = getChar();
                return Tok_Semicolon;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                {
                    QByteArray ba;
                    ba+=yyCh;
                    yyCh = getChar();
                    bool hex = yyCh == 'x';
                    if ( hex ) {
                        ba+=yyCh;
                        yyCh = getChar();
                    }
                    while ( (hex ? isxdigit(yyCh) : isdigit(yyCh)) ) {
                        ba+=yyCh;
                        yyCh = getChar();
                    }
                    bool ok;
                    yyInteger = ba.toLongLong(&ok);
                    if (ok) return Tok_Integer;
                    break;
                }
            default:
                yyCh = getChar();
            }
        }
    }
    return Tok_Eof;
}

static bool match( int t )
{
    bool matches = ( yyTok == t );
    if ( matches )
        yyTok = getToken();
    return matches;
}

static bool matchString( QByteArray *s )
{
    bool matches = ( yyTok == Tok_String );
    *s = "";
    while ( yyTok == Tok_String ) {
        *s += yyString;
        yyTok = getToken();
    }
    return matches;
}

static bool matchEncoding( bool *utf8 )
{
    if ( yyTok == Tok_Ident ) {
        // com.trolltech.qt.QCoreApplication.encoding
        while(strcmp(yyIdent, "Encoding") != 0) {
            getToken();
            if(!( match( Tok_Dot ) && yyTok == Tok_Ident ))
                return false;
        }

        yyTok = getToken();

        if(!( match( Tok_Dot ) && yyTok == Tok_Ident )) {
            return false;
        }
    
        if (strcmp(yyIdent, "UnicodeUTF8") == 0) {
            *utf8 = true;
            yyTok = getToken();
        } else if (strcmp(yyIdent, "CodecForTr") == 0) {
            *utf8 = false;
            yyTok = getToken();
        } else {
            return false;
        }
        return true;
    } else {
        return false;
    }
}

static bool matchInteger( qlonglong *number)
{
    bool matches = (yyTok == Tok_Integer);
    if (matches) {
        yyTok = getToken();
        *number = yyInteger;
    }
    return matches;
}

static bool matchStringOrNull(QByteArray *s)
{
    bool matches = matchString(s);
    qlonglong num = 0;
    if (!matches) matches = matchInteger(&num);
    return matches && num == 0;
}

/*
 * match any expression that can return a number, which can be
 * 1. Literal number (e.g. '11')
 * 2. simple identifier (e.g. 'm_count')
 * 3. simple function call (e.g. 'size()' )
 * 4. function call on an object (e.g. 'list.size()')
 * 5. function call on an object (e.g. 'list->size()')
 *
 * Other cases:
 * size(2,4)
 * list().size()
 * list(a,b).size(2,4)
 * etc...
 */
static bool matchExpression()
{
    if (match(Tok_Integer)) {
        return true;
    }
    
    int parenlevel = 0;
    while (match(Tok_Ident) || parenlevel > 0) {
        if (yyTok == Tok_RightParen) {
            if (parenlevel == 0) break;
            --parenlevel;
            yyTok = getToken();
        } else if (yyTok == Tok_LeftParen) {
            yyTok = getToken();
            if (yyTok == Tok_RightParen) {
                yyTok = getToken();
            } else {
                ++parenlevel;
            }
        } else if (yyTok == Tok_Ident) {
            continue;
        } else if (parenlevel == 0) {
            return false;
        }
    }
    return true;
}

static const QString context()
{
      QString context(yyPackage);
      bool innerClass = false;
      for (int i = 0; i < yyScope.size(); ++i) {
         if (yyScope.at(i)->type == Scope::Clazz){
             if(innerClass)
                 context.append("$");
             else
                 context.append(".");
                         
             context.append(yyScope.at(i)->name);
             innerClass = true;
         }    
     }
     return context.isEmpty() ? yyDefaultContext : context;
}

static void parse( MetaTranslator *tor )
{
    QByteArray text;
    QByteArray com;

    bool utf8 = false;
    
    yyTok = getToken();
    while ( yyTok != Tok_Eof ) {
        switch ( yyTok ) {
        case Tok_class:
            yyTok = getToken();
            if(yyTok == Tok_Ident) {
                yyScope.push(new Scope(yyIdent, Scope::Clazz, yyLineNo));
            }
            else {
                qFatal( "%s:%d: Class must be followed by a classname",
                                          (const char *) yyFileName, yyLineNo );
            }
            while (!match(Tok_LeftBrace)) {
                yyTok = getToken();
            }
            break;

        case Tok_tr:
            yyTok = getToken();
            if ( match(Tok_LeftParen) && matchString(&text) ) {
                com = "";
                bool plural = false;
                
                if ( match(Tok_RightParen) ) {
                    // no comment
                } else if (match(Tok_Comma) && matchStringOrNull(&com)) {   //comment
                    if ( match(Tok_RightParen)) {
                        // ok,
                    } else if (match(Tok_Comma)) {
                        plural = true;
                    }
                }
                tor->insert( MetaTranslatorMessage(context().toLatin1(), text, com, QLatin1String(yyFileName), yyLineNo,
                    QStringList(), false, MetaTranslatorMessage::Unfinished, plural) );
            }
            break;
        case Tok_translate:
            {
                QByteArray contextOverride;
                utf8 = false;
                yyTok = getToken();
                if ( match(Tok_LeftParen) &&
                     matchString(&contextOverride) &&
                     match(Tok_Comma) &&
                     matchString(&text) ) {

                    com = "";
                    bool plural = false;
                    if (!match(Tok_RightParen)) {
                        // look for comment
                        if ( match(Tok_Comma) && matchStringOrNull(&com)) {
                            if (!match(Tok_RightParen)) {
                                // look for encoding
                                if (match(Tok_Comma)) {
                                    if (matchEncoding(&utf8)) {
                                        if (!match(Tok_RightParen)) {
                                            if (match(Tok_Comma) && matchExpression() && match(Tok_RightParen)) {
                                                plural = true;
                                            }
                                        }
                                    }
                                } else {
                                    break;
                                }
                            }
                        } else {
                            break;
                        }
                    }
                    tor->insert( MetaTranslatorMessage(contextOverride, text, com, QLatin1String(yyFileName), yyLineNo,
                                                       QStringList(), utf8, MetaTranslatorMessage::Unfinished, plural) );
                }
            }
            break;

        case Tok_Ident:
            yyTok = getToken();
            break;
            
        case Tok_RightBrace:
            if ( yyScope.isEmpty() )
            {
                qFatal( "%s:%d: Unbalanced right brace in Java code\n",
                        (const char *)yyFileName, yyLineNo );
            }
            else
                delete (yyScope.pop());
            yyTok = getToken();
            break;
            
         case Tok_LeftBrace:
            yyScope.push(new Scope("", Scope::Other, yyLineNo));
            yyTok = getToken();
            break;
            
        case Tok_Semicolon:
            yyTok = getToken();
            break;
            
        case Tok_Package:
            yyTok = getToken();
            while(!match(Tok_Semicolon)) {
                switch(yyTok) {
                    case Tok_Ident:
                        yyPackage.append(yyIdent);
                        break;
                    case Tok_Dot:
                        yyPackage.append(".");
                        break;
                    default:
                         qFatal( "%s:%d: Package keyword should be followed by com.package.name;",
                                          (const char *) yyFileName, yyLineNo );
                         break;
                }
                yyTok = getToken();
            }
            break;
            
        default:
            yyTok = getToken();
        }
    }
    
    if ( !yyScope.isEmpty() )
        qFatal( "%s:%d: Unbalanced braces in Java code\n",
                 (const char *)yyFileName, yyScope.top()->line );
    else if ( yyParenDepth != 0 )
        qFatal( "%s:%d: Unbalanced parentheses in Java code\n",
                 (const char *)yyFileName, yyParenLineNo );
}

void fetchtr_java( const char *fileName,  MetaTranslator *tor,
                   const char *defaultContext, bool mustExist, const QByteArray &codecForSource )
{
    yyDefaultContext = defaultContext;   
    
#if defined(_MSC_VER) && _MSC_VER >= 1400
    if (fopen_s(&yyInFile, fileName, "r")) {
        if ( mustExist ) {
            char buf[100];
            strerror_s(buf, sizeof(buf), errno);
            fprintf( stderr,
                     "lupdate error: Cannot open java source file '%s': %s\n",
                     fileName, buf );
        }
#else
        yyInFile = fopen( fileName, "r" );
        if ( yyInFile == 0 ) {
            if ( mustExist )
                fprintf( stderr,
                         "lupdate error: Cannot open java source file '%s': %s\n",
                         fileName, strerror(errno) );
#endif
            return;
    }
    
    startTokenizer( fileName, getCharFromFile, tor->codecForTr(), QTextCodec::codecForName(codecForSource) );
    parse( tor );
    fclose( yyInFile );
}
