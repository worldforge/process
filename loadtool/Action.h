#ifndef LOADTOOL_ACTION_H
#define LOADTOOL_ACTION_H

#include <memory>

class Character;

class Action
{
public:
    virtual ~Action()
    {
    }
    
    virtual void run()
    {
    }
    
    virtual bool finished() = 0;

    static std::auto_ptr<Action> newRandomAction(Character* c);

protected:
    Action(Character* c) : m_char(c) { }
    
    Character* m_char;
};

#endif
