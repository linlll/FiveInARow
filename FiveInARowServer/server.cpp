#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <unordered_map>

using namespace std;

#define SRV_PORT 8888
#define OPEN_MAX 5000

void sys_erro(const char *err) {
    perror(err);
    exit(0);
}

int main(int argc, char const *argv[])
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) sys_erro("socket error");

    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    struct sockaddr_in srv_addr;
    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SRV_PORT);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(lfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (ret == -1) sys_erro("bind error");

    ret = listen(lfd, 128);
    if (ret == -1) sys_erro("listen error");

    int efd = epoll_create(OPEN_MAX);
    if (ret == -1) sys_erro("epoll_create error");

    struct epoll_event tep, ep[OPEN_MAX]; // temp epoll event
    tep.events = EPOLLIN;
    tep.data.fd = lfd;

    ret = epoll_ctl(efd, EPOLL_CTL_ADD, lfd, &tep);
    if (ret == -1) sys_erro("epoll_ctl error");

    unordered_map<int, int> rooms;
    while (1) {
        int n = epoll_wait(efd, ep, OPEN_MAX, -1);
        if (n == -1) sys_erro("epoll_wait error");
        for (int i = 0; i < n; i++) {
            if (!ep[i].events & EPOLLIN)
                continue;
            
            if (ep[i].data.fd == lfd) {
                struct sockaddr_in clt_addr;
                socklen_t clt_addr_len = sizeof(clt_addr);
                int cfd = accept(lfd, (struct sockaddr *)&clt_addr, &clt_addr_len);
                if (cfd == -1) sys_erro("accept error");

                char str[INET_ADDRSTRLEN];
                cout << "received from " << inet_ntop(AF_INET, &clt_addr.sin_addr.s_addr, str, INET_ADDRSTRLEN) \
                     << " at port " << ntohs(clt_addr.sin_port) << endl;

                if (rooms.find(cfd) == rooms.end()) {
                    bool find = false;
                    for (auto it = rooms.begin(); it != rooms.end(); it++) {
                        if (it->second == -1) {
                            find = true;
                            rooms[it->first] = cfd;
                            rooms[cfd] = it->first;
                            char buf[1024];
                            memset(buf, '\0', sizeof(buf));
                            strcpy(buf, "you first\n");
                            write(cfd, buf, strlen(buf));
                            strcpy(buf, "you second\n");
                            write(rooms[cfd], buf, strlen(buf));
                            break;
                        }
                    }
                    if (!find) rooms[cfd] = -1;
                }

                tep.events = EPOLLIN;
                tep.data.fd = cfd;
                ret = epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &tep);
                if (ret == -1) sys_erro("epoll_ctl error");
            } else {
                char buf[1024];
                memset(buf, '\0', sizeof(buf));
                int fd = ep[i].data.fd;
                int n = read(fd, buf, sizeof(buf));
                if (n == -1) {
                    perror("read error");
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
                    if (ret == -1) sys_erro("epoll_ctl error");
                    int f = rooms[fd];
                    rooms.erase(fd);
                    rooms.erase(f);
                    close(fd);
                } else if (n == 0) {
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
                    if (ret == -1) sys_erro("epoll_ctl error");
                    int f = rooms[fd];
                    rooms.erase(fd);
                    rooms.erase(f);
                    close(fd);
                    cout << "client[" << fd << "] closed connection" << endl;
                } else {
                    write(rooms[fd], buf, strlen(buf));
                }
            }
        }
    }

    close(lfd);
    close(efd);

    return 0;
}
