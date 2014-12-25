#include <iostream>
#include <chrono>
#include "Space.h"

using namespace std;
using namespace YAECS;


struct Position : public Component
{
	int _x=0, _y=0;
	Position()
	{
		//cout << "Position()" << endl;
	}
	Position(int x, int y):_x(x),_y(y)
	{
		//cout << "Position("<< _x << _y <<")" << endl;
	}
	Position(const Position & )=delete;
    ~Position()
    {
        //cout << "Position destructed" << endl;
    };
};
struct Vitesse : public Component
{
	int vx, vy;
    Vitesse(int vx=0,int vy=0):vx(vx),vy(vy)
    {
        //cout <<"Vitesse("<< vx<<","<<vy<<"). instance nb : "<<++nb<<endl;
    }
    ~Vitesse()
    {
        //cout << "Vitesse destructed. left : "<< --nb << endl;
    };
    Vitesse(const Vitesse& )=delete;
    static int nb;
};
int Vitesse::nb=0;

struct Affichable : public Component{};

class Affichage : public System
{
    public:
    Affichage(){
        //cout<<"System Affichage added to space"<<endl;
        }
    ~Affichage(){//cout<<"System Affichage removed from space"<<endl;
    }
    void update(Space& space) override
	{
		auto view = space.getEntitiesWith<Position,Affichable>();
		for (auto it = view.begin(); it != view.end();++it)
		{
			cout << "l'entite "<< *it << "a la position "<<it.getComponent<Position>()._x<<","<<it.getComponent<Position>()._y<< endl;
		}
		auto view2 = space.getEntitiesWith<Vitesse,Affichable>();
		for (auto it = view2.begin(); it != view2.end();++it)
		{
			cout << "L'entite "<< *it << "a la vitesse "<<it.getComponent<Vitesse>().vx<<","<<it.getComponent<Vitesse>().vy<< endl;
		}
	};

};

class Physique : public System
{
    public:
    Physique(){//cout<<"System Physique added to space"<<endl;
    }
    ~Physique(){//cout<<"System Physique removed from space"<<endl;
    }
    void update(Space& space) override
	{
		auto view = space.getEntitiesWith<Position,Vitesse>();
		for (auto it = view.begin(); it != view.end();++it)
		{
			assert(!it.isEnd());
		    Vitesse &v= it.getComponent<Vitesse>();
		    Position &p= it.getComponent<Position>();

		    p._x += v.vx;
		    p._y += v.vy;
		}
	};

};


int main()
{
    cout << "main()" << endl;
    Space space;
    for(int i=0;i<10000;++i)
    {
        Entity::Id ent = space.createEntity();
        space.addComponent<Position>(ent, i, i);
		space.addComponent<Vitesse>(ent, 100, 100);
		if(i<10)space.addComponent<Affichable>(ent);
        if(0)space.deleteComponent<Position>(ent);
        if(0)
        {
            space.destroyEntity(ent);
        }
    }

	cout << "space.addSystem<Physique>() : " << endl;
	space.addSystem<Physique>();
	//space.addSystem<Affichage>();
	Space space2;
	cout << "space2.addSystem<Physique>() : " << endl;
	space2.addSystem<Physique>();
	auto t1 = std::chrono::high_resolution_clock::now();
	int i;
	for (i = 0; i < 1001;i++)
        space.update();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << i << "updates took "
            << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
            << " milliseconds\n";
		
	space.deleteSystem<Physique>();
	
	space.addSystem<Affichage>();
	space.update();



    cout << "main() end" << endl;
    return 0;
}
