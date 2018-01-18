#include "Player.h"
#include <string>

namespace WZRY
{

    Player::Player()
    {}

    void Player::Init(int id, int sock, std::string name)
    {
        m_id = id;
        m_socket = sock;
        m_name = name;
    }

}  // namespace WZRY







