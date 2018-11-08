cmake_minimum_required(VERSION 2.8.3)

project(RE216_JALON04)
add_subdirectory(include)


#add here source files you need to compile and link to the client app
SET(SRC_CLIENT
	client.c
	${client_tools_files}
    include/general_tools.h
    include/general_tools.c
    include/client/client_tools.h
    include/client/client_tools.c
	)
add_executable(RE216_JALON04_CLIENT ${SRC_CLIENT})


#add here source files you need to compile and link to the server app
SET(SRC_SERVER
	server.c
	${server_tools_files}
    include/general_tools.h
    include/general_tools.c
    include/server/server_tools.h
    include/server/server_tools.c
    include/server/server_tools_users.h
    include/server/server_tools_users.c
    include/server/server_tools_channels.h
    include/server/server_tools_channels.c
	)
add_executable(RE216_JALON04_SERVER  ${SRC_SERVER})

target_link_libraries (RE216_JALON04_SERVER ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (RE216_JALON04_CLIENT ${CMAKE_THREAD_LIBS_INIT})
