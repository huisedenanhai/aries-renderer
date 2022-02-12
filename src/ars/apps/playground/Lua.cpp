#include <iostream>
#include <lua/lua.hpp>
#include <string>

static int my_print(lua_State *L) {
    std::cout << "my_print: ";
    int n = lua_gettop(L); /* number of arguments */
    int i;
    for (i = 1; i <= n; i++) { /* for each argument */
        size_t l;
        const char *s = luaL_tolstring(L, i, &l); /* convert it to string */
        if (i > 1) {                              /* not the first element? */
            lua_writestring("\t", 1);             /* add a tab before it */
        }
        lua_writestring(s, l); /* print it */
        lua_pop(L, 1);         /* pop result */
    }
    std::cout << "\n";
    return 0;
}

static luaL_Reg log_funcs[] = {
    {"print", my_print},
    {nullptr, nullptr},
};

int protected_main(lua_State *L) {
    lua_pushglobaltable(L);
    luaL_setfuncs(L, log_funcs, 0);
    luaL_dostring(L, "print 'hello lua'");

    for (int i = 0; i < 3; i++) {
        lua_getglobal(L, "print");
        if (lua_isfunction(L, -1)) {
            auto info = "Another Hello " + std::to_string(i);
            lua_pushstring(L, info.c_str());
            lua_pcall(L, 1, 0, 0);
        } else {
            std::cerr << "Fuck" << std::endl;
        }
    }
    std::cout << "Hello World" << std::endl;

    lua_pushinteger(L, 0);
    // only return one value
    return 1;
}

static int check_and_report(lua_State *L, int status) {
    if (status != LUA_OK) {
        const char *msg = lua_tostring(L, -1);
        std::cerr << msg << std::endl;
        lua_pop(L, 1); /* remove message */
    }
    return status;
}

int main() {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    lua_gc(L, LUA_GCGEN, 0, 0); /* GC in generational mode */

    // call the logic in protected mode to get better error info
    lua_pushcfunction(L, protected_main);
    auto status = lua_pcall(L, 0, 1, 0);
    auto ret = lua_tointeger(L, -1);
    check_and_report(L, status);

    lua_close(L);
    return static_cast<int>(ret);
}
