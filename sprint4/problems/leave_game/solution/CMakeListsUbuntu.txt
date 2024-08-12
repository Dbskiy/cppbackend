cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(MSVC)
  add_compile_options(/bigobj)
endif()

add_library(MyLib STATIC
    src/collision_detector.h
    src/collision_detector.cpp
    src/providers.h
    src/model.h
    src/model.cpp
    src/state_serialization.h
    src/loot_generator.h
    src/loot_generator.cpp
    src/tagged.h
)

target_include_directories(MyLib PUBLIC CONAN_PKG::boost)
target_link_libraries(MyLib PUBLIC CONAN_PKG::boost Threads::Threads CONAN_PKG::libpq CONAN_PKG::libpqxx)

add_executable(game_server
    src/main.cpp
    src/http_server.cpp
    src/http_server.h
    src/sdk.h
    src/boost_json.cpp
    src/json_loader.h
    src/json_loader.cpp
    src/request_handler.cpp
    src/request_handler.h
    src/static_content.cpp
    src/static_content.h
    src/logger.cpp
    src/logger.h
    src/app.cpp
    src/app.h
    src/ticker.cpp
    src/ticker.h
    src/loot_extra_data.cpp
    src/loot_extra_data.h
    src/save_and_restore_state.h
    src/serializing_listener.h
    src/connection_pool.h
    src/data_base.cpp
    src/data_base.h
    src/tagged_uuid.cpp
    src/tagged_uuid.h
)

target_link_libraries(game_server PRIVATE MyLib)

add_executable(game_server_tests 
    tests/loot_generator_tests.cpp
    tests/model_tests.cpp
    tests/collision-detector-tests.cpp
    tests/state-serialization-tests.cpp
)

target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2 MyLib)
