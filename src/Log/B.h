#ifndef _B_H_
#define _B_H_

class A;

class B
{
    public:
        B(A&);
        void deal();
    private:
        A& m_a;
};


#endif  // _B_H_


