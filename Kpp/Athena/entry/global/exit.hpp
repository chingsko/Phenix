#include "PrecompiledHeader.hpp"
#include "kv/entry/IncludeAll.hpp"

// exit
KV_AddGlobalEntry("exit")
{
    cout << boldfore << green << "wish you have a nice day, bye bye!" << reset << endl;
    Manager.Exit();
};