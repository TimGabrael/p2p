cmake_minimum_required(VERSION 3.25)
project(p2p_project)

# REMINDER FOR MYSELF:
# in build dir:
# cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1


set (CMAKE_BUILD_TYPE Release)

if(WIN32)
    find_path(LIBNICE_INC NAMES "nice.h" PATHS /include/nice)
    find_library(LIBNICE_LIB nice PATHS /lib/)

    find_path(GLIB_INC NAMES "glib-object.h" PATHS /include/glib-2.0)
    find_path(GLIB_CONF_INC NAMES "glibconfig.h" PATHS /lib/glib-2.0/include)

    find_library(GLIB_LIB glib-2.0 PATHS /lib/)
    find_library(GOBJ_LIB gobject-2.0 PATHS /lib/)
    find_library(GIO_LIB gio-2.0 PATHS /lib/)
else()
    find_path(LIBNICE_INC NAMES "nice.h" PATHS usr/local/include/nice/)
    find_library(LIBNICE_LIB libnice.so PATHS nice/)

    find_path(GLIB_INC NAMES "glib-object.h" PATHS /usr/include/glib-2.0)
    find_path(GLIB_CONF_INC NAMES "glibconfig.h" PATHS /usr/lib/x86_x64-linux-gnu/glib-2.0/include)

    find_library(GLIB_LIB glib-2.0 PATHS /lib/)
    find_library(GOBJ_LIB gobject-2.0 PATHS /lib/)
    find_library(GIO_LIB gio-2.0 PATHS /lib/)
endif()


include_directories(${LIBNICE_INC} ${GLIB_INC} ${GLIB_CONF_INC})
link_libraries(${LIBNICE_LIB} ${GLIB_LIB} ${GOBJ_LIB} ${GIO_LIB})

add_executable (p2p src/main.cpp)
