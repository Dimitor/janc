cmake_minimum_required(VERSION 3.18.0)
project(Client VERSION 0.0.1)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(SRC_LIST src/screens/chat_list_screen.cpp
             src/screens/chat_list_screen.hpp
             src/screens/info_screen.cpp
             src/screens/info_screen.hpp
             src/screens/message_input_screen.cpp
             src/screens/message_input_screen.hpp
             src/screens/message_list_screen.cpp
             src/screens/message_list_screen.hpp
             src/screens/profile_screen.cpp
             src/screens/profile_screen.hpp
             src/screens/screen_manager.cpp
             src/screens/screen_manager.hpp
             src/screens/app_screen.cpp
             src/screens/app_screen.hpp
             src/screens/create_chat_screen.cpp
             src/screens/create_chat_screen.hpp
             src/screens/selected_participants_screen.cpp
             src/screens/selected_participants_screen.hpp
             src/screens/user_list_screen.cpp
             src/screens/user_list_screen.hpp
             src/screens/dialog_screen.cpp
             src/screens/dialog_screen.hpp
             src/buffer.cpp
             src/buffer.hpp
             src/chat_client.cpp
             src/chat_client.hpp
             src/chat_message.cpp
             src/chat_message.hpp
             src/database.cpp
             src/database.hpp
             src/editor.cpp
             src/editor.hpp
             src/inf_line.cpp
             src/inf_line.hpp
             src/message_handler.cpp
             src/message_handler.hpp
             src/message_parser.cpp
             src/message_parser.hpp
             src/main.cpp
             src/chat_item.hpp
             src/chat_user.hpp
             src/connection_type.hpp
             src/inet.hpp
             src/message_item.hpp
             src/scr_size_data.hpp)

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

add_compile_definitions(NCURSES_NOMACROS)

set(OUTPUT_TARGET janc)

add_executable(${OUTPUT_TARGET} ${SRC_LIST})

target_compile_features(${OUTPUT_TARGET} PUBLIC cxx_std_20)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${OUTPUT_TARGET} Boost::system Boost::thread)

find_package(Curses REQUIRED)
find_library(CURSES_PANEL_LIBRARY NAMES panel REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})
target_link_libraries (${OUTPUT_TARGET} ${CURSES_LIBRARIES} ${CURSES_PANEL_LIBRARY})

find_package(SQLite3 REQUIRED)
include_directories(${SQLITE3_INCLUDE_DIRS})
target_link_libraries(${OUTPUT_TARGET} SQLite::SQLite3)

# include(FetchContent)
# set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
# FetchContent_Declare(
#     ftxui
#     GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
#     GIT_TAG        master
# )
# FetchContent_GetProperties(ftxui)
# if(NOT ftxui_POPULATED)
#   FetchContent_Populate(ftxui)
#   add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
# endif()

# target_link_libraries(${OUTPUT_TARGET} ftxui::screen ftxui::dom ftxui::component)