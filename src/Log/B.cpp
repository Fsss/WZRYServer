#include "B.h"
#include "A.h"

#include <stdio.h>


B::B(A& a) : m_a(a)
{
    
}

void B::deal()
{
    printf("BB deal");
    m_a.print();
}

