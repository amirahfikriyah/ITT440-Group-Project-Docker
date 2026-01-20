#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <mariadb/mysql.h>

#define PORT 5002
#define BUF_SZ 1024

const char *DB_HOST = "db";
const char *DB_USER = "root";
const char *DB_PASS = "rootpass";
const char *DB_NAME = "itt440";
const char *USER_ID = "c_user";

MYSQL* db_connect() {
    MYSQL *conn = mysql_init(NULL);
    if (!conn) return NULL;

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "[C SERVER] DB connect error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }
    return conn;
}

void db_update_points() {
    MYSQL *conn = db_connect();
    if (!conn) return;

    char q[256];
    snprintf(q, sizeof(q),
        "UPDATE scores SET points=points+1, datetime_stamp=NOW() WHERE user='%s'", USER_ID);

    if (mysql_query(conn, q)) {
        fprintf(stderr, "[C SERVER] Update error: %s\n", mysql_error(conn));
    } else {
        printf("[C SERVER] updated %s\n", USER_ID);
        fflush(stdout);
    }

    mysql_close(conn);
}

int db_get_points(char *out, size_t out_sz) {
    MYSQL *conn = db_connect();
    if (!conn) return -1;

    char q[256];
    snprintf(q, sizeof(q),
        "SELECT points, datetime_stamp FROM scores WHERE user='%s'", USER_ID);

    if (mysql_query(conn, q)) {
        fprintf(stderr, "[C SERVER] Select error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) { mysql_close(conn); return -1; }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        mysql_close(conn);
        return -1;
    }

    snprintf(out, out_sz, "user=%s points=%s updated=%s\n", USER_ID, row[0], row[1]);

    mysql_free_result(res);
    mysql_close(conn);
    return 0;
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen"); return 1;
    }

    printf("[C SERVER] listening on %d\n", PORT);
    fflush(stdout);

    time_t last_update = 0;

    while (1) {
        // update every 30 seconds
        time_t now = time(NULL);
        if (now - last_update >= 30) {
            db_update_points();
            last_update = now;
        }

        // accept with timeout (so loop can keep updating)
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(server_fd, &fds);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int r = select(server_fd + 1, &fds, NULL, NULL, &tv);
        if (r <= 0) continue;

        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) continue;

        char buf[BUF_SZ];
        memset(buf, 0, sizeof(buf));
        int n = recv(client_fd, buf, sizeof(buf)-1, 0);

        if (n > 0 && strncmp(buf, "GET", 3) == 0) {
            char out[BUF_SZ];
            if (db_get_points(out, sizeof(out)) == 0) {
                send(client_fd, out, strlen(out), 0);
            } else {
                const char *msg = "NO_RECORD\n";
                send(client_fd, msg, strlen(msg), 0);
            }
        } else {
            const char *msg = "Send GET\n";
            send(client_fd, msg, strlen(msg), 0);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
