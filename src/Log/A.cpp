#include "A.h"
#include <stdio.h>

A::A() : m_b(*this)
{
    
}

void A::print()
{
    printf("AA print");
}

void A::Input()
{
    printf("AA Input");
    m_b.deal();
}

