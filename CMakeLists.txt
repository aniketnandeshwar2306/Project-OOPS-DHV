cmake_minimum_required(VERSION 3.10)
project(CivicCare LANGUAGES CXX)

# Require C++17 standard (with C++14 backward compatibility)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(include)

# Member 2 Source Files
set(MEMBER2_SOURCES
    src/models/User.cpp
    src/models/Citizen.cpp
    src/models/MunicipalStaff.cpp
    src/models/Administrator.cpp
    src/models/Session.cpp
    src/repositories/FileUserRepository.cpp
    src/services/AuthenticationService.cpp
    src/utils/PasswordHasher.cpp
)

# Optional SQLite Support (coordinated with Member 4)
find_package(SQLite3 QUIET)
if(SQLite3_FOUND)
    message(STATUS "SQLite3 found! Enabling SQLiteUserRepository.")
    list(APPEND MEMBER2_SOURCES src/repositories/SQLiteUserRepository.cpp)
    add_definitions(-DCIVICCARE_ENABLE_SQLITE=1)
else()
    message(STATUS "SQLite3 not found. Compiling with FileUserRepository (file stream persistence).")
    list(APPEND MEMBER2_SOURCES src/repositories/SQLiteUserRepository.cpp)
endif()

# Static library for Member 2's components
add_library(civiccare_user_mgmt STATIC ${MEMBER2_SOURCES})
if(SQLite3_FOUND)
    target_link_libraries(civiccare_user_mgmt PRIVATE SQLite::SQLite3)
endif()

# Test Suite Executable
add_executable(test_user_management tests/test_user_management.cpp)
target_link_libraries(test_user_management PRIVATE civiccare_user_mgmt)

# Demonstration Executable
add_executable(civiccare_member2_demo demo/main_member2_demo.cpp)
target_link_libraries(civiccare_member2_demo PRIVATE civiccare_user_mgmt)

# Enable CTest
enable_testing()
add_test(NAME Member2_UserManagementTests COMMAND test_user_management)
