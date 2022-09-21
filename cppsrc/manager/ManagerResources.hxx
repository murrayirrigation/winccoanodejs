#ifndef ManagerRESOURCES_H
#define ManagerRESOURCES_H

#include <Resources.hxx>

class ManagerResources : public Resources
{
  public:
    // These functions initializes the manager
    static void init(int &argc, char *argv[]);

    // Read the config section
    static PVSSboolean readSection();
};

#endif
