cmake_minimum_required(VERSION 3.22)

project(SofaImplicitField_test)

set(SOURCE_FILES
    ImplicitShape_test.cpp
    ScalarFieldMapping_test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} SofaImplicitField Sofa.Testing Sofa.Component.Mapping.Testing)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
