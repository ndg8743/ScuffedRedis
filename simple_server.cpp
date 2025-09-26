#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <vector>
#include <map>
#include <string>

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    fprintf(stderr, "[%d] %s\n", errno, msg);
    abort();
}

static void fd_set_nb(int fd) {
    errno = 0;
    int flags = fcntl(fd, F_GETFL, 0);
    if (errno) {
        die("fcntl error");
        return;
    }
    flags |= O_NONBLOCK;
    errno = 0;
    (void)fcntl(fd, F_SETFL, flags);
    if (errno) {
        die("fcntl error");
    }
}

const size_t k_max_msg = 4096;

struct Conn {
    int fd = -1;
    bool want_read = false;
    bool want_write = false;
    bool want_close = false;
    std::vector<uint8_t> incoming;
    std::vector<uint8_t> outgoing;
};

// Simple key-value store
static std::map<std::string, std::string> g_map;

static void buf_append(std::vector<uint8_t> &buf, const uint8_t *data, size_t len) {
    buf.insert(buf.end(), data, data + len);
}

static void buf_consume(std::vector<uint8_t> &buf, size_t n) {
    buf.erase(buf.begin(), buf.begin() + n);
}

// Parse command and execute
static void do_request(const uint8_t *req, uint32_t reqlen, std::vector<uint8_t> &out) {
    // Simple text protocol: "GET key" or "SET key value"
    std::string cmd((char*)req, reqlen);
    
    if (cmd.substr(0, 4) == "PING") {
        std::string resp = "PONG";
        uint32_t len = resp.size();
        buf_append(out, (uint8_t*)&len, 4);
        buf_append(out, (uint8_t*)resp.data(), len);
    }
    else if (cmd.substr(0, 3) == "GET") {
        size_t space = cmd.find(' ');
        if (space != std::string::npos) {
            std::string key = cmd.substr(4);
            std::string value = g_map.count(key) ? g_map[key] : "";
            uint32_t len = value.size();
            buf_append(out, (uint8_t*)&len, 4);
            if (len > 0) {
                buf_append(out, (uint8_t*)value.data(), len);
            }
        }
    }
    else if (cmd.substr(0, 3) == "SET") {
        size_t space1 = cmd.find(' ');
        size_t space2 = cmd.find(' ', space1 + 1);
        if (space1 != std::string::npos && space2 != std::string::npos) {
            std::string key = cmd.substr(space1 + 1, space2 - space1 - 1);
            std::string value = cmd.substr(space2 + 1);
            g_map[key] = value;
            std::string resp = "OK";
            uint32_t len = resp.size();
            buf_append(out, (uint8_t*)&len, 4);
            buf_append(out, (uint8_t*)resp.data(), len);
        }
    }
    else {
        std::string resp = "ERROR";
        uint32_t len = resp.size();
        buf_append(out, (uint8_t*)&len, 4);
        buf_append(out, (uint8_t*)resp.data(), len);
    }
}

static bool try_one_request(Conn *conn) {
    if (conn->incoming.size() < 4) {
        return false;
    }
    uint32_t len = 0;
    memcpy(&len, conn->incoming.data(), 4);
    if (len > k_max_msg) {
        msg("too long");
        conn->want_close = true;
        return false;
    }
    if (4 + len > conn->incoming.size()) {
        return false;
    }
    
    const uint8_t *request = &conn->incoming[4];
    do_request(request, len, conn->outgoing);
    buf_consume(conn->incoming, 4 + len);
    
    conn->want_write = true;
    return true;
}

static void handle_write(Conn *conn) {
    assert(conn->outgoing.size() > 0);
    ssize_t rv = write(conn->fd, &conn->outgoing[0], conn->outgoing.size());
    if (rv < 0 && errno == EAGAIN) {
        return;
    }
    if (rv < 0) {
        msg("write() error");
        conn->want_close = true;
        return;
    }
    
    buf_consume(conn->outgoing, (size_t)rv);
    if (conn->outgoing.size() == 0) {
        conn->want_read = true;
        conn->want_write = false;
    }
}

static void handle_read(Conn *conn) {
    assert(conn->incoming.size() < k_max_msg);
    
    ssize_t cap = k_max_msg - conn->incoming.size();
    conn->incoming.resize(conn->incoming.size() + cap);
    
    ssize_t rv = read(conn->fd, &conn->incoming[conn->incoming.size() - cap], cap);
    if (rv < 0 && errno == EAGAIN) {
        conn->incoming.resize(conn->incoming.size() - cap);
        return;
    }
    if (rv <= 0) {
        msg("EOF or read() error");
        conn->want_close = true;
        return;
    }
    
    conn->incoming.resize(conn->incoming.size() - cap + rv);
    assert(conn->incoming.size() <= k_max_msg);
    
    while (try_one_request(conn)) {}
}

static Conn *handle_accept(int fd) {
    struct sockaddr_in client_addr = {};
    socklen_t addrlen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
    if (connfd < 0) {
        msg("accept() error");
        return NULL;
    }
    
    fd_set_nb(connfd);
    
    Conn *conn = new Conn();
    conn->fd = connfd;
    conn->want_read = true;
    return conn;
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }
    
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(6379);
    addr.sin_addr.s_addr = ntohl(0);
    
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("bind()");
    }
    
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        die("listen()");
    }
    
    fd_set_nb(fd);
    
    std::vector<Conn *> fd2conn;
    std::vector<struct pollfd> poll_args;
    
    printf("Redis server listening on port 6379\n");
    
    while (true) {
        poll_args.clear();
        poll_args.push_back({fd, POLLIN, 0});
        
        for (Conn *conn : fd2conn) {
            if (!conn) continue;
            struct pollfd pfd = {};
            pfd.fd = conn->fd;
            pfd.events = (conn->want_read ? POLLIN : 0) | (conn->want_write ? POLLOUT : 0);
            if (pfd.events) {
                poll_args.push_back(pfd);
            }
        }
        
        int rv = poll(poll_args.data(), (nfds_t)poll_args.size(), 1000);
        if (rv < 0) {
            die("poll");
        }
        
        if (poll_args[0].revents) {
            Conn *conn = handle_accept(fd);
            if (conn) {
                if (conn->fd >= (int)fd2conn.size()) {
                    fd2conn.resize(conn->fd + 1);
                }
                fd2conn[conn->fd] = conn;
            }
        }
        
        for (size_t i = 1; i < poll_args.size(); ++i) {
            if (poll_args[i].revents) {
                Conn *conn = fd2conn[poll_args[i].fd];
                if (conn->want_read && (poll_args[i].revents & POLLIN)) {
                    handle_read(conn);
                }
                if (conn->want_write && (poll_args[i].revents & POLLOUT)) {
                    handle_write(conn);
                }
                if (conn->want_close) {
                    close(conn->fd);
                    fd2conn[conn->fd] = NULL;
                    delete conn;
                }
            }
        }
    }
    
    return 0;
}
