class TestScanner
{
public:
    TestScanner(const QString &inp);

    inline QString lexem() const {
        return input.mid(lexemStart, lexemLength);
    }
    int lex();

private:
    inline QChar next() {
        return (pos < input.length()) ? input.at(pos++).toLower() : QChar();
    }

    QString input;
    int pos;
    int lexemStart;
    int lexemLength;
};


TestScanner::TestScanner(const QString &inp)
{
    input = inp;
    pos = 0;
    lexemStart = 0;
    lexemLength = 0;
}


int TestScanner::lex()
{
    lexemStart = pos;
    lexemLength = 0;
    int lastAcceptingPos = -1;
    int token = -1;
    QChar ch;
    
        ch = next();
        if (ch.unicode() >= 97 && ch.unicode() <= 107)
            goto state_1;
        if (ch.unicode() == 108)
            goto state_3;
        if (ch.unicode() >= 109 && ch.unicode() <= 119)
            goto state_1;
        if (ch.unicode() == 120)
            goto state_2;
        if (ch.unicode() == 121)
            goto state_1;
        if (ch.unicode() == 122)
            goto state_1;
        goto out;
    state_1:
        ch = next();
        if (ch.unicode() == 97)
            goto state_4;
        goto out;
    state_2:
        ch = next();
        if (ch.unicode() == 97)
            goto state_4;
        if (ch.unicode() == 116)
            goto state_5;
        goto out;
    state_3:
        ch = next();
        if (ch.unicode() == 97)
            goto state_4;
        if (ch.unicode() == 101)
            goto state_6;
        goto out;
    state_4:
        lastAcceptingPos = pos;
        token = TOK_SEQ;
        goto out;
    state_5:
        ch = next();
        if (ch.unicode() == 114)
            goto state_7;
        goto out;
    state_6:
        ch = next();
        if (ch.unicode() == 116)
            goto state_8;
        goto out;
    state_7:
        ch = next();
        if (ch.unicode() == 97)
            goto state_9;
        goto out;
    state_8:
        lastAcceptingPos = pos;
        token = TOK_LET;
        ch = next();
        if (ch.unicode() == 120)
            goto state_10;
        goto out;
    state_9:
        lastAcceptingPos = pos;
        token = TOK_XTRA;
        goto out;
    state_10:
        ch = next();
        if (ch.unicode() == 120)
            goto state_11;
        goto out;
    state_11:
        lastAcceptingPos = pos;
        token = TOK_LETXX;
        goto out;
    out:
    if (lastAcceptingPos != -1) {
        lexemLength = lastAcceptingPos - lexemStart;
        pos = lastAcceptingPos;
    }
    return token;
}

