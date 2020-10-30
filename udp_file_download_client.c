/* upd_file_download_client.c - main */

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>                                                                            
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

struct PDU {
	char type;
	char data[100];
};

#define	BUFSIZE 	sizeof(struct PDU)
#define DATA 		'D'
#define FINAL 		'F'
#define ERROR 		'E'
#define FILENAME 	'C'

/*------------------------------------------------------------------------
 * main - UDP client for FILE DOWNLOAD service that prints the resulting time
 *------------------------------------------------------------------------
 */
int main(int argc, char **argv)
{
	char	*host = "localhost";
	int		port = 3000;
	char	now[100];		/* 32-bit integer to hold time	*/ 
	struct 	hostent	*phe;	/* pointer to host information entry	*/
	struct 	sockaddr_in sin;	/* an Internet endpoint address		*/
	int		s, n, type;	/* socket descriptor and socket type	*/
	int		socket_read_size;
	char	socket_read_buf[BUFSIZE];
	int 	console_read_size;
	char 	console_read_buf[100];
	struct 	PDU 	msg;
	FILE*	fp;
	char	file_name[100];
	int		error_found = 0;

	switch (argc) {
		case 1:
			break;
		case 2:
			host = argv[1];
		case 3:
			host = argv[1];
			port = atoi(argv[2]);
			break;
		default:
			fprintf(stderr, "usage: UDP File Download [host [port]]\n");
			exit(1);
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;                                                                
	sin.sin_port = htons(port);
                                                                                        
    /* Map host name to IP address, allowing for dotted decimal */
	if ( phe = gethostbyname(host) ){
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	}
	else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");
                                                                                
    /* Allocate a socket */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		fprintf(stderr, "Can't create socket \n");
	
                                                                                
    /* Connect the socket */
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		fprintf(stderr, "Can't connect to %s\n", host);
	}

	printf("Enter File Name:\n");
	console_read_size = read(0, console_read_buf, BUFSIZE);
	if (console_read_size <= 0) {
		printf("Error Reading File Name\n");
		return -1;
	}
	
	msg.type = FILENAME;
	memcpy(msg.data, console_read_buf, console_read_size);
	msg.data[console_read_size - 1] = '\0';
	write(s, &msg, BUFSIZE);
	
	while(socket_read_size = read(s, socket_read_buf, BUFSIZE)) {
		
		memcpy(&msg, socket_read_buf, BUFSIZE);
		
		if (msg.type == FILENAME) {
			memcpy(file_name, msg.data, socket_read_size - 4);
			file_name[socket_read_size - 1] = '\0';
			fp = fopen(file_name, "w");
		}
		else if (msg.type == DATA) {
			fwrite(msg.data, 1, socket_read_size - 1, fp);
		}
		else if (msg.type == FINAL) {
			int i;
			int data_size = 0;
			for (i = 0; i < socket_read_size - 1; i++) {
				if (msg.data[i] == '\0') {
					data_size = i + 1;
					break;
				}
			}
			fwrite(msg.data, 1, data_size - 1, fp);
			fclose(fp);
			break;
		}
		else if (msg.type == ERROR) {
			error_found = 1;
			break;
			
		}
	}
	
	if (error_found){
		printf("Got an error\n");
		printf("File Download Incomplete\n");
	}
	else {
		printf("No errors\n");
		printf("File Download Complete\n");
	}
	
	close(s);
	exit(0);
}
