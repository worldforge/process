#ifndef LOADTOOL_CHARACTER_H
#define LOADTOOL_CHARACTER_H

#include <Eris/Avatar.h>
#include <Eris/View.h>
#include "Scheduler.h"

class Action;

class Character : public SigC::Object, public Schedulable
{
public:
    Character(Eris::Avatar* av);
    
    virtual void tick();
    
    Eris::Avatar* avatar() const
    { return m_avatar; }
private:
    void randomMove();
    void randomTalk();
    void moveToOrigin();
        
// lots of slots    
    void onEntityTalk();
    void onHear(Eris::Entity* src, const Atlas::Objects::Operation::RootOperation&);
    
    void onGotEntity(Eris::Entity*);
    
    Eris::View* m_view;
    Eris::Avatar* m_avatar;
    
    std::auto_ptr<Action> m_action;
};


#endif
