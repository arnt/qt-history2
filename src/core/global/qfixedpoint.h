#ifndef QREAL_H
#define QREAL_H

/* Fixed point class. emulates IEEE behaviour for infinity, doesn't have NaN */

class QFixedPoint {
public:
    QFixedPoint() : val(0) {}
    QFixedPoint(int i) : val(i<<8) {}
    QFixedPoint(unsigned int i) : val(i*256) {}
    QFixedPoint(long i) : val(i*256) {}
    QFixedPoint(double d) { val = (int)(d*256.); }
    QFixedPoint(const QFixedPoint &other) : val(other.val) {}
    QFixedPoint & operator=(const QFixedPoint &other) { val = other.val; return *this; }

    inline int toInt() const { return (((val)+128) & -256)/256; }
    inline double toDouble() const { return ((double)val)/256.; }

    inline bool operator==(const QFixedPoint &other) const { return val == other.val; }
    inline bool operator!=(const QFixedPoint &other) const { return val != other.val; }
    inline bool operator<(const QFixedPoint &other) const { return val < other.val; }
    inline bool operator>(const QFixedPoint &other) const { return val > other.val; }
    inline bool operator<=(const QFixedPoint &other) const { return val <= other.val; }
    inline bool operator>=(const QFixedPoint &other) const { return val >= other.val; }
    inline bool operator!() const { return !val; }

    inline QFixedPoint operator+(const QFixedPoint &other) const { return QFixedPoint(val + other.val, F24Dot8); }
    inline QFixedPoint &operator+=(const QFixedPoint &other) { val += other.val; return *this; }

    inline QFixedPoint operator-(const QFixedPoint &other) const { return QFixedPoint(val - other.val, F24Dot8); }
    inline QFixedPoint &operator-=(const QFixedPoint &other) { val -= other.val; return *this; }

    inline QFixedPoint operator-() const { return QFixedPoint(-val, F24Dot8); }


    inline QFixedPoint &operator/=(int d) { val /= d; return *this; }
    inline QFixedPoint &operator/=(const QFixedPoint &o) {
        if (o.val == 0) {
            val =0x7FFFFFFFL;
        } else {
            bool neg = false;
            Q_INT64 a = val;
            Q_INT64 b = o.val;
            if (a < 0) { a = -a; neg = true; }
            if (b < 0) { b = -b; neg = !neg; }

            int res = (int)(((a << 8) + (b >> 1)) / b);

            val = (neg ? -res : res);
        }
        return *this;
    }
    inline QFixedPoint &operator/=(double d) { QFixedPoint v(d); return operator/=(v); }

    inline QFixedPoint operator/(int d) const { return QFixedPoint(val/d, F24Dot8); }
    inline QFixedPoint operator/(const QFixedPoint &b) const { QFixedPoint v = *this; return (v /= b); }
    inline QFixedPoint operator/(double d) const { QFixedPoint v(d); return (*this)/v; }

    inline QFixedPoint &operator*=(int i) { val *= i; return *this; }
    inline QFixedPoint &operator*=(const QFixedPoint &o) {
        bool neg = false;
        Q_INT64 a = val;
        Q_INT64 b = o.val;
        if (a < 0) { a = -a; neg = true; }
        if (b < 0) { b = -b; neg = !neg; }

        int res = (int)((a * b + 0x80L) >> 8);
        val = neg ? -res : res;
        return *this;
    }
    inline QFixedPoint &operator*=(double d) { QFixedPoint v(d); return operator*=(val); }

    inline QFixedPoint operator*(int i) const { return QFixedPoint(val*i, F24Dot8); }
    inline QFixedPoint operator*(double f) const { QFixedPoint v(f); return (v *= *this); }
    inline QFixedPoint operator*(const QFixedPoint &o) const { QFixedPoint v = *this; return (v *= o); }

    friend inline int qRound(QFixedPoint f);
    friend inline int floor(QFixedPoint f);
    friend inline int ceil(QFixedPoint f);

    int value() const { return val; }
private:
    enum T24Dot8 { F24Dot8 };
    QFixedPoint(int, T24Dot8);
    int val;
};
Q_DECLARE_TYPEINFO(QFixedPoint, Q_PRIMITIVE_TYPE);

inline QFixedPoint operator*(int i, const QFixedPoint &d) { return d*i; }
inline QFixedPoint operator*(double d, const QFixedPoint &d2) { return d2*d; }

inline QFixedPoint operator/(int i, const QFixedPoint &d) { return QFixedPoint(i)/d; }
inline QFixedPoint operator/(double d, const QFixedPoint &d2) { return QFixedPoint(d)/d2; }

inline int qRound(QFixedPoint f) { return f.val > 0 ? (f.val + 128)>>8 : (f.val - 128)/256; }
inline int floor(QFixedPoint f) { return f.val > 0 ? f.val >> 8 : (f.val - 255)/256; }
inline int ceil(QFixedPoint f) { return f.val > 0 ? (f.val + 255) >> 8 : f.val/256; }

#endif
