cmake_minimum_required (VERSION 3.21)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

if(POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif()

project (ClothSimGL)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/external/glad/src/gl.c)
set(HEADERS ${CMAKE_SOURCE_DIR}/include)

set(IS_MACOS_RELEASE FALSE)
if(APPLE AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(IS_MACOS_RELEASE TRUE)
endif()

if(IS_MACOS_RELEASE)
    
    set(MACOSX_ICON_FILE ${CMAKE_SOURCE_DIR}/exe_icon.icns)
    set_source_files_properties(${MACOSX_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${MACOSX_ICON_FILE})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_ICON_FILE exe_icon.icns
        MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.xavieryarde.${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/resource.rc)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 glm::glm $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static> imgui::imgui) 
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/external/glad/include ${HEADERS}) 

if (WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>"
        "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
    )
endif()

if(IS_MACOS_RELEASE)
    set(ASSET_DEST_DIR "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Resources/assets")
else()
    set(ASSET_DEST_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets")
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            ${ASSET_DEST_DIR})

# adds exe and other necessary files to install dir
install(TARGETS ${PROJECT_NAME} RUNTIME_DEPENDENCY_SET deps RUNTIME DESTINATION ${PROJECT_NAME} BUNDLE DESTINATION ${PROJECT_NAME})

if(NOT IS_MACOS_RELEASE)
    install(DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets DESTINATION ${PROJECT_NAME})
endif()

install(RUNTIME_DEPENDENCY_SET deps DESTINATION ${PROJECT_NAME} PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-" POST_EXCLUDE_REGEXES ".*system32/.*")