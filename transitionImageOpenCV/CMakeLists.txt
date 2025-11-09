cmake_minimum_required(VERSION 3.10)
project(transition_cli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

# Si tu as installé nlohmann-json en package CMake, tu peux décommenter :
# find_package(nlohmann_json CONFIG REQUIRED)

add_executable(transition_cli
    main.cpp
    video_producer.cpp
    transitions.cpp
)

target_include_directories(transition_cli PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    # ajoute ici le chemin vers nlohmann/json.hpp si besoin
    # /usr/include/nlohmann
)

target_link_libraries(transition_cli PRIVATE
    ${OpenCV_LIBS}
)

