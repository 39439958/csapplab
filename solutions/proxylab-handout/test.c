#include <stdio.h>
#include "csapp.h"

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_hdr = "Proxy-Connection: close\r\n";

void doit(int client_fd);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_new_request_hdr(rio_t *rio_packet, char *new_request, char *hostname, char *port);


/* boot proxy as server get connfd from client*/
int main(int argc, char **argv) 
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); 
    	Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
		Close(connfd);                                                                                     
    }
}

/*
 * doit - handle one HTTP request/response transaction
 */
void doit(int client_fd) 
{
    int real_server_fd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t real_client, real_server;
    char hostname[MAXLINE], path[MAXLINE];
    int port;

    /* Read request line and headers */
    Rio_readinitb(&real_client, client_fd);
    if (!Rio_readlineb(&real_client, buf, MAXLINE))  	 
        return;
    sscanf(buf, "%s %s %s", method, uri, version);       
    if (strcasecmp(method, "GET")) {                     
        clienterror(client_fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }                                                    
    
    /* perpare: parse uri and build new request */
    parse_uri(uri, hostname, path, &port);
    char port_str[10];
    sprintf(port_str, "%d", port); /* port from int convert to char */
    real_server_fd = Open_clientfd(hostname, port_str);  /* real server get fd from proxy(as client) */
	if(real_server_fd < 0){
        printf("connection failed\n");
        return;
    }
    Rio_readinitb(&real_server, real_server_fd);
    
    char new_request[MAXLINE];
    sprintf(new_request, "GET %s HTTP/1.0\r\n", path);
    build_new_request_hdr(&real_client, new_request, hostname, port_str);

    /* proxy as client sent to web server */
    Rio_writen(real_server_fd, new_request, strlen(new_request));
    
    /* then proxy as server respond to real client */
    int char_nums;
    while((char_nums = Rio_readlineb(&real_server, buf, MAXLINE)))
        Rio_writen(client_fd, buf, char_nums);
}



/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

/*
 * parse_uri - parse uri to get hostname, port, path from real client
 */
void parse_uri(char *uri, char *hostname, char *path, int *port) {
    *port = 80; /* default port */
    char* ptr_hostname = strstr(uri,"//");
    /* normal uri => http://hostname:port/path */
    /* eg. uri => http://www.cmu.edu:8080/hub/index.html */
    if (ptr_hostname) 
        /* hostname_eg1. uri => http://hostname... */
        ptr_hostname += 2; 
    else
        /* hostname_eg2. uri => hostname... <= NOT "http://"*/
        ptr_hostname = uri; 
    
    char* ptr_port = strstr(ptr_hostname, ":"); 
    /* port_eg1. uri => ...hostname:port... */
    if (ptr_port) {
        *ptr_port = '\0'; /* c-style: the end of string(hostname) is '\0' */
        strncpy(hostname, ptr_hostname, MAXLINE);

        /* change default port to current port */
        /* if path not char, sscanf will automatically store the ""(null) int the path */
        sscanf(ptr_port + 1,"%d%s", port, path); 
    } 
    /* port_eg1. uri => ...hostname... <= NOT ":port"*/
    else {
        char* ptr_path = strstr(ptr_hostname,"/");
        /* path_eg1. uri => .../path */
        if (ptr_path) {
            *ptr_path = '\0';
            strncpy(hostname, ptr_hostname, MAXLINE);
            *ptr_path = '/';
            strncpy(path, ptr_path, MAXLINE);
            return;                               
        }
        /* path_eg2. uri => ... <= NOT "/path"*/
        strncpy(hostname, ptr_hostname, MAXLINE);
        strcpy(path,"");
    }
    return;
}

/*
 * build_new_request_hdr - get old request_hdr then build new request_hdr
 */
void build_new_request_hdr(rio_t *real_client, char *new_request, char *hostname, char *port){
    char temp_buf[MAXLINE];

    /* get old request_hdr */
    while(Rio_readlineb(real_client, temp_buf, MAXLINE) > 0){
        if (strstr(temp_buf, "\r\n")) break; /* read to end */

        /* if all old request_hdr had been read, we get it */
        if (strstr(temp_buf, "Host:")) continue;
        if (strstr(temp_buf, "User-Agent:")) continue;
        if (strstr(temp_buf, "Connection:")) continue;
        if (strstr(temp_buf, "Proxy Connection:")) continue;

        sprintf(new_request, "%s%s", new_request, temp_buf);
    }

    /* build new request_hdr */
    sprintf(new_request, "%sHost: %s:%s\r\n", new_request, hostname, port);
    sprintf(new_request, "%s%s%s%s", new_request, user_agent_hdr, conn_hdr, proxy_hdr);
    sprintf(new_request,"%s\r\n", new_request);
}
