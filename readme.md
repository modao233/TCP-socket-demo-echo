# TCP socket demo: echo

`io_base.h`: 改善read、write读不完或写不完，参考自unp

`tcp_client.c`: 普通的echo客户端

`tcp_server.c`: 普通的echo服务端，主进程处理连接，子进程处理IO

`./select`目录: 使用IO复用-select重写echo demo

`./poll`目录: 使用IO复用-poll重写echo demo
