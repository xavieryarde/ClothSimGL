cmake_minimum_required (VERSION 3.21)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (ClothSimGL)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/external/glad/src/gl.c)
set(HEADERS ${CMAKE_SOURCE_DIR}/include)

add_executable (${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 glm::glm $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static> assimp::assimp) 
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/external/glad/include ${HEADERS}) 

if (WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>"
        "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

# adds exe and other necessary files to install dir
install(TARGETS ${PROJECT_NAME} RUNTIME_DEPENDENCY_SET deps RUNTIME DESTINATION ${PROJECT_NAME})
install(DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets DESTINATION ${PROJECT_NAME})
install(RUNTIME_DEPENDENCY_SET deps DESTINATION ${PROJECT_NAME} PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-" POST_EXCLUDE_REGEXES ".*system32/.*")




