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
        m_finishTime = WFMath::TimeStamp::now() + WFMath::TimeDiff(durationMsec);
        
        WFMath::Quaternion q(2, drand48() * WFMath::Pi * 2); // random heading
        double scalarVel = 3.0 + (drand48() * 3.0);
        WFMath::Vector<3> vel(scalarVel, 0, 0);
    
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
        m_dest = WFMath::Point<3>( 10 * (drand48() - 0.5), 10 * (drand48() - 0.5), 0.0);
        c->avatar()->moveToPoint(m_dest);
    }

    virtual bool finished()
    {
        WFMath::Point<3> cur = m_char->avatar()->getEntity()->getPosition();
        cur.z() = 0;
        
        double d = (cur - m_dest).mag();
        if (d < 3.0) {
            // within 3 metres of target, that'll do
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
    "I said Fuck Yeah!",
    "my nipples nooooooo",
    "i had sex with a bird and now I caught the asian bird flu",
    "i know a long word that means squeezed to death by a snake",
    "Did you hear that shitbag Scott Kurtz was nominated for an Eisner?",
    "You are the anchor around my neck. You are drowning me.",
    "Christ? Judas is teamkilling again.",
    "Cut yourself.",
    "You could get the fuck out of my face.",
    "I just baked a cinnamon roll as big as my head!",
    "I think the main problem I have is that you are a shitfucker.",
    "Yesterday, I kissed a boy! Does that mean I'm gay?",
    "The delivery guy from Ikea went poop in our bathroom.",
    "I smolder with generic rage.",
    "This game will give you leprosy.",
    "I am talking about a chunk of metal so far up my ass it's a Goddam tourist attraction.",
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
            if (drand48() < 0.3) return true;
            speak();
            return false;
        } else
            return false;
    }
    
private:
    void speak()
    {
        std::string s= msg();
        m_char->avatar()->say(s);
        int durationMsec = 1000 + (random() % 1500);
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
        return std::auto_ptr<Action>(new Babble(c));
    
    case 5:
    default:
        return std::auto_ptr<Action>(new RandomMove(c));
    }
}
