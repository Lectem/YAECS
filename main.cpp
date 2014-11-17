#include <iostream>

using namespace std;
#include "Space.h"
#include "Component.h"
#include "ComponentManager.h"
#include "Pools.h"

struct AComponent : Component{
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
    ComponentManager<AComponent>* cma= space.getManager<AComponent>();
    cma->addComponent(0);
    cma->addComponent(0);
    cma->addComponent(1);
    cma->addComponent(0);
    ComponentManager<BComponent>* cmb= space.getManager<BComponent>();
    cmb->addComponent(0,1);
    return 0;
}
