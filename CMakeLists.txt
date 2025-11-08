cmake_minimum_required(VERSION 3.20)
project(untitled5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Calea către SFML (folderul care conține fișierele .cmake)
set(SFML_DIR "C:/SFML-2.6.1/lib/cmake/SFML")

# Caută SFML 2.6
find_package(SFML 2.6 COMPONENTS system window graphics REQUIRED)

# Creează executabilul
add_executable(untitled6 main.cpp)

# Leagă SFML
target_link_libraries(untitled6 sfml-system sfml-window sfml-graphics)

# Copiază DLL-urile SFML lângă executabil (pentru a rula corect)
add_custom_command(TARGET untitled6 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "C:/SFML-2.6.1/bin"
        $<TARGET_FILE_DIR:untitled6>
)
