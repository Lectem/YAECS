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
struct BComponent : Component{};

int main()
{
    ComponentManager<AComponent> cma;
    cma.addComponent(0);
    cma.addComponent(1);
    cma.addComponent(2);
    cma.deleteComponent(0);
    cma.deleteComponent(1);
    cma.deleteComponent(1);
    cma.deleteComponent(2);

    return 0;
}
