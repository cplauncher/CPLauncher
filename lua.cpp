#include "lua.h"
#define LUA_IMPL
#include <minilua/minilua.h>

void*getUserDataFromGlobalVar(lua_State *state, const char*name) {
    lua_getglobal(state, name);
    void *p=lua_touserdata(state, -1);
    lua_pop(state, 1);

    return p;
}

int getVar(lua_State *state) {
    int argsCount = lua_gettop (state);
    if(argsCount==0) {
        qDebug()<<"lua \"var\" function expects string argument";
        lua_pushstring(state, "");
        return 1;
    }

    QString varName=QString(lua_tostring(state, 1));
    WFExecutionContext*executionContext = (WFExecutionContext*)getUserDataFromGlobalVar(state, "executionContext");
    QString input="";
    if(executionContext->variables.contains(varName)) {
        input=executionContext->variables[varName].toString();
    }

    QByteArray utf8 = input.toUtf8();
    lua_pushstring(state, utf8.constData());
    return 1;
}

int output(lua_State *state) {
    int argsCount = lua_gettop (state);
    if(argsCount!=2) {
        qDebug()<<"lua \"output\" function expects 2 arguments(port:int, data:string)";
        return 0;
    }
    if(!lua_isinteger(state, 1)) {
        qDebug()<<"lua \"output\" function expects first arguments port:int";
        return 0;
    }

    QString data = lua_tostring(state,2);
    int port = lua_tointegerx(state,1,NULL);
    WFExecutionContext*executionContext = (WFExecutionContext*)getUserDataFromGlobalVar(state, "executionContext");
    WFNodeHandler*nodeHandler = (WFNodeHandler*)getUserDataFromGlobalVar(state, "wfNodeHandler");
    AppGlobals*appGlobals = (AppGlobals*)getUserDataFromGlobalVar(state, "appGlobal");
    WorkflowPlugin*plugin=(WorkflowPlugin*)appGlobals->plugins[CONF_WORKFLOW];
    executionContext->variables["input"]=data;
    nodeHandler->sendToOutput(plugin, port, *executionContext);
    return 0;
}

int input(lua_State *state) {
    WFExecutionContext*executionContext = (WFExecutionContext*)getUserDataFromGlobalVar(state, "executionContext");
    QString input="";
    if(executionContext->variables.contains("input")) {
        input=executionContext->variables["input"].toString();
    }

    QByteArray utf8 = input.toUtf8();
    lua_pushstring(state, utf8.constData());
    return 1;
}

LuaExecutor::LuaExecutor(AppGlobals*appGlobals) {
    this->appGlobals=appGlobals;
}

void LuaExecutor::execute(QString luaScript, WFExecutionContext*executionContext, WFNodeHandler*wfNodeHandler) {
    lua_State *state = luaL_newstate();
    luaL_openlibs(state);
    luaState = state;

    lua_pushlightuserdata(state,wfNodeHandler);
    lua_setglobal(state, "wfNodeHandler");

    lua_pushlightuserdata(state,appGlobals);
    lua_setglobal(state, "appGlobal");
    lua_register(state, "var", getVar);
    lua_register(state, "input", input);
    lua_register(state, "output", output);

    lua_pushlightuserdata(state, executionContext);
    lua_setglobal(state, "executionContext");

    QByteArray utf8 = luaScript.toUtf8();
    const char *data = utf8.constData();
    int result=luaL_dostring(state, data);
    if(result!=LUA_OK) {
        const char*result=lua_tostring(state, 1);
        QString errorMessage=QString(result);
        qDebug()<<"Lua Script error: "<<errorMessage;
        lua_pop(state, 1);
        lua_close(state);
        return;
    }
    lua_close(state);
}
