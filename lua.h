#ifndef LUA_H
#define LUA_H
#include "includes.h"

class LuaExecutor
{
    AppGlobals*appGlobals;
    void*luaState;
public:
    LuaExecutor(AppGlobals*appGlobals);
    void execute(QString luaScript, WFExecutionContext*executionContext, WFNodeHandler*wfNodeHandler);
};

#endif // LUA_H
