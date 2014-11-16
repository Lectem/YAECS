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

        ~Space()
        {
            for(BaseComponentManager* cm : cmanagers_)
                delete cm;
        };

        template<class CompType>
        ComponentManager<CompType>* getManager()
        {
            std::size_t index = ComponentsIndex<CompType>();
            if(index >= cmanagers_.size())cmanagers_.push_back(new ComponentManager<CompType>());
            return (ComponentManager<CompType>*) cmanagers_[index];
        }


    protected:
    private:
        typedef vector<BaseComponentManager*> veccm;
        unordered_set<Entity::Id> entities_;
        veccm cmanagers_;
        size_t componentTypesCount_=0;

        template<class CompType>
        size_t& ComponentsIndex()
        {
            static size_t index=componentTypesCount_++;
            return index;
        }
};


