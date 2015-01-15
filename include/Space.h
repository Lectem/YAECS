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


				template<size_t N = 0>
				bool hasComponent(Entity::Id ent)
				{
					get<N>(comps_iters_) = get<N>(comps_)->getAttachedComponent(ent);
					return std::get<N>(comps_iters_) != std::get<N>(comps_)->end();
				}
				template<size_t N>
				typename std::enable_if< N == 1 + sizeof...(Components), bool>::type hasComponents(Entity::Id)
				{
					return true;
				}

				template<size_t N>
				typename std::enable_if< N <= sizeof...(Components), bool>::type hasComponents(Entity::Id ent)
				{
					return hasComponent<N>(ent) && hasComponents<N + 1>(ent);
				}

			public:
				iterator(Space* space, bool end)
						:space_(space)
				{
					comps_ = make_tuple(space_->getManager<FirstComp>(), space->getManager<Components>()...);
					if (end)get<0>(comps_iters_) = std::get<0>(comps_)->end();
					else{
						get<0>(comps_iters_) = std::get<0>(comps_)->begin();
						if(get<0>(comps_iters_) != get<0>(comps_)->end() && !hasComponents<1>(getEnt())) operator++();
					}
				};
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
				}
			};
			iterator begin() { return iterator(space_, false); }
			iterator end() { return iterator(space_, true); }

		private:
			Space *space_;
			View(Space* space) :space_(space) {}
			friend class Space;
		};

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
		{/*
		    auto & sysind =systemIndex<SysType>();
			if(sysind == systems_.end() )
            {
                systems_.push_front(new SysType(args...));
                sysind = systems_.begin();
            }*/
            size_t index = systemIndex<SysType>();
			if (index >= systems_.size())
			{
				//necessary because comp indexes are the same for all space instances...
				systems_.resize(index+1,nullptr);
			}
			if(systems_[index] == nullptr)systems_[index]=(new SysType(args...));
		}

		template<class SysType>
		void deleteSystem()
		{
			size_t index=systemIndex<SysType>();
			if(systems_[index] != nullptr)
            {
                delete systems_[index];
                systems_[index]=nullptr;
            }
			/*auto & it = systemIndex<SysType>();
			assert(it != systems_.end());
			delete *it;
			systems_.erase(it);
			it = systems_.end();*/

		}

		template<class ...T>
		View<T...> getEntitiesWith()
            { return View<T...>(this); }

		void update()
		{
			for (System* sys : systems_)
			{
			    if(sys != nullptr)
                {
                    sys->update(*this);
                }
			}
		}


		template<class C>
		bool hasComponent(Entity::Id ent)
		{
			return getManager<C>()->hasAttachedComponent(ent);
		}

		template<class C>
		C& getComponent(Entity::Id ent)
		{
			return *getManager<C>()->getAttachedComponent(ent)->second;
		}

	protected:
	private:
		vector<BaseComponentManager*> cmanagers_;
		unordered_set<Entity::Id> entities_;
		Entity::Id lastEntityId;
		vector<System*> systems_;

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
		//list<System*>::iterator& systemIndex()
		size_t systemIndex()
		{
			//static typename std::list<System*>::iterator sys_index_=systems_.end();
			static size_t sys_index_=systemTypesCount_++;
			return sys_index_;
		}
	};


}
