#include "Action.h"
#include "Character.h"
#include <Eris/Avatar.h>
#include <Eris/Entity.h>

#include <wfmath/timestamp.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

class RandomMove : public Action
{
public:
    RandomMove(Character* c) : 
        Action(c)
    {
        int durationMsec = random() % 0x7fff; // 32 seconds max
        cout << "move duration is " << durationMsec << endl;
        m_finishTime = WFMath::TimeStamp::now() + WFMath::TimeDiff(durationMsec);
        
        WFMath::Quaternion q(2, drand48() * WFMath::Pi * 2); // random heading
        WFMath::Vector<3> vel(3.0, 0, 0);
    
        c->avatar()->moveInDirection(vel.rotate(q), q);
    }
    
    virtual ~RandomMove()
    {
    }
    
    virtual bool finished()
    {
        return (WFMath::TimeStamp::now() >= m_finishTime);
    }
    
private:
    WFMath::TimeStamp m_finishTime;
};

class MoveNearOrigin : public Action
{
public:
    MoveNearOrigin(Character* c) :
        Action(c)
    {
        m_dest = WFMath::Point<3>( 100 * (drand48() - 0.5), 100 * (drand48() - 0.5), 0.0);
        c->avatar()->moveToPoint(m_dest);
    }

    virtual bool finished()
    {
        WFMath::Point<3> cur = m_char->avatar()->getEntity()->getPosition();
        if ((cur - m_dest).mag() < 5.0) {
            cout << "Character returned to near origin" << endl;
            return true;
        } else
            return false;
    }
private:
    WFMath::Point<3> m_dest;
};

#pragma mark -

std::auto_ptr<Action> Action::newRandomAction(Character* c)
{
    switch (random() % 4) {
    case 1:
    case 2:
        return std::auto_ptr<Action>(new MoveNearOrigin(c));
        
    default:
        return std::auto_ptr<Action>(new RandomMove(c));
    }
}
