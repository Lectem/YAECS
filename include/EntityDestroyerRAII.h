#pragma once

#include "Entity.h"
namespace YAECS {

    class EntityDestroyerRAII {
        std::vector <Entity::Id> _toDestroy;
        Space &_s;
    public:
        EntityDestroyerRAII(Space &s) :
                _s(s) {
        }


        ~EntityDestroyerRAII() {
            for (Entity::Id id : _toDestroy) {
                _s.destroyEntity(id);
            }
        }

        void add(Entity::Id id)
        {
            _toDestroy.push_back(id);
        }

    };

}