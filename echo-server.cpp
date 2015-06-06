#include <iostream>
#include <string>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int MAX_NPORTS = 5;

int nports = 0;
int ports[MAX_NPORTS];
HANDLE threads[MAX_NPORTS];
SOCKET sockets[MAX_NPORTS];

void parseArgs(int argc, char** argv);
DWORD WINAPI thread(LPVOID param);

int main(int argc, char** argv) {
	setlocale(LC_ALL, "Russian");

	try {
		parseArgs(argc, argv);
	}
	catch (const exception& ex) {
		cerr << "error: " << ex.what() << endl;
		return EXIT_FAILURE;
	}

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,0), &wsaData) != NO_ERROR) { 
		return EXIT_FAILURE;
	}
	
	for (int i = 0; i<nports; ++i) {
		threads[i] = CreateThread(NULL, 0, thread, reinterpret_cast<LPVOID>(i), 0, NULL);
		cout << "listen: " << ports[i] << endl;
	}

	for (;;);
	
	for (int i = 0; i<nports; ++i) {
		if (sockets[i]) closesocket(sockets[i]);
	}
	WSACleanup();
	return 0;
}

void parseArgs(int argc, char** argv) {
	if (argc < 2) {
		cout << "Usage: " << argv[0] << " <port1> <port2> ..." << endl << endl;
		exit(EXIT_SUCCESS);
	}

	nports = argc - 1;
	for (int i = 0; i<nports; ++i) {
		ports[i] = atoi(argv[i+1]);
	}
}

DWORD WINAPI thread(LPVOID param) {
	int id = reinterpret_cast<int>(param);
	
	SOCKET server = sockets[id] = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	if (server==SOCKET_ERROR) return EXIT_FAILURE;

	sockaddr_in inf_addr;
	memset(&inf_addr, 0, sizeof(inf_addr));
	inf_addr.sin_port = htons(ports[id]);
	inf_addr.sin_family = AF_INET;
	inf_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server, (sockaddr*)&inf_addr, sizeof(inf_addr)) == SOCKET_ERROR) {
		return EXIT_FAILURE;
	}

	if (listen(server, SOMAXCONN) == SOCKET_ERROR) {
		return EXIT_FAILURE;
	}
	
	for (;;) {
		sockaddr_in tmp_addr;
		int addrlen = sizeof(tmp_addr);
		memset(&tmp_addr, 0, sizeof(tmp_addr));
		SOCKET client = accept(server, (sockaddr*)&tmp_addr, &addrlen);
		cout << "accept: " << ports[id] << endl;
		int length;
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		while ((length = recv(client, buf, sizeof(buf), 0)) > 0) {
			if (length==1 && buf[0]=='D') { //D
				for (int i = 0; i<nports; ++i) {
					if (i!=id && threads[i]) {
						cout << "Closed connections on port: " << ports[i] << endl;
						TerminateThread(threads[i], 0);
						closesocket(sockets[i]);
						sockets[i] = 0;
						threads[i] = 0;
					}
				}
			}
			else {
				send(client, buf, length, 0);
				cout << "send: " << buf << endl;
			}
			memset(buf, 0, sizeof(buf));
		}
		closesocket(client);
	}

	return 0;
}
