#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <string>


namespace WZRY
{

    class Player
    {
    public:
        Player();
        void Init(int, int, std::string);
        void SetId(int);
    private:
        Player(const Player&) = delete;
        Player& operator=(const Player&) = delete;
    private:
        int m_id;
        int  m_socket;
        std::string m_name;
    };

}  // namespace WZRY

#endif  // _PLAYER_H_


