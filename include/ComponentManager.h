#pragma once
/**
 * \file
 * \brief
 * \author Lectem
 */

#include <assert.h>
#include <unordered_map>
#include <vector>

#include "Component.h"
#include "Entity.h"

#include <iostream>
    using namespace std;


#include "Pools.h"

class BaseComponentManager
{
    public:
    virtual ~BaseComponentManager(){}
};

//TODO: add a base pool class to inherit from, to hide container behaviors
template <class T,class Pool = BasicPool<T>>
class ComponentManager : public BaseComponentManager
{
public:
  template<class ...Args>
    bool addComponent(Entity::Id ent,Args&& ...args);
    void deleteComponent(Entity::Id id);
    void clear();

private:
  template<class ...Args>
    typename Pool::iterator createComponent(Args&& ...args);
    bool attachComponent(Entity::Id ent,typename Pool::iterator cp);
    typename Pool::iterator dettachComponent(Entity::Id ent);

    std::unordered_map<Entity::Id,typename Pool::iterator > linksEC_;
    Pool components_;
};


template<class T,class Pool>
template<class ...Args> typename Pool::iterator ComponentManager<T,Pool>::createComponent(Args&& ...args)
{
    return components_.emplace(args...);
}

template<class T,class Pool>
template<class ...Args> bool ComponentManager<T,Pool>::addComponent(Entity::Id ent,Args&& ...args)
{
    return attachComponent(ent,createComponent(args...));
}

template<class T,class Pool>
bool ComponentManager<T,Pool>::attachComponent(Entity::Id ent,typename Pool::iterator cp)
{
    return linksEC_.emplace(ent,cp).second;
}

template<class T,class Pool> typename Pool::iterator ComponentManager<T,Pool>::dettachComponent(Entity::Id ent)
{
    typename Pool::iterator comp=components_.end();
    auto entry=linksEC_.find(ent);
    if(entry != linksEC_.end())
    {
        comp=entry->second;
        linksEC_.erase(entry);
    }
    return comp;
}


/*Destruction of components will be done upon manager's destruction*/
template<class T,class Pool>
void ComponentManager<T,Pool>::deleteComponent(Entity::Id ent)
{
    typename Pool::iterator comp=dettachComponent(ent);
    components_.remove(comp);
}

template<class T,class Pool>
void ComponentManager<T,Pool>::clear()
{
    components_.clear();
    linksEC_.clear();
}
