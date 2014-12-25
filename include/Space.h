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

namespace YAECS {

	class Space
	{
	public:
		template<class First, class ... Components>
		class View;
		Space() :lastEntityId(0) {}
		~Space()
		{
			for (BaseComponentManager* cm : cmanagers_)
				if(cm)	delete cm;
			for (System* sys : systems_)
				if(sys)	delete sys;
		};

		Entity::Id createEntity()
		{
			entities_.insert(lastEntityId);
			return lastEntityId++;
		}

		void destroyEntity(Entity::Id entity)
		{
			entities_.erase(entity);
			for (auto it = cmanagers_.begin(); it != cmanagers_.end(); ++it)
			{
				(*it)->deleteComponent(entity);
			}
		}

		template<class CompType, class ... Args>
		bool addComponent(Entity::Id entity, Args&& ...args)
		{
			assert(entities_.find(entity) != entities_.end());
			static ComponentManager<CompType>* manager = getManager<CompType>();
			return manager->addComponent(entity, std::forward<Args>(args)...);
		}

		template<class CompType, class ... Args>
		void deleteComponent(Entity::Id entity)
		{
			assert(entities_.find(entity) != entities_.end());
			static ComponentManager<CompType>* manager = getManager<CompType>();
			return manager->deleteComponent(entity);
		}

		template<class SysType, class ... Args>
		void addSystem(Args&& ...args)
		{
		    auto & sysind =systemIndex<SysType>();
			if(sysind == systems_.end() )
            {
                systems_.push_front(new SysType(args...));
                sysind = systems_.begin();
            }
		}

		template<class SysType>
		void deleteSystem()
		{
			auto & it = systemIndex<SysType>();
			assert(it != systems_.end());
			delete *it;
			systems_.erase(it);
			it = systems_.end();
		}

		template<class ...T>
		typename Space::View<T...> getEntitiesWith()
            { return typename Space::View<T...>(this); }

		void update()
		{
			for (auto sys : systems_)
			{
				sys->update(*this);
			}
		}

	protected:
	private:
		vector<BaseComponentManager*> cmanagers_;
		unordered_set<Entity::Id> entities_;
		Entity::Id lastEntityId;
		list<System*> systems_;

		size_t componentTypesCount_ = 0;

		template<class CompType>
		ComponentManager<CompType>* getManager()
		{
			std::size_t index = ComponentsIndex<CompType>();
			if (index >= cmanagers_.size())
			{
				//necessary because comp indexes are the same for all space instances...
				cmanagers_.resize(index+1,nullptr);
				cmanagers_[index]=(new ComponentManager<CompType>());
			}
			
			return (ComponentManager<CompType>*) cmanagers_[index];
		}

		template<class CompType>
		size_t& ComponentsIndex()
		{
			static size_t index = componentTypesCount_++;
			return index;
		}

		template<class SysType>
		list<System*>::iterator& systemIndex()
		{
			static typename std::list<System*>::iterator sys_index_=systems_.end();
			return sys_index_;
		}
	};


	template<class FirstComp, class ... Components>
	class Space::View
	{
		class iterator : public std::iterator < std::input_iterator_tag, Entity::Id >
		{
			Space* space_;
			tuple<ComponentManager<FirstComp>*, ComponentManager<Components>* ... > comps_;
			tuple<typename ComponentManager<FirstComp>::iterator, typename ComponentManager<Components>::iterator ...> comps_iters_;


			template<std::size_t N = 0>
			bool getComponent(Entity::Id ent)
			{
				get<N>(comps_iters_) = get<N>(comps_)->getAttachedComponent(ent);
				return std::get<N>(comps_iters_) != std::get<N>(comps_)->end();
			}
			template<std::size_t N>
			typename std::enable_if< N == 1 + sizeof...(Components), bool>::type hasComponents(Entity::Id)
			{
				return true;
			}

			template<std::size_t N>
			typename std::enable_if< N <= sizeof...(Components), bool>::type hasComponents(Entity::Id ent)
			{
				return getComponent<N>(ent) && hasComponents<N + 1>(ent);
			}

		public:

			iterator(Space* space, bool end)
				:space_(space), comps_(space_->getManager<FirstComp>(), space->getManager<Components>()...)
			{
				if (end)get<0>(comps_iters_) = std::get<0>(comps_)->end();
				else{
                        get<0>(comps_iters_) = std::get<0>(comps_)->begin();
                        if(!hasComponents<1>(getEnt())) operator++();
				}
			}
			iterator(const iterator& it)
				:space_(it.space_), comps_(it.comps_),comps_iters_(it.comps_iters_){}

			iterator& operator++()
			{
				do ++get<0>(comps_iters_);
				while (get<0>(comps_iters_) != std::get<0>(comps_)->end()
					&& !hasComponents<1>(getEnt()));
				return *this;
			}
			iterator operator++(int) { iterator tmp(*this); operator++(); return tmp; }
			bool operator==(const iterator& rhs) { return get<0>(comps_iters_) == get<0>(rhs.comps_iters_); }
			bool operator!=(const iterator& rhs) { return get<0>(comps_iters_) != get<0>(rhs.comps_iters_); }
			Entity::Id operator*() { return getEnt(); }
			bool isEnd() { return get<0>(comps_iters_) == std::get<0>(comps_)->end(); }
			Entity::Id getEnt(){return get<0>(comps_iters_)->first;}

			template<class T>
            T& getComponent()
			{
			    return *tupleGet<typename ComponentManager<T>::iterator>(comps_iters_)->second;
			    //return *(space_->getManager<T>()->getAttachedComponent(get<0>(comps_iters_)->first))->second;
			}
		};
		Space *space_;
		View(Space* space) :space_(space) {}
		friend class Space;
	public:
		iterator begin() { return iterator(space_, false); }
		iterator end() { return iterator(space_, true); }
	};
}
