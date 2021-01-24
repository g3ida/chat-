set( PROJECT_NAME chatpp )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

project(chatpp)

if(WIN32)
  set(CHATTPP_WINSOCK_LIBRARY ws2_32 wsock32)
endif()

if(UNIX)
  set(PTHREAD_LIBRARY pthread)
endif()

add_executable(chatpp main.cpp server.cpp client.cpp message.cpp)

target_link_libraries(chatpp PRIVATE ${CHATTPP_WINSOCK_LIBRARY} ${PTHREAD_LIBRARY})
