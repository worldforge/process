#include "Character.h"
#include <sigc++/object_slot.h>
#include <Atlas/Objects/Operation.h>
#include "Action.h"
#include <iostream>
#include <Eris/Entity.h>
#include <Eris/TypeService.h>
#include <Eris/Connection.h>

using std::cout;
using std::endl;
using std::cerr;

using namespace Atlas::Objects::Operation;
using Atlas::Objects::smart_dynamic_cast;
using Atlas::Objects::Root;

Character::Character(Eris::Avatar* av) :
    m_avatar(av)
{
    m_view = av->getView();
    av->GotCharacterEntity.connect(sigc::mem_fun(*this, &Character::onGotEntity));
    m_view->Appearance.connect(sigc::mem_fun(*this, &Character::onAppear));
}

Character::~Character()
{
}

void Character::onGotEntity(Eris::Entity*)
{
    m_avatar->Hear.connect(sigc::mem_fun(*this, &Character::onHear));
    
    Eris::TypeService* tysv = m_avatar->getConnection()->getTypeService();

    registerTypeTrigger(tysv->getTypeByName("axe"), sigc::mem_fun(*this, &Character::spotTool));
    registerTypeTrigger(tysv->getTypeByName("pickaxe"), sigc::mem_fun(*this, &Character::spotTool));
    registerTypeTrigger(tysv->getTypeByName("tinderbox"), sigc::mem_fun(*this, &Character::spotTool));
    registerTypeTrigger(tysv->getTypeByName("scythe"), sigc::mem_fun(*this, &Character::spotTool));
    registerTypeTrigger(tysv->getTypeByName("shovel"), sigc::mem_fun(*this, &Character::spotTool));

    // start sending actions
    
    Scheduler::instance()->addItem(this);
}

void Character::onHear(Eris::Entity* src, const Atlas::Objects::Operation::RootOperation& arg)
{
    Talk tk = smart_dynamic_cast<Talk>(arg);
    if (!tk.isValid()) return;
    
    Root args = tk->getArgs().front();
    if (!args->hasAttr("say")) return; // weird
    std::string m = args->getAttr("say").asString();
    
    if (m == "Shitcock!") {
        cout << "doing retort" << endl;
        m_avatar->say("I fucked your dad!");
    }
}

void Character::onAppear(Eris::Entity* app)
{
    m_memory[app] = 0;
    
    spotSomething(app);
}

void Character::spotSomething(Eris::Entity* app)
{
    if (m_triggers.count(app->getType())) {
        // we have a trigger, run it
        m_triggers[app->getType()](app);
    }
}

void Character::registerTypeTrigger(Eris::TypeInfo* ty, const SpotTypeTrigger& tg)
{
    assert(ty);
    m_triggers[ty].connect(tg);
}

void Character::tick()
{
    m_view->update(); // motion prediction
    
    MemoryStore::iterator it;
    for (it = m_memory.begin(); it != m_memory.end(); ++it) {
        ++(it->second);
    }
    
    if (m_action.get()) {
        m_action->run();
        if (m_action->finished()) {
            m_action.reset();
        }
    } else
        m_action = ::Action::newRandomAction(this);
}

void Character::spotTool(Eris::Entity* toolEnt)
{
    cout << "entity spotted tool " << toolEnt->getId() << endl;
}
