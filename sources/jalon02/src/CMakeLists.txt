cmake_minimum_required(VERSION 2.8.3)

project(RE216_JALON02)
add_subdirectory(include)


#add here source files you need to compile and link to the client app
SET(SRC_CLIENT
	client.c
	${client_tools_files}
    include/general_tools.h
    include/general_tools.c
    include/client/client.h
    include/client/client_tools.h
    include/client/client_tools.c
	)
add_executable(RE216_JALON02_CLIENT ${SRC_CLIENT})


#add here source files you need to compile and link to the server app
SET(SRC_SERVER
	server.c
	${server_tools_files}
    include/general_tools.h
    include/general_tools.c
    include/server/server.h
    include/server/server_tools.h
    include/server/server_tools.c
	)
add_executable(RE216_JALON02_SERVER  ${SRC_SERVER})
