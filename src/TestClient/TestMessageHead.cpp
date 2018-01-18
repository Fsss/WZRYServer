#include "MessageHeadProto.pb.h"

using namespace std;
using namespace WZRY;

int main()
{

    MessageHead mh;
    cout << "0      " << mh.ByteSizeLong() << "\n";

    mh.set_len(10);
    cout << "1.1    " << mh.ByteSizeLong() << "\n";
    mh.set_len(1000000000);
    cout << "1.2    " << mh.ByteSizeLong() << "\n";

    mh.set_len(10);
    mh.set_protocol(10);
    cout << "2      " << mh.ByteSizeLong() << "\n";

    return 0;
}


/*

g++ -g -pthread -lprotobuf  -Wall -std=c++11 TestMessageHead.cpp MessageHeadProto.pb.cc -o MHTest

*/


