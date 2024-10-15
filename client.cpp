#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <iostream>
#include "helpers.hpp"
#include "requests.hpp"
#include <string>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

int main(int argc, char *argv[])
{
    string message;
    string response;
    string input;
    string sessionCookie;
    string sessionJWT;
    bool isLogged = false;
    bool hasAccess = false;
    int sockfd;
    while(1) {
        getline(cin, input);
        if (input == "exit") {
            break;
        } else if (input == "register") {
            string usrname, passwrd;
            cout << "username=";
            getline(cin, usrname);
            cout << "password=";
            getline(cin, passwrd);
            if (usrname.find(' ') < usrname.size() || passwrd.find(' ') < usrname.size()) {
                cout << "Error: Invalid user credentials!\n";
                continue;
            }
            json userCredentials;

            userCredentials["username"] = usrname;
            userCredentials["password"] = passwrd;

            string payload = userCredentials.dump();

            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char *msg = compute_post_request("34.246.184.49", "/api/v1/tema/auth/register",
                "application/json", payload.c_str(), NULL, NULL);
            send(sockfd, msg, strlen(msg), 0);
            free(msg);
            char *res = receive_from_server(sockfd);
            response.assign(res);
            free(res);
            size_t lastNewline = response.find_last_of("\n");
            response = response.substr(lastNewline + 1);
            if (response == "ok") {
                cout << "User registerd successfully!\n";
            } else {
                cout << "Error: " << json::parse(response)["error"] << "\n";
            }
            close_connection(sockfd);
        } else if (input == "login") {
            string usrname, passwrd;
            cout << "username=";
            getline(cin, usrname);
            cout << "password=";
            getline(cin, passwrd);
            if (usrname.find(' ') < usrname.size() || passwrd.find(' ') < usrname.size()) {
                cout << "Error: Invalid user credentials!\n";
                continue;
            }
            json userCredentials;

            userCredentials["username"] = usrname;
            userCredentials["password"] = passwrd;

            string payload = userCredentials.dump();

            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char *msg = compute_post_request("34.246.184.49", "/api/v1/tema/auth/login",
                "application/json", payload.c_str(), NULL, NULL);
            send(sockfd, msg, strlen(msg), 0);
            free(msg);
            char *res = receive_from_server(sockfd);
            response.assign(res);
            free(res);
            size_t lastNewline = response.find_last_of("\n");
            string result = response.substr(lastNewline + 1);
            if (result == "ok") {
                cout << "User logged in successfully!\n";
                isLogged = true;
                size_t cookieStart = response.find("Set-Cookie: ") + strlen("Set-Cookie: ");
                // Luam si ';' avand in vedere ca oricum va trebui sa sa il adaugam
                size_t cookieEnd = response.find("; ", cookieStart);
                sessionCookie = response.substr(cookieStart, cookieEnd - cookieStart);
            } else {
                cout << "Error: " << json::parse(result)["error"] << "\n";
            }
            close_connection(sockfd);
        } else if (input == "logout") {
            if (!isLogged) {
                cout << "Error: The user is not logged in!\n";
                continue;
            }
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char *msg = compute_get_request("34.246.184.49", "/api/v1/tema/auth/logout", 
                NULL, sessionCookie.c_str(), NULL);
            send(sockfd, msg, strlen(msg), 0);
            free(msg);
            char *res = receive_from_server(sockfd);
            response.assign(res);
            free(res);
            size_t lastNewline = response.find_last_of("\n");
            response = response.substr(lastNewline + 1);
            if (response == "ok") {
                cout << "User logged out successfully!\n";
                isLogged = false;
                sessionCookie.clear();
                hasAccess = false;
                sessionJWT.clear();
            } else {
                cout << "Error: " << json::parse(response)["error"] << "\n";
            }
            close_connection(sockfd);
        } else if (input == "enter_library") {
            if (!isLogged) {
                cout << "Error: The user is not logged in!\n";
                continue;
            }
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char *msg = compute_get_request("34.246.184.49", "/api/v1/tema/library/access", 
                NULL, sessionCookie.c_str(), NULL);
            send(sockfd, msg, strlen(msg), 0);
            free(msg);
            char *res = receive_from_server(sockfd);
            response.assign(res);
            free(res);
            size_t lastNewline = response.find_last_of("\n");
            response = response.substr(lastNewline + 1);
            json result = json::parse(response);
            if (result.contains("token")) {
                cout << "User successfully got access!\n";
                sessionJWT.assign(result["token"]);
                hasAccess = true;
            } else {
                cout << "Error: " << result["error"] << "\n";
            }
            close_connection(sockfd);
        } else if (input == "get_books") {
            if (!isLogged) {
                cout << "Error: The user is not logged in!\n";
                continue;
            }
            if (!hasAccess) {
                cout << "Error: The user doesn't have access to the library!\n";
                continue;
            }
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char *msg = compute_get_request("34.246.184.49", "/api/v1/tema/library/books", 
                NULL, sessionCookie.c_str(), sessionJWT.c_str());
            send(sockfd, msg, strlen(msg), 0);
            free(msg);
            char *res = receive_from_server(sockfd);
            response.assign(res);
            free(res);
            size_t jsonFinder = response.find_last_of("[");
            string result = response.substr(jsonFinder);
            if (jsonFinder < response.size()) {
                cout << json::parse(result).dump(4) << "\n";
            } else {
                cout << "Error: " << "Unexpected error occured!" << "\n";
            }
            close_connection(sockfd);
        } else if(input == "add_book") {
            if (!isLogged) {
                cout << "Error: The user is not logged in!\n";
                continue;
            }
            if (!hasAccess) {
                cout << "Error: The user doesn't have access to the library!\n";
                continue;
            }
            string title, author, genre, publisher, page_count;
            int page;
            // For trailing whitespaces
            cout << "title=";
            getline(cin, title);
            cout << "author=";
            getline(cin, author);
            cout << "genre=";
            getline(cin, genre);
            cout << "publisher=";
            getline(cin, publisher);
            cout << "page_count=";
            getline(cin, page_count);
            try {
                page = stoi(page_count);
            } catch (const invalid_argument &e) {
                cout << "Error: Invalid data from input!\n";
                continue;
            }

            json userCredentials;

            userCredentials["title"] = title;
            userCredentials["author"] = author;
            userCredentials["genre"] = genre;
            userCredentials["page_count"] = page;
            userCredentials["publisher"] = publisher;

            string payload = userCredentials.dump();
            
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char *msg = compute_post_request("34.246.184.49", "/api/v1/tema/library/books",
                "application/json", payload.c_str(), sessionCookie.c_str(),
                sessionJWT.c_str());
            send(sockfd, msg, strlen(msg), 0);
            free(msg);
            char *res = receive_from_server(sockfd);
            response.assign(res);
            free(res);
            size_t lastNewline = response.find_last_of("\n");
            string result = response.substr(lastNewline + 1);
            if (result == "ok") {
                cout << "Book successfully added!" << "\n";
            } else {
                cout << "Error: " << json::parse(result)["error"] << "\n";
            }
        } else if (input == "get_book") {
            if (!isLogged) {
                cout << "Error: The user is not logged in!\n";
                continue;
            }
            if (!hasAccess) {
                cout << "Error: The user doesn't have access to the library!\n";
                continue;
            }
            string idStr;
            cout << "id=";
            getline(cin, idStr);
            try {
                stoi(idStr);
            } catch (const invalid_argument &e) {
                cout << "Error: Invalid id: " << idStr << "\n";
                continue;
            }
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char *msg = compute_get_request("34.246.184.49", "/api/v1/tema/library/books", 
                idStr.c_str(), sessionCookie.c_str(), sessionJWT.c_str());
            send(sockfd, msg, strlen(msg), 0);
            free(msg);
            char *res = receive_from_server(sockfd);
            response.assign(res);
            free(res);
            size_t jsonFinder = response.find_last_of("{");
            json result = json::parse(response.substr(jsonFinder));
            if (!result.contains("error")) {
                cout << result.dump(4) << "\n";
            } else {
                cout << "Error: " << result["error"] << "\n";
            }
            close_connection(sockfd);
        } else if (input == "delete_book") {
            if (!isLogged) {
                cout << "Error: The user is not logged in!\n";
                continue;
            }
            if (!hasAccess) {
                cout << "Error: The user doesn't have access to the library!\n";
                continue;
            }
            string idStr;
            cout << "id=";
            getline(cin, idStr);
            try {
                stoi(idStr);
            } catch (const invalid_argument &e) {
                cout << "Error: Invalid id: " << idStr << "\n";
                continue;
            }
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            char *msg = compute_delete_request("34.246.184.49", "/api/v1/tema/library/books", 
                idStr.c_str(), sessionCookie.c_str(), sessionJWT.c_str());
            send(sockfd, msg, strlen(msg), 0);
            free(msg);
            char *res = receive_from_server(sockfd);
            response.assign(res);
            free(res);
            size_t lastNewline = response.find_last_of("\n");
            string result = response.substr(lastNewline + 1);
            if (result == "ok") {
                cout << "Book successfully deleted" << "\n";
            } else {
                cout << "Error: " << json::parse(result)["error"] << "\n";
            }
            close_connection(sockfd);
        } else {
            cout << "Error: Invalid input command!\n";
        }
    }

    return 0;
}
