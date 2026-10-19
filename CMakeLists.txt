cmake_minimum_required(VERSION 3.21)

project(NPTVision
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Widgets
)

qt_standard_project_setup()

qt_add_executable(NPTVision
    src/app/main.cpp
    src/gui/MainWindow.h
    src/gui/MainWindow.cpp
)

target_link_libraries(NPTVision PRIVATE
    Qt6::Widgets
)

if(WIN32)
    set_target_properties(NPTVision PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
