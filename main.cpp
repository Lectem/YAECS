#include <iostream>

using namespace std;
#include "Space.h"
#include "Component.h"
#include "ComponentManager.h"
#include "Pools.h"
#include <map>
struct AComponent : public Component
{
    AComponent()
    {
        //cout<<"AComponent()"<<endl;
    }
    ~AComponent()
    {
        //cout << "AComponent destructed" << endl;
    };
};
struct BComponent : public Component
{
    BComponent(int i,char c)
    {
        //cout <<"BComponent("<< i<<","<<c<<")"<<endl;
    }
    ~BComponent()
    {
        //cout << "BComponent destructed" << endl;
    };
};

int main()
{
    Space space;
    for(int i=0;i<10000000;++i)
    {
        Entity::Id ent = space.createEntity();
        space.addComponent<AComponent>(ent);
        space.addComponent<BComponent>(ent,42,'a');
        if(i%2)space.deleteComponent<BComponent>(ent);
        if(!i%5)space.destroyEntity(ent);
    }


    return 0;
}
