/**
 * Network Broadcast (UDP on 255.255.255.255:port) send and rerceive
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>


static short port = 1883;


int send_msg()
{
	char data[] = "This is the message!";
	size_t data_len = sizeof(data);

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "error: no socket\n");
		return -1;
	}

	int on = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

	struct sockaddr_in sendAddr;
	memset(&sendAddr, 0, sizeof(sendAddr));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_port = htons(port);
	sendAddr.sin_addr.s_addr = htonl(0xffffffff); /* 255.255.255.255 */

	printf("sending [%i] \"%s\"\n", (int)data_len, data);

	ssize_t result = sendto(sockfd, data, data_len, 0, (struct sockaddr *)&sendAddr, sizeof(sendAddr));

	if ((ssize_t)data_len != result) {
		printf("error: send result=%i, errno=%i %s\n", (int)result, errno, strerror(errno));
		return -1;
	} else {
		printf("send succeeded\n");
	}
	return 0;
}


int receive_msg()
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "error: no socket\n");
		return -1;
	}

	int on = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);


	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		fprintf(stderr, "error: bind\n");
		return -1;
	}

	while (1) {
		char data[1024];
		struct sockaddr_in src_addr_in;
		socklen_t src_addr_in_len = sizeof(src_addr_in);

		char s_srcaddr[16];

		ssize_t len = recvfrom(sockfd, data, sizeof(data) - 1, 0, (struct sockaddr *)&src_addr_in, &src_addr_in_len);
		data[len] = '\0';

		printf("received [%i] \"%s\"\n", (int)len, data);

		unsigned long senderip = ntohl(src_addr_in.sin_addr.s_addr);
		printf("sender: %i.%i.%i.%i port=%i\n", 
			(int)((senderip >> 24) & 0x000000ff),
			(int)((senderip >> 16) & 0x000000ff),
			(int)((senderip >>  8) & 0x000000ff),
			(int)((senderip >>  0) & 0x000000ff),
			src_addr_in.sin_port);
	}
}

int main(int argc, char** argv)
{
	if (argc <= 1) {
		return -1;
	}

	if (0 == strcmp("-s", argv[1])) {
		return send_msg();
	}
	if (0 == strcmp("-r", argv[1])) {
		return receive_msg();
	}
	return -1;
}

