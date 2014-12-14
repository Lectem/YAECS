#pragma once
/**
 * \file
 * \brief
 * \author Lectem
 */

#include <vector>
#include <unordered_set>
#include <list>
#include "ComponentManager.h"
#include "System.h"
#include <typeinfo>
class Space
{
public:
    template<class First,class ... Components>
    class View;
    Space():lastEntityId(0) {}
    ~Space()
    {

        for(BaseComponentManager* cm : cmanagers_)
            delete cm;

        for(System* sys : systems_)
            delete sys;
    };

    Entity::Id createEntity()
    {
        entities_.insert(lastEntityId);
        return lastEntityId++;
    }

    void destroyEntity(Entity::Id entity)
    {
        entities_.erase(entity);
        for(auto it=cmanagers_.begin(); it!=cmanagers_.end(); ++it)
        {
            (*it)->deleteComponent(entity);
        }
    }

    template<class CompType,class ... Args>
    bool addComponent(Entity::Id entity,Args&& ...args)
    {
        assert(entities_.find(entity) != entities_.end());
        static ComponentManager<CompType>* manager=getManager<CompType>();
        return manager->addComponent(entity,std::forward<Args>(args)...);
    }

    template<class CompType,class ... Args>
    void deleteComponent(Entity::Id entity)
    {
        assert(entities_.find(entity) != entities_.end());
        static ComponentManager<CompType>* manager=getManager<CompType>();
        return manager->deleteComponent(entity);
    }

    template<class SysType,class ... Args>
    void addSystem(Args&& ...args)
    {
        systems_.push_front(new SysType(args...));
        systemIndex<SysType>() = systems_.begin();
    }

    template<class SysType>
    void deleteSystem()
    {
        auto it=systemIndex<SysType>();
        assert(it!=systems_.end());
        delete *it;
        systems_.erase(it);
        it=systems_.end();
    }

    template<class SysType>
    list<System*>::iterator& systemIndex()
    {
        static typename list<System*>::iterator sys_index_;
        return sys_index_;
    }

    void update()
    {
        for(auto sys:systems_)
        {
            sys->update();
        }
    }

protected:
private:
    vector<BaseComponentManager*> cmanagers_;
    unordered_set<Entity::Id> entities_;
    Entity::Id lastEntityId;
    list<System*> systems_;

    size_t componentTypesCount_=0;

    template<class CompType>
    ComponentManager<CompType>* getManager()
    {
        std::size_t index = ComponentsIndex<CompType>();
        if(index >= cmanagers_.size())
        {
            cout << "test" << endl;
            cmanagers_.push_back(new ComponentManager<CompType>());
        }

        return (ComponentManager<CompType>*) cmanagers_[index];
    }

    template<class CompType>
    size_t& ComponentsIndex()
    {
        static size_t index=componentTypesCount_++;
        return index;
    }
};


template<class First,class ... Components>
class Space::View
{
    class iterator : public std::iterator<std::input_iterator_tag, Entity::Id>
    {
            Space* space_;
            tuple<ComponentManager<First>*,ComponentManager<Components>* ... > comps_;
            //Iterator on the first ComponentManager
            typename ComponentManager<First>::iterator it_first_comp;

        public:
            iterator(Space* space,bool end)
                :space_(space),comps_(space_->getManager<First>(),space->getManager<Components>()...)
                {
                    if(end)
                    {
                        it_first_comp=std::get<0>(comps_)->end();
                    }
                    else it_first_comp=std::get<0>(comps_)->begin();
                }
            iterator(const iterator& it)
                :space_(it.space_),comps_(it.comps_),it_first_comp(it.it_first_comp) {}
            iterator& operator++()
            {
++it_first_comp;
                //while(it_first_comp != std::get<0>(comps_)->end()){++it_first_comp;}
                return *this;
            }
            iterator operator++(int) {iterator tmp(*this); operator++(); return tmp;}
            bool operator==(const iterator& rhs) {return it_first_comp==rhs.it_first_comp;}
            bool operator!=(const iterator& rhs) {return it_first_comp!=rhs.it_first_comp;}
            Entity::Id operator*() {return it_first_comp->first;}
    };
    Space *space_;
  public:
    View(Space* space):space_(space){}

    iterator begin(){return iterator(space_,false);}
    iterator end(){return iterator(space_,true);}


};
