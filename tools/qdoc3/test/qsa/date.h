/*
    Wrapper for Date class.

    The EcmaScript Date class is not implemented like the other
    built-in classes (e.g. Array). The easiest way for qdoc to cope
    with it is to define a fake wrapper, i.e. a wrapper class that
    will never go through any C++ compiler.
*/

#ifdef qdoc

class QuickDate
{
public slots:
    QString toString() const;
    QString toUTCString() const;
    QString toDateString() const;
    QString toTimeString() const;
    QString toLocaleString() const;
    QString toLocaleDateString() const;
    QString toLocaleTimeString() const;
    int valueOf() const;
    int getTime() const;
    int getFullYear() const;
    int getUTCFullYear() const;
    int getMonth() const;
    int getUTCMonth() const;
    int getDate() const;
    int getUTCDate() const;
    int getDay() const;
    int getUTCDay() const;
    int getHours() const;
    int getUTCHours() const;
    int getMinutes() const;
    int getUTCMinutes() const;
    int getSeconds() const;
    int getUTCSeconds() const;
    int getMilliseconds() const;
    int getUTCMilliseconds() const;
    int getTimezoneOffset() const;

    void setTime( int );
    void setMilliseconds( int );
    void setUTCMilliseconds( int );
    void setSeconds( int );
    void setUTCSeconds( int );
    void setMinutes( int );
    void setUTCMinutes( int );
    void setHours( int );
    void setUTCHours( int );
    void setDate( int );
    void setUTCDate( int );
    void setMonth( int );
    void setUTCMonth( int );
    void setFullYear( int );
    void setUTCFullYear( int );

    int getYear() const;
    void setYear( int );
    QString toGMTString() const;
};

#endif
