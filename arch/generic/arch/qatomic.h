#ifndef QATOMIC_P_H
#define QATOMIC_P_H

inline int q_cas_32(int * volatile ptr, int expected, int newval)
{
    int p = *ptr;
    if (p == expected) {
        *ptr = newval;
        return expected;
    }
    return p;
}

inline void *q_cas_ptr(void * volatile *ptr, void *expected, void *newval)
{
    void *p = *ptr;
    if (p == expected) {
        *ptr = newval; 
        return expected;
    }
    return p;
}

#endif // QATOMIC_P_H

