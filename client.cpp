/*

Example: Client.exe <port>

*/

#include <iostream>
#include <string>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main(int argc, char** argv) {
  setlocale(LC_ALL, "Russian");

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2,0), &wsaData) != NO_ERROR) { 
    return EXIT_FAILURE;
  }

  long inadr = inet_addr("127.0.0.1");
  auto host = gethostbyaddr((char*)&inadr, 40, AF_INET);

  int port = atoi(argv[1]);
  SOCKET server = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
  if (server == SOCKET_ERROR) {
    cout<<"Сокет создать не удалось..."<<endl;
	return EXIT_FAILURE;
  }

  char buffer[256];
  sockaddr_in inf_addr;
  memset(&inf_addr, 0, sizeof(inf_addr));
  inf_addr.sin_port = htons(port);
  inf_addr.sin_family = AF_INET;
  inf_addr.sin_addr = *((in_addr*)*host->h_addr_list);
  if (connect(server, (sockaddr*)&inf_addr, sizeof(inf_addr)) != SOCKET_ERROR) {
    cout << "Связь с сервером установлена!" << endl;
	for (;;) {
	  cout << "> ";
	  cin.getline(buffer, 255);
	  if (strlen(buffer)==1 && buffer[0]=='D') {
		send(server, buffer, 1, 0);
	  }
	  else {
	    cout << "send: " << strlen(buffer)+1 << ", " << buffer << endl;
		send(server, buffer, strlen(buffer)+1, 0);
		int n = recv(server, buffer, sizeof(buffer), 0);
		cout << "recv: " << n << ", " << buffer << endl;
	  }
	}
  }
  else {
	cout << "Связь с сервером установить не удалось..." << endl;
	closesocket(server);
	WSACleanup();
	return EXIT_FAILURE;
  }

  closesocket(server);
  WSACleanup();

  return 0;
}
