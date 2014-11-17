#pragma once
/**
 * \file
 * \brief
 * \author Lectem
 */

#include <vector>
#include <unordered_set>
#include "ComponentManager.h"

class Space
{
    public:
        Space():lastEntityId(0){}
        ~Space()
        {
            for(BaseComponentManager* cm : cmanagers_)
                delete cm;
        };

        Entity::Id createEntity()
        {
            entities_.insert(lastEntityId);
            return lastEntityId++;
        }

        template<class CompType,class ... Args>
        bool addComponent(Entity::Id entity,Args&& ...args)
        {
            assert(entities_.find(entity) != entities_.end());
            static ComponentManager<CompType>* manager=getManager<CompType>();
            return manager->addComponent(entity,std::forward<Args>(args)...);
        }

        template<class CompType,class ... Args>
        void deleteComponent(Entity::Id entity,Args&& ...args)
        {
            assert(entities_.find(entity) != entities_.end());
            static ComponentManager<CompType>* manager=getManager<CompType>();
            return manager->deleteComponent(entity,std::forward<Args>(args)...);
        }


    protected:
    private:
        typedef vector<BaseComponentManager*> veccm;
        unordered_set<Entity::Id> entities_;
        Entity::Id lastEntityId;


        veccm cmanagers_;
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


