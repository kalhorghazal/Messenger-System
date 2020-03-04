#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#define LOCAL_ADDRESS_IP "127.0.0.1"
#define HEARTBEAT_RATE 5
#define COMMAND_MAX_LENGTH 1024
#define EMPTY " "
#define DELIM "\n"
#define ZERO 0
#define TRUE 1
#define ONE 1
#define LOGIN_COMMAND "login"
#define GET_GROUP_LIST_COMMAND "get_group_list"
#define JOIN_GROUP_COMMAND "join"
#define CREATE_GROUP_COMMAND "create_group"
#define CHAT_SIGN ":)"
#define CHAT_SIGN_GROUP ":|"
#define START_SECRET_CHAT_COMMAND "start_secret_chat_with"
#define LEAVE_GROUP "leave_group"
#define LEAVE_SECRET_CHAT "leave_secret_chat"
#define MAXLINE 1024
#define MAX_PENDING 3
#define FALSE 0
#define NUMBER_OF_ARGUMENTS 2
#define EQUAL 0

int globalHeartbeatSocketFD;
struct sockaddr_in globalHeartbeatAddress;
int group_port = 0;
int group_socket;
struct sockaddr_in group_address;
int friend_port = 0;
int friend_socket;
int my_port = 0;
char my_username[MAXLINE];

struct sockaddr_in get_server_address(int port);
struct sockaddr_in get_broadcast_address(int port);
void bind_socket(int sockfd, struct sockaddr_in servaddr);
int get_udp_socket();
int get_tcp_socket();
void set_broadcast_options(int sockfd);
void set_reusable_option(int sockfd);
void set_reusable_option(int sockfd);
void handle_heartbeat_signal();
void start_heartbeat(int heartbeat_port);
int login_user(char* username, int server_port);
int create_group(char* group_name, int server_port);
int join_group(char* group_name, int server_port);
int start_secret_chat(char* username, int server_port);
int get_group_list(int server_port);
void parse_input(char** command, char**argument, char* line);
void broadcast(char* buffer);
int start_secret_chatting();
void start_group_chatting();
void handle_leave_group();
int handle_leave_secret_chat();
int handle_friend_chat(char* message);
int handle_group_chat(char* message);
int run_client(int server_port);
char* itoa(int val);


int max(int x, int y);
void print(char* buf);

#endif