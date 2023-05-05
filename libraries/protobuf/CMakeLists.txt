cmake_minimum_required(VERSION 3.8)

project(mmm_proto)

set (SRCS
	src/proto_library.cpp
	"include/proto_library.h")

set(PROTO_DEF 
	src/enum.proto
	src/midi.proto
	src/midi_internal.proto
	src/track_type.proto
	src/test.proto)

include_directories(${Protobuf_INCLUDE_DIRS})
find_package(Protobuf REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
	${PROTO_DEF})

add_library(mmm_proto
	${SRCS}
	${PROTO_SRCS} 
	${PROTO_HDRS})

target_link_libraries(mmm_proto ${Protobuf_LIBRARIES})