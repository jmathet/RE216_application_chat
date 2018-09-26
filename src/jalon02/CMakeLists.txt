cmake_minimum_required(VERSION 2.8.3)
find_package (Threads)

project(RE216_JALON02)

#add here source files you need to compile and link to the client app
SET(SRC_CLIENT
	client.c
	)


add_executable(RE216_JALON02_CLIENT ${SRC_CLIENT})


#add here source files you need to compile and link to the server app
SET(SRC_SERVER
        server.c)


add_executable(RE216_JALON02_SERVER  ${SRC_SERVER})


