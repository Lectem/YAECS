#include <iostream>
#include <chrono>
#include "Space.h"

using namespace std;
using namespace YAECS;


struct Position : public Component
{
	int _x, _y;
	//You can define any constructor you need
	Position(int x, int y)
			:_x(x),_y(y) {}
	Position(const Position & )=delete; // This is guarantee that you won't copy your component
};
struct Speed : public Component
{
	int vx, vy;
	Speed(int vx=0,int vy=0)
			:vx(vx),vy(vy){}
	Speed(const Speed& )=delete;
};

struct Displayable : public Component{};

class Display : public System
{
public:
	Display(){
		cout<<"System Display added to space"<<endl;
	}
	~Display(){
		cout<<"System Display removed from space"<<endl;
	}
	void update(Space& space) override // Make sure you override the virtual method update
	{
		//For each entity made of Displayable and Position components
		auto view = space.getEntitiesWith<Displayable,Position>();
		for (auto it = view.begin(); it != view.end();++it)
		{
			// it is a "Handle"
			// *it will only give you the entity ID, but you can also access the components THAT YOU ASKED FOR easily !
			// \code{.cpp} Speed &speed = it.getComponent<Speed>(); \endcode will NOT compile
			// if you asked only for \code{.cpp}getEntitiesWith<Displayable,Position>()\endcode !
			// So don't worry, you'll be safe.
			Position &pos = it.getComponent<Position>();
			cout << "Entity "<< *it << " is at ( "<< pos._x<<","<< pos._y<<")"<< endl;
		}
		// Same for Displayable and Speed.
		// Working on entities that have Displayable, Position and Speed would be faster
		auto view2 = space.getEntitiesWith<Displayable,Speed>();
		for (auto it = view2.begin(); it != view2.end();++it)
		{
			Speed &spd = it.getComponent<Speed>();
			cout << "Entity "<< *it << " has speed "<< spd.vx<<","<< spd.vy<< endl;
		}
	};

};

class Physics : public System
{
public:
	Physics(){
		cout<<"Physics system added to space"<<endl;
	}
	~Physics(){
		cout<<"Physics system removed from space"<<endl;
	}
	void update(Space& space) override
	{
		auto view = space.getEntitiesWith<Position,Speed>();
		for (auto it = view.begin(); it != view.end();++it)
		{
			//Get references to the components we need
			Speed &v= it.getComponent<Speed>();
			Position &p= it.getComponent<Position>();
			//Update the position according to the speed
			p._x += v.vx;
			p._y += v.vy;
			// We're done !
		}
	};

};


int main()
{
	//We are working with spaces. Each space has its own Entities and Systems
	Space space;

	// Let's create a few entities
	for(int i=0;i<10000;++i)
	{
		// You mustn't create entities yourself (it has no meaning anyway)
		// An entity is modified through its ID or views (take a look at the systems)
		Entity::Id ent = space.createEntity();

		//Let's add a Position and a Speed to our newly created entity
		// Note that addComponent will deduce your component constructor based on the arguments
		space.addComponent<Position>(ent, i, i);
		space.addComponent<Speed>(ent, 100, 100);
		//Only display the 10 first entities, we don't want to see everything on cout...
		if(i<10)space.addComponent<Displayable>(ent);
	}

	// Add our systems to the space
	space.addSystem<Physics>();
	space.addSystem<Display>();

	// Basic performance test
	auto t1 = std::chrono::high_resolution_clock::now();
	int i;

	// Update the space 1000 times !
	// It will call all the registered systems update method
	// Note : The order might not be the same as the one you added the systems
	for (i = 0; i < 1000;i++)
		space.update();


	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << i << "updates took "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
			<< " milliseconds\n";

	// Let's delete a system !
	space.deleteSystem<Physics>();

	// If a system was already added to your space, nothing happens
	// (though you should never rely on this kind of behavior...)
	space.addSystem<Display>();

	// Let's only display our entities (since we removed the Physics system)
	space.update();

	//Nothing to free ! Everything is done by the space destructor.
	return 0;
}
