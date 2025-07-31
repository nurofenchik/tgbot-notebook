cmake_minimum_required(VERSION 3.10)
project(MyTelegramBot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Добавляем tgbot-cpp как подпроект
add_subdirectory(src/tgbot-cpp)

# Для sqlite_modern_cpp создаем библиотеку вручную, так как их CMakeLists.txt несовместим
# Это header-only библиотека, поэтому просто создаем INTERFACE target
add_library(sqlite_modern_cpp INTERFACE)
target_include_directories(sqlite_modern_cpp INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite_modern_cpp/hdr
)

# Включаем директории заголовочных файлов
include_directories(src/tgbot-cpp/include)
include_directories(include)  # Добавляем папку include для наших заголовков

# Ищем необходимые системные библиотеки для нашего проекта
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Ищем SQLite3 для работы с базой данных
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# Создаем исполняемый файл нашего бота
add_executable(bot 
    src/main.cpp
    src/database.cpp  # Добавляем наш файл database.cpp
)

# Линкуем все необходимые библиотеки
target_link_libraries(bot 
    TgBot 
    sqlite_modern_cpp
    Threads::Threads 
    Boost::system 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    ${SQLITE3_LIBRARIES}
    z
)

# Добавляем include директории для SQLite3
target_include_directories(bot PRIVATE ${SQLITE3_INCLUDE_DIRS})

# Устанавливаем свойства компиляции
if(NOT WIN32)
    target_compile_options(bot PRIVATE -Wall)
endif()

# Копируем необходимые файлы в build директорию
configure_file(${CMAKE_SOURCE_DIR}/files/help_response.txt ${CMAKE_BINARY_DIR}/help_response.txt COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/files/token.txt ${CMAKE_BINARY_DIR}/token.txt COPYONLY)
