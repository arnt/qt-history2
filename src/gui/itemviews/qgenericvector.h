#ifndef QGENERICVECTOR_H
#define QGENERICVECTOR_H

#include <qvector.h>
#include <string.h> // memmove

template <typename T>
class QGenericVector : public QVector<T>
{
public:
    QGenericVector() : QVector<T>() {}
    QGenericVector(size_t n, const T &v = T()) : QVector<T>(n, v) {}
    ~QGenericVector() {}

    inline void expand(int after, size_t n)
    {
	size_t m = size() - after - 1;
	resize(size() + n);
 	T *b = (T*)data();
 	T *src = b + after + 1;
 	T *dst = src + n;
 	memmove(dst, src, m * sizeof(T));
    }

    inline void collaps(int after, size_t n)
    {
	if (after + 1 + n < (size_t)size()) {
  	    T *b = data();
  	    T *dst = b + after + 1;
  	    T *src = dst + n;
	    size_t m = size() - n - after - 1;
  	    memmove(dst, src, m * sizeof(T));
	}
	resize(size() - n);
    }
    
    inline void insert(int i, const T &item)
    {
 	expand(i, 1);
 	(*this)[i + 1] = item;
    }

    inline void remove(int i)
    {
	collaps(i - 1, 1);
    }
};

template <typename T>
int bsearch(const QVector<T> &vec, const T &item, int first, int last)
{
    T val;
    int mid;
    while (true) {
	mid = first + ((last - first) >> 1);
	val = vec[mid];
	if (val == item || (last - first) < 2)
	    return mid;
	if (val > item)
	    last = mid;
	if (val < item)
	    first = mid;
    }
}

#endif // QGENERICVECTOR_H
