#include "employee.h"

int main()
{
    {
        Employee e1(10, "Alfonso Berlusconi");
        Employee e2 = e1;
        e1.setName("Bill Gates");
    }
}
