#ifndef _SOLOMESSAGE_H_
#define _SOLOMESSAGE_H_

#include "ServerMessageProto.pb.h"
#include "Protobuf.h"


#include <stddef.h>

namespace WZRY
{
    class SoloBattle;

    class SoloMessage
    {
    public:
        SoloMessage(SoloBattle&);
        void Receive(int);
        void Send(int, ChatRSPS);
        void Send(int, MoveRSPS);
        void Send(int, SkillRSPS);
    private:
        SoloMessage(const SoloMessage&) = delete;
        SoloMessage& operator=(const SoloMessage&) = delete;
        int ReadN(int, void*, size_t);
        int WriteN(int, const void*, size_t);
    private:
        SoloBattle& m_soloBattle;
    };

}  // namespace WZRY

#endif  // _SOLOMESSAGE_H_



