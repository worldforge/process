#include "Character.h"
#include <sigc++/object_slot.h>
#include <Atlas/Objects/Operation.h>
#include "Action.h"
#include <iostream>

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
    av->GotCharacterEntity.connect(SigC::slot(*this, &Character::onGotEntity));
}

Character::~Character()
{
}

void Character::onGotEntity(Eris::Entity*)
{
    m_avatar->Hear.connect(SigC::slot(*this, &Character::onHear));
    
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

void Character::tick()
{
    m_view->update(); // motion prediction
    if (m_action.get()) {
        m_action->run();
        if (m_action->finished()) {
            m_action.reset();
        }
    } else
        m_action = ::Action::newRandomAction(this);
}
