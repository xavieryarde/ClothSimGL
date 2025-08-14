cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (ClothSimGL)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/external/glad/src/gl.c)
set(HEADERS ${CMAKE_SOURCE_DIR}/include)

add_executable (${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 glm::glm) 
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/external/glad/include ${HEADERS}) 

if (WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>"
        "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
    )
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# adds exe and other necessary files to install dir
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

if(WIN32)
    install(IMPORTED_RUNTIME_ARTIFACTS SDL3::SDL3-shared RUNTIME DESTINATION bin)
endif()

