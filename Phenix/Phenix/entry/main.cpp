#include "PrecompiledHeader.hpp"
#include "global/_unity_build_.hpp"
#include "kv/predef/Type.hpp"
#include "kv/entry/IncludeAll.hpp"

#include "fmt/format.h"

using namespace std;
using namespace termcolor;
using namespace kv::entry;

int main(int argc, char ** argv)
{
    autox mgr  = GetManager();
    autox path = ExtractPathInfo(argv[0]);

    set_title(get<1>(path));

    mgr.SetWorkspace(__FILE__);
    mgr.SetBootPath(argv[0]);

    _putenv((string("PWD=") + get<0>(path)).c_str());

    switch (argc)
    {
    default:
    case 1:
        mgr.SetCurrent(get<0>(ExtractPathInfo(__FILE__)));
        break;
    case 2:
        mgr.SetCurrent(argv[1]);
        break;
    }

    mgr.Run();

    return 0;
}