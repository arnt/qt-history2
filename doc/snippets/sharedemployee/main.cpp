#include "employee.h"

int main()
{
    {
        Employee e1(10, "Albrecht D�rer");
        Employee e2 = e1;
        e1.setName("Hans Holbein");
    }
}
