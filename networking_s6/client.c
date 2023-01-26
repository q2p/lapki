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
	struct sockaddr_in serveraddr;

	// Создание сокета
	int socketfd = socket(AF_INET, SOCK_DGRAM, 0);

	if (socketfd == -1) {
		perror("Failed to create socket!");
		exit(EXIT_FAILURE);
	}

	memset(&serveraddr, 0, sizeof(serveraddr));

	// Информация о сервере
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(6789);
	serveraddr.sin_addr.s_addr = INADDR_ANY;

	printf("Enter a filename: ");
	scanf("%s", filename);
	printf("\n");

	if (sendto(socketfd, filename, strlen(filename), 0, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr)) < 0) {
		perror("Failed to sent filename!");
	}

	FILE *f;
	f = fopen(filename, "r");

	if (f) {
		fseek(f, 0, SEEK_END);
		size_t file_size = ftell(f);
		fseek(f, 0, SEEK_SET);
		printf("File size: %li bytes\n", file_size);
		if (fread(file_buffer, file_size, 1, f) <= 0) {
			perror("Unable to copy file into buffer or file is empty!");
			exit(EXIT_FAILURE);
		}
	}
	else {
		perror("Failed to open file!");
		exit(EXIT_FAILURE);
	}
	fclose(f);

	if (sendto(socketfd, file_buffer, strlen(file_buffer), 0, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr)) == -1) {
		perror("Failed to sent file!");
	}
	else {
		printf("The file has been sent successfully!\n");
	}

	printf("The content of the sent file:\n");
	printf("%s\n", file_buffer);

	close(socketfd);
	return 0;
}
