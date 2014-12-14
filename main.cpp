#include <iostream>

using namespace std;
#include "Space.h"


struct AComponent : public Component
{
    AComponent()
    {
        cout<<"AComponent()"<<endl;
    }
    ~AComponent()
    {
        cout << "AComponent destructed" << endl;
    };
};
struct BComponent : public Component
{
    BComponent(int i,char c)
    {
        cout <<"BComponent("<< i<<","<<c<<"). instance nb : "<<++nb<<endl;
    }
    ~BComponent()
    {
        cout << "BComponent destructed. left : "<< --nb << endl;
    };
    static int nb;
};

int BComponent::nb=0;

class SystemB : public System
{
    public:
    SystemB(){cout<<"System added to space"<<endl;}
    ~SystemB(){cout<<"System removed from space"<<endl;}
    virtual void update() override {};

};


int main()
{
    Space space;
    for(int i=0;i<10;++i)
    {
        Entity::Id ent = space.createEntity();
        space.addComponent<AComponent>(ent);
        space.addComponent<BComponent>(ent,42,'a');
        if(i%3)space.deleteComponent<BComponent>(ent);
        if(! (i%5))
        {
            cout << "space.destroyEntity(ent)" << endl;
            space.destroyEntity(ent);
        }
    }
    cout << "space.addSystem<SystemB>()" << endl;
    space.addSystem<SystemB>();
    cout << "space.deleteSystem<SystemB>()" << endl;
    space.deleteSystem<SystemB>();
    Space::View<AComponent> v(&space);
    for(Entity::Id ent : v){cout << "Ent:"<<ent <<endl; }
    cout << "main() end" << endl;
    return 0;
}
