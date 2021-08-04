cmake_minimum_required(VERSION 3.10)
project(TransportCatalogue CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

aux_source_directory(source SOURCE_LIST)
aux_source_directory(proto_files PROTO_FILES_LIST)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto map_renderer.proto
                          svg.proto graph.proto transport_router.proto)


add_executable(${PROJECT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${SOURCE_LIST})
target_include_directories(${PROJECT_NAME} PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(${PROJECT_NAME} "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)

set (CMAKE_CXX_FLAGS "-Wall -Wpedantic")