rm -rf select_server select_client epoll_server epoll_client
gcc select/server.c common/wrap.c common/wrap.h -o select_server
gcc select/client.c common/wrap.c common/wrap.h -o select_client
gcc epoll/sever.c common/wrap.c common/wrap.h -o epoll_server
gcc epoll/client.c common/wrap.c common/wrap.h -o epoll_client