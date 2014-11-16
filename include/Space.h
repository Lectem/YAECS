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
        Space();
        virtual ~Space();

    protected:
    private:
        typedef vector<ComponentManager<Component>> veccm;
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


