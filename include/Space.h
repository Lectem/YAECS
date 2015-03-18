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
#include "TupleGet.h"
#include <map>

namespace YAECS {

	/**
	* Space
	* |- View
	*   |- iterator
	*/


	class Space
	{
	public:

		template<class FirstComp, class ... Components>
		class View
		{
		public:
			class iterator : public std::iterator < std::input_iterator_tag, Entity::Id >
			{
				friend class View;
				Space* space_;
				tuple<ComponentManager<FirstComp>*, ComponentManager<Components>* ... > comps_;
				tuple<typename ComponentManager<FirstComp>::iterator, typename ComponentManager<Components>::iterator ...> comps_iters_;

                //TODO:There are probably (actually i'm sure there are) better ways to do this
				template<size_t N = 0>
				bool hasComponent()
				{
					get<N>(comps_iters_) = get<N>(comps_)->getAttachedComponent(getEnt());
					return std::get<N>(comps_iters_) != std::get<N>(comps_)->end();
				}
				template<size_t N>
				typename std::enable_if< N == 1 + sizeof...(Components), bool>::type hasComponents()
				{
					return true;
				}

				template<size_t N>
				typename std::enable_if< N <= sizeof...(Components), bool>::type hasComponents()
				{
					return hasComponent<N>() && hasComponents<N + 1>();
				}

			public:
				iterator(Space* space, bool end)
						:space_(space)
				{
					comps_ = make_tuple(space_->getManager<FirstComp>(), space->getManager<Components>()...);
					if (end)get<0>(comps_iters_) = std::get<0>(comps_)->end();
					else{
						get<0>(comps_iters_) = std::get<0>(comps_)->begin();
						if(get<0>(comps_iters_) != get<0>(comps_)->end() && !hasComponents<1>()) operator++();
					}
				};
				iterator(const iterator& it)
						:space_(it.space_), comps_(it.comps_),comps_iters_(it.comps_iters_){}

				iterator& operator++()
				{
					do ++get<0>(comps_iters_);
					while (get<0>(comps_iters_) != std::get<0>(comps_)->end()
							&& !hasComponents<1>());
					return *this;
				}
				iterator operator++(int) =delete;//{ iterator tmp(*this); operator++(); return tmp; }
				bool operator==(const iterator& rhs) { return get<0>(comps_iters_) == get<0>(rhs.comps_iters_); }
				bool operator!=(const iterator& rhs) { return get<0>(comps_iters_) != get<0>(rhs.comps_iters_); }
				Entity::Id operator*() { return getEnt(); }
				bool isEnd() { return get<0>(comps_iters_) == std::get<0>(comps_)->end(); }
				Entity::Id getEnt(){return get<0>(comps_iters_)->first;}

				template<class T>
				T& getComponent()
				{
					return *tupleGet<typename ComponentManager<T>::iterator>(comps_iters_)->second;
				}
                /////////////////////////////////////////
                ////// Space::View::iterator end ////////
                /////////////////////////////////////////
			};
			iterator begin() {newIter =true;
                return iterator(space_, false); }
			iterator end() { if(newIter) {
                    endIter = iterator(space_, true);
                    newIter = false;
                }
                return endIter; }

		private:
			Space *space_;
			View(Space* space) :space_(space),endIter(end()) {}
			friend class Space;
            bool newIter = true;
            iterator endIter;
            ///////////////////////////////////////
            ////////// Space::View end ////////////
            ///////////////////////////////////////
		};


		Space() :lastEntityId(0) {}
		~Space()
		{
			for (BaseComponentManager* cm : cmanagers_)
				if(cm)	delete cm;
			for (System* sys : systems_)
				if(sys)	delete sys;
		};

		Entity::Id createEntity();

		void destroyEntity(Entity::Id entity);

        size_t getNbEntities();

		template<class CompType, class ... Args>
		bool addComponent(Entity::Id entity, Args&& ...args);

		template<class CompType, class ... Args>
		void deleteComponent(Entity::Id entity);

        /**
        * \brief Adds a new System of class SysType to the Space. Does nothing if the System already exists in this Space
        *
        * Adds a new System of class SysType to the Space. Does nothing if the System already exists in this Space
        * Note that the lowest priority will be set (as in using minimum value so that it will be executed last).
        * See setSystemPriority() for more information about priorities.
        *
        * \param args Arguments matching those of one of the SysType constructor
        */
		template<class SysType, class ... Args>
		void addSystem(Args&& ...args);
		template<class SysType>
		System* getSystem();

        /**
        * \brief Sets a system priority.
        *
        * The Space::update method calls all the systems in the ascending order of priorities. It means that a System with
        * priority 0 will be executed before a System with priority 42. Note that in case of multiple Systems having the
        * same priority, the order to be expected is the one of the insertions/priority modifications.
        *
        * Note that it will also unpause the System.
        *
        * \param prio the new priority of the System
        */
        template<class SysType>
		void setSystemPriority(int prio);

        /**
        * \brief Pauses the SysType System. It will not be called by the Space::update method
        */
        template<class SysType>
		void pauseSystem();

        /**
        * \brief Unpauses the SysType System. It will called by the Space::update method
        *
        * While this will actually unpause a System, it will also update its priority.
        * You should expect an unpaused System to be executed after the other Systems of same priority.
        */
        template<class SysType>
		void unpauseSystem();
		template<class SysType>
		void deleteSystem();

		template<class ...T>
		View<T...> getEntitiesWith();

        /**
        * \brief Updates all the Systems.
        *
        * The Space::update method calls all the systems in the ascending order of priorities. It means that a System with
        * priority 0 will be executed before a System with priority 42. Note that in case of multiple Systems having the
        * same priority, the order to be expected is the one of the insertions/priority modifications.
        */
		void update();


        template<class C,class ...CTail>
		bool hasComponent(Entity::Id ent);

		template<class C>
		C& getComponent(Entity::Id ent);

	private:
		vector<BaseComponentManager*> cmanagers_;
		unordered_set<Entity::Id> entities_;
		Entity::Id lastEntityId;
		vector<System*> systems_;
        vector<size_t > systemsPriorities_;
        multimap<int,size_t > systemsOrder_;

		size_t componentTypesCount_ = 0;
		size_t systemTypesCount_ = 0;

		template<class CompType>
		ComponentManager<CompType>* getManager()
		{
			size_t index = ComponentsIndex<CompType>();
			if (index >= cmanagers_.size())
			{
				//necessary because comp indexes are the same for all space instances...
				cmanagers_.resize(index+1,nullptr);
			}
			if(cmanagers_[index] == nullptr)cmanagers_[index]=(new ComponentManager<CompType>());

			return (ComponentManager<CompType>*) cmanagers_[index];
		}

		template<class CompType>
		size_t ComponentsIndex()
		{
			static size_t index = componentTypesCount_++;
			return index;
		}

		template<class SysType>
		size_t systemIndex()
		{
			static size_t sys_index_=systemTypesCount_++;
			return sys_index_;
		}

        void removeSystemFromPriorityList(size_t index);
	};


    template <>
    bool Space::hasComponent<void>(Entity::Id)
    {
        return true;
    }
    Entity::Id Space::createEntity()
    {
        entities_.insert(lastEntityId);
        return lastEntityId++;
    }

    void Space::destroyEntity(Entity::Id entity)
    {
        auto it =entities_.find(entity);
        if( it != entities_.end())
        {
            entities_.erase(it);
            for (auto it = cmanagers_.begin(); it != cmanagers_.end(); ++it)
            {
                (*it)->deleteComponent(entity);
            }
        }
    }

    void Space::update()
    {
        for( pair<int,int> sysIndex : systemsOrder_)
        {
            if(systems_[sysIndex.second] != nullptr)
            {
                systems_[sysIndex.second]->update(*this);
            }
        }
    }

    size_t Space::getNbEntities()
    {
        return entities_.size();
    }

    template<class CompType, class ... Args>
    bool Space::addComponent(Entity::Id entity, Args&& ...args)
    {
        assert(entities_.find(entity) != entities_.end());
        static ComponentManager<CompType>* manager = getManager<CompType>();
        return manager->addComponent(entity, std::forward<Args>(args)...);
    }

    template<class CompType, class ... Args>
    void Space::deleteComponent(Entity::Id entity)
    {
        assert(entities_.find(entity) != entities_.end());
        static ComponentManager<CompType>* manager = getManager<CompType>();
        return manager->deleteComponent(entity);
    }

    template<class SysType, class ... Args>
    void Space::addSystem(Args&& ...args)
    {
        size_t index = systemIndex<SysType>();
        if (index >= systems_.size())
        {
            //necessary because comp indexes are the same for all space instances...
            systems_.resize(index+1,nullptr);
            systemsPriorities_.resize(index+1,0);
        }
        if(systems_[index] == nullptr)
        {
            systems_[index]=(new SysType(args...));
            if(systemsOrder_.empty())systemsPriorities_[index]=0;
            else systemsPriorities_[index]=systemsOrder_.rbegin()->first + 1 ;
            systemsOrder_.insert(make_pair(systemsPriorities_[index], index));
        }
    }


    template<class SysType>
    System* Space::getSystem()
    {
        size_t index=systemIndex<SysType>();
        if(index < systems_.size() && systems_[index] != nullptr)
        {
            return systems_[index];
        }
        return nullptr;
    }


    template<class SysType>
    void Space::setSystemPriority(int prio)
    {
        size_t index=systemIndex<SysType>();
        if(index < systemsPriorities_.size())
        {
            removeSystemFromPriorityList(index);
            systemsOrder_.insert(make_pair(prio, index));
        }
    }

    void Space::removeSystemFromPriorityList(size_t index)
    {
        auto ret = systemsOrder_.equal_range(systemsPriorities_[index]);
        for (auto it = ret.first; it != ret.second; ++it)
        {
            if (it->second == index) {
                systemsOrder_.erase(it);
                break;
            }
        }
    }


    template<class SysType>
    void Space::pauseSystem()
    {
        size_t index=systemIndex<SysType>();
        if(index < systemsPriorities_.size())
        {
            removeSystemFromPriorityList(index);
        }
    }


    template<class SysType>
    void Space::unpauseSystem()
    {
        size_t index=systemIndex<SysType>();
        if(index < systemsPriorities_.size())
        {
            removeSystemFromPriorityList(index);
            systemsOrder_.insert(make_pair(systemsPriorities_[index], index));
        }
    }



    template<class SysType>
    void Space::deleteSystem()
    {
        size_t index=systemIndex<SysType>();
        if(index < systems_.size() && systems_[index] != nullptr)
        {
            delete systems_[index];
            systems_[index]=nullptr;
            removeSystemFromPriorityList(systemsPriorities_[index]);
            systemsPriorities_[index]=0;
        }
    }

    template<class ...T>
    Space::View<T...> Space::getEntitiesWith()
    { return Space::View<T...>(this); }

    void update();


    template<class C,class ...CTail>
    bool Space::hasComponent(Entity::Id ent)
    {
        return getManager<C>()->hasAttachedComponent(ent) && hasComponent< CTail... , void>(ent);
    }

    template<class C>
    C& Space::getComponent(Entity::Id ent)
    {
        return *getManager<C>()->getAttachedComponent(ent)->second;
    }

}
#include "EntityDestroyerRAII.h"
