#ifndef LOADTOOL_SCHEDULER_H
#define LOADTOOL_SCHEDULER_H

#include <vector>

class Schedulable
{
public:
    virtual void tick() = 0;
};

class Scheduler
{
public:
    static Scheduler* instance();

    void update();
    
    void addItem(Schedulable* s);
    void removeItem(Schedulable* s);

private:
    static Scheduler* static_instance;
    std::vector<Schedulable*> m_items;
};

#endif // of LOADTOOL_SCHEDULER_H
