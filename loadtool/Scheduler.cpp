#include "Scheduler.h"
#include <cassert>
#include <algorithm>

Scheduler* Scheduler::static_instance = NULL;

Scheduler* Scheduler::instance()
{
    if (!static_instance) static_instance = new Scheduler();
    return static_instance;
}

void Scheduler::update()
{
    for (unsigned int it=0; it < m_items.size(); ++it) {
        m_items[it]->tick();
    }
}

void Scheduler::addItem(Schedulable* s)
{
    assert(s);
    m_items.push_back(s);
}

void Scheduler::removeItem(Schedulable* s)
{
    assert(s);
    std::vector<Schedulable*>::iterator it = 
        std::find(m_items.begin(), m_items.end(), s);
        
    assert(it != m_items.end());
    m_items.erase(it);
}
