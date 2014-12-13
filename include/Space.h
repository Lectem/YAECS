#pragma once
/**
 * \file
 * \brief
 * \author Lectem
 */

#include <vector>
#include <unordered_set>
#include "ComponentManager.h"
#include "System.h"

class Space
{
    public:
        Space():lastEntityId(0){}
        ~Space()
        {

            for(BaseComponentManager* cm : cmanagers_)
                delete cm;

            for(System* sm : systems_)
                delete sm;
        };

        Entity::Id createEntity()
        {
            entities_.insert(lastEntityId);
            return lastEntityId++;
        }

        void destroyEntity(Entity::Id entity)
        {
            entities_.erase(entity);
            for(auto it=cmanagers_.begin();it!=cmanagers_.end();++it)
            {
                (*it)->deleteComponent_no_type_(entity);
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
        bool addSystem(Args&& ...args)
        {
            return systems_.emplace_back(new SysType(args...));
        }

        template<class SysType>
        void deleteSystem()
        {

        }


    protected:
    private:
        vector<BaseComponentManager*> cmanagers_;
        unordered_set<Entity::Id> entities_;
        Entity::Id lastEntityId;
        vector<System*> systems_;

        size_t componentTypesCount_=0;

        template<class CompType>
        ComponentManager<CompType>* getManager()
        {
            std::size_t index = ComponentsIndex<CompType>();
            if(index >= cmanagers_.size())cmanagers_.push_back(new ComponentManager<CompType>());
            return (ComponentManager<CompType>*) cmanagers_[index];
        }

        template<class CompType>
        size_t& ComponentsIndex()
        {
            static size_t index=componentTypesCount_++;
            return index;
        }
};


