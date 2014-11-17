#include <iostream>

using namespace std;
#include "Space.h"
#include "Component.h"
#include "ComponentManager.h"
#include "Pools.h"

struct AComponent : Component
{
    ~AComponent()
    {
        cout << "AComponent destructed" << endl;
    };
};
struct BComponent : Component
{
    BComponent(int i){ cout <<"BComponent("<< i<<")"<<endl;}
    ~BComponent()
    {
        cout << "BComponent destructed" << endl;
    };
};

int main()
{
    Space space;
    Entity::Id ent = space.createEntity();
    space.addComponent<AComponent>(ent);
    space.addComponent<AComponent>(ent);
    space.addComponent<AComponent>(ent);
    space.addComponent<BComponent>(ent,42);
    return 0;
}
