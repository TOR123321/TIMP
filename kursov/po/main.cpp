#include <iostream>
#include <cstdlib>
#include <cstring>//line СИ
#include <unistd.h> // for close()
//#include
#include <netinet/in.h>// sock_addr_in and allthing function  for network
#include <arpa/inet.h>
#include <sstream>
#include <string>
using namespace std;
void errors(const char * why, const int exitCode = 1)
{
    cerr<<why<<endl;
    exit(exitCode);
}
int main(int argc, char **argv)
{ int port;
    cout<<"Введите порт: ";
    cin>>port;
    // подготовка структуры с адресом нашей программы (клиента)
    sockaddr_in * selfAddr=new (sockaddr_in);
    selfAddr->sin_family = AF_INET; //интернет протокол IPv4
    selfAddr->sin_port = 0;         // при взаимодейстиве ОС выберет сама порт, как удобней
    selfAddr->sin_addr.s_addr = 0;  // все адреса нашего компьютера
    // то есть для своего адреса желатаельно указывать для поля нуль, чтобы ОС сама выбирала
    // подготовить структуру с адресом сервера
    sockaddr_in * remoteAddr=new (sockaddr_in);
    remoteAddr->sin_family = AF_INET;
    remoteAddr->sin_port = htons(port);
    remoteAddr->sin_addr.s_addr = inet_addr("127.0.0.1");

    // подготовка буфера для передачи и приема данных
    char *buf = new char[256];
    cout<<"Введите адрес файла, который хотите получить и сохранить";
    cin>>buf;
    int msgLen = strlen(buf); // вычисление длины строки

    // создание сокета
    int mySocket = socket(AF_INET, SOCK_STREAM, 0); //TCP
    if (mySocket == -1) {
        errors("Error open socket", 11);
    }
    // привязка сокета к своему адресу
    int rc = bind (mySocket, (const sockaddr*) selfAddr, sizeof (sockaddr_in));
    if (rc == -1) {
        close(mySocket);
        errors("Error bind socket with local address", 12);
    }
    // установка соединение с сервером
    rc = connect (mySocket, (const sockaddr*) remoteAddr/*адрес*/, sizeof (sockaddr_in));
    if (rc == -1) {
        close(mySocket);
        errors("Error bind socket with remote address", 13);
    }
    // передача данных
    rc = send(mySocket, buf, msgLen, 0); // возвращает количестово переданных байт
    if (rc == -1) {
        close(mySocket);
        errors("Error send message", 14);
    }
    cout<< "We send: "<<buf<<endl;

    // принятие ответа
    stringstream response; // сюда будет записываться ответ клиенту
    stringstream response_body; // тело ответа
    rc = recv(mySocket, buf, 256, 0);
    if (rc == -1) {
        close(mySocket);
        errors("Error recevie answer", 15);
    } else {
        // Мы знаем фактический размер полученных данных, поэтому ставим метку конца строки
        // В буфере запроса.
        buf[rc] = '\0';

        // Данные успешно получены
        // формировка тело ответа (HTML)
        response_body << "<title>Test C++ HTTP Server</title>\n"
                      << "<h1>Test page</h1>\n"
                      << "<p>This is body of the test page...</p>\n"
                      << "<h2>Request headers</h2>\n"
                      << "<pre>" << buf << "</pre>\n"
                      << "<em><small>Test C++ Http Server</small></em>\n";
        // Формировка ответа вместе с заголовками
        response << "HTTP/1.1 200 OK\r\n"
                 << "Version: HTTP/1.1\r\n"
                 << "Content-Type: text/html; charset=utf-8\r\n"
                 << "Content-Length: " << response_body.str().length()
                 << "\r\n\r\n"
                 << response_body.str();
        // Отправка ответа клиенту с помощью функции send
        rc = send(mySocket, response.str().c_str(),response.str().length(), 0);
        if (rc == -1) {
            close(mySocket);
            errors("Error send answer", 16);
        }
    }
    // закрытие сокета
    close(mySocket);
    // освобождение памяти
    delete selfAddr;
    delete remoteAddr;
    delete[] buf;
    return 0;
}
