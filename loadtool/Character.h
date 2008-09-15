#ifndef LOADTOOL_CHARACTER_H
#define LOADTOOL_CHARACTER_H

#include <Eris/Avatar.h>
#include <Eris/View.h>
#include "Scheduler.h"

#include <sigc++/trackable.h>

#include <memory>

class Action;

class Character : public sigc::trackable, public Schedulable
{
public:
    Character(Eris::Avatar* av);
    virtual ~Character();
    
    virtual void tick();
    
    Eris::Avatar* avatar() const
    { return m_avatar; }
private:        
// lots of slots    
    void onEntityTalk();
    void onHear(Eris::Entity* src, const Atlas::Objects::Operation::RootOperation&);
    
    void onGotEntity(Eris::Entity*);
    void onAppear(Eris::Entity*);
    
    typedef sigc::slot<void, Eris::Entity*> SpotTypeTrigger;
    
    void spotSomething(Eris::Entity* app);
    void registerTypeTrigger(Eris::TypeInfo* ty, const SpotTypeTrigger& tg);
    
    void spotTool(Eris::Entity* toolEnt);
    
    Eris::View* m_view;
    Eris::Avatar* m_avatar;
    
    std::auto_ptr<Action> m_action;
    
    typedef std::map<Eris::Entity*, int> MemoryStore;
    MemoryStore m_memory;
    
    typedef sigc::signal<void, Eris::Entity*> TypeTriggerSig;
    typedef std::map<Eris::TypeInfo*, TypeTriggerSig > TypeTriggerMap;
    TypeTriggerMap m_triggers;
};


#endif
