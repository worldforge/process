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
        int durationMsec = random() % 0x3fff; // 16 seconds max
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
        cout << "moving to point near origin: " << m_dest << endl;
    }

    virtual bool finished()
    {
        WFMath::Point<3> cur = m_char->avatar()->getEntity()->getPosition();
        cur.z() = 0;
        
        double d = (cur - m_dest).mag();
        if (d < 5.0) {
            cout << "Character returned to near origin" << endl;
            return true;
        } else {
            return false;
        }
    }
private:
    WFMath::Point<3> m_dest;
};

/*
The following are all quotes from various Penny Arcade strips. Read at you
peril.
*/
static const char* static_messages[] = {
    "Shitcock!",
    "Shitfuck",
    "You are the king of all homos.",
    "I see your queerination ceremony is almost complete.",
    "Claw shrimp. Big as your head.",
    "I loathe the undead",
    "Sweet holy jesus - fruit fucker prime!",
    "Also, it seems like they could fill more things with cream.",
    0 // important!
};

class Babble : public Action
{
public:
    Babble(Character* c) :
        Action(c)
    {
        speak();
    }
    
    virtual bool finished()
    {
        if (WFMath::TimeStamp::now() >= m_finishTime) {
            if (drand48() < 0.2) return true;
            speak();
            return false;
        } else
            return false;
    }
    
private:
    void speak()
    {
        std::string s= msg();
        cout << "saying: " << s << endl;
        m_char->avatar()->say(s);
        int durationMsec = random() % 2000; // 2 seconds max
        m_finishTime = WFMath::TimeStamp::now() + WFMath::TimeDiff(durationMsec);
    }
    
    std::string msg()
    {
        static int msgCount = 0;
        if (msgCount == 0) {
            while (static_messages[msgCount] != NULL) ++msgCount;
        }
        
        return static_messages[random() % msgCount];
    }
    
    WFMath::TimeStamp m_finishTime;
};


#pragma mark -

std::auto_ptr<Action> Action::newRandomAction(Character* c)
{
    switch (random() % 7) {
    case 1:
    case 2:
        return std::auto_ptr<Action>(new MoveNearOrigin(c));
        
    case 3:
    case 4:
    case 5:
        return std::auto_ptr<Action>(new Babble(c));
        
    default:
        return std::auto_ptr<Action>(new RandomMove(c));
    }
}
