cmake_minimum_required(VERSION 3.0.0)
project(Server VERSION 0.0.1)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(SRC_LIST src/chat_message.cpp
             src/chat_message.hpp
             src/database.cpp
             src/database.hpp
             src/hasher.cpp
             src/hasher.hpp
             src/message_parser.cpp
             src/message_parser.hpp
             src/server_type.cpp
             src/server_type.hpp
             src/server.cpp
             src/server.hpp
             src/session.cpp
             src/session.hpp
             src/main.cpp
             src/chat_item.hpp
             src/inet.hpp
             src/connection_type.hpp
             src/message_item.hpp)


set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -lmariadbcpp)

add_compile_options(
   # -Werror
   # -pedantic-errors
    
    -Wall 
    -Wextra 
    -Wpedantic
    
    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wctor-dtor-privacy
    -Wduplicated-branches
    -Wduplicated-cond
    -Wextra-semi
    -Wfloat-equal
    -Wlogical-op
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    -Wsign-conversion
    -Wsign-promo
    -Wzero-as-null-pointer-constant
    -Wshadow=compatible-local
)

set(OUTPUT_TARGET server)

add_executable(${OUTPUT_TARGET} ${SRC_LIST})

target_compile_features(${OUTPUT_TARGET} PUBLIC cxx_std_20)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${OUTPUT_TARGET} Boost::system Boost::thread)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(${OUTPUT_TARGET} OpenSSL::SSL OpenSSL::Crypto)
