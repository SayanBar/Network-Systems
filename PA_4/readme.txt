Programming Assignment 4

Objective:To create a local proxy server


There are 4 files in the folder. Makefile, proxy_server.c, Blocked.conf and DNSCache.conf

Proxy_server.c is the source code.
Blocked.conf contains all the websites and IP address of the blocked websites.
DNSCache.conf caches all the IP addresses of the visited websites and retrieves it from the file when requested by the client.


The proxy server is made using the makefile and run as ./proxy <portno>

The proxy server serves the following functionalities:
1)Acts as a mediator between server and client.
2)Filters 400 Bad Requests by the client without connecting to the server
3)Blocks the client from visiting blocked websites and returns a 403 Forbidden Error.
4)Caches visited websites IP and also files so that they can retrieved from cache.
5)Cache Timeout.


Used md5sum value to name files so that they are unique




