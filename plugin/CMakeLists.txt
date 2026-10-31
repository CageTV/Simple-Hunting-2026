cmake_minimum_required(VERSION 3.21)

# SimpleHunting.dll - the SKSE plugin behind Simple Hunting Overhaul 2026's SKSE Menu Framework settings pages.
project(SimpleHunting VERSION 1.1.0 LANGUAGES CXX)

find_package(CommonLibSSE CONFIG REQUIRED)

add_commonlibsse_plugin(${PROJECT_NAME} NAME "SimpleHunting" AUTHOR "CageTV" SOURCES
    src/main.cpp
    src/Settings.cpp
    src/Hotkeys.cpp
    src/Actions.cpp
    src/Menu.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_include_directories(${PROJECT_NAME} PRIVATE src include)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/Z7>)
target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/DEBUG /OPT:REF /OPT:ICF>)
