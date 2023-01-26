#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main() {
	char filename[100], file_buffer[3000];
	struct sockaddr_in serveraddr, clientaddr;

	// Создание сокета
	int socketfd = socket(AF_INET, SOCK_DGRAM, 0);

	if (socketfd < 0) {
		perror("Failed to create socket!");
		exit(EXIT_FAILURE);
	}
	else {
		printf("Socket created!\n");
	}

	// Параметры сокета
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY; // для localhost
	serveraddr.sin_port = htons(6789);

	// Привязка сокета
	if (bind(socketfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		perror("Failed to bind!");
	}
	else {
		printf("Binded!\n");
	}

	memset(filename, 0, 100);

	while(1) {
		int lenght_addr = sizeof(clientaddr);
		recvfrom(socketfd, filename, 1024, 0, (struct sockaddr *)&clientaddr, &lenght_addr);
		printf("Received filename: %s\n", filename);

		FILE *f;
		printf("Contents of the received file:\n");
		recvfrom(socketfd, file_buffer, 1024, 0, (struct sockaddr *)&clientaddr, &lenght_addr);
		printf("%s\n", file_buffer);
		int file_size = strlen(file_buffer);
		f = fopen(filename, "w");

		if (f) {
			fwrite(file_buffer, file_size, 1, f);
			printf("The file was successfully received!\n");
		}
		else {
			perror("Failed to open file!\n");
		}

		memset(filename, '\0', sizeof(filename));
		fclose(f);
		break;
	}

	close(socketfd);
  return 0;
}