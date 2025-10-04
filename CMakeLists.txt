cmake_minimum_required(VERSION 3.10)
project(VideoMerger)

# Définir le standard C++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trouver OpenCV
find_package(OpenCV REQUIRED)

# Inclure les headers OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Ajouter les exécutables
add_executable(video_merger main.cpp)
add_executable(mergeimagetovideo mergeimagetovideo.cpp)

# Lier avec OpenCV
target_link_libraries(video_merger ${OpenCV_LIBS})
target_link_libraries(mergeimagetovideo ${OpenCV_LIBS})

# Options de compilation
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(video_merger PRIVATE -Wall -Wextra -O3)
    target_compile_options(mergeimagetovideo PRIVATE -Wall -Wextra -O3)
endif()

# Afficher les informations
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libs: ${OpenCV_LIBS}")
message(STATUS "Building executables: video_merger, mergeimagetovideo")
