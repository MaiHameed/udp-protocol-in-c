# UDP Protocol in C

This repo showcases an example application that showcases a file download system using the UDP protocol, written in C.
To use, compile both services, and run the server service on a host, providing a port number as an argument `./server <port>`. 
Run the client service on another host on the same network, providing the IP address of the server host as well as the port as two seperate arguments
`./client <ip-address> <port>`. The server host needs to contain a file that the client wants to download. The client is provided with the file name, and the client requests the file from the server. If successful, file download over UDP commences. 