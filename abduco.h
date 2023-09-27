#ifndef ABDUCO_H
#define ABDUCO_H
#include <stdbool.h>


#if defined CTRL && defined _AIX
  #undef CTRL
#endif
#ifndef CTRL
  #define CTRL(k)   ((k) & 0x1F)
#endif

#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <signal.h>
#include <libgen.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/queue.h>
#if defined(__linux__) || defined(__CYGWIN__)
# include <pty.h>
#elif defined(__FreeBSD__) || defined(__DragonFly__)
# include <libutil.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
# include <util.h>
#endif

#if defined(_AIX)
# include "forkpty-aix.c"
#elif defined(__sun)
# include "forkpty-sunos.c"
#endif

#define countof(arr) (sizeof(arr) / sizeof((arr)[0]))

enum PacketType {
	MSG_CONTENT = 0,
	MSG_ATTACH  = 1,
	MSG_DETACH  = 2,
	MSG_RESIZE  = 3,
	MSG_EXIT    = 4,
	MSG_PID     = 5,
};

typedef struct {
	uint32_t type;
	uint32_t len;
	union {
		char msg[4096 - 2*sizeof(uint32_t)];
		struct {
			uint16_t rows;
			uint16_t cols;
		} ws;
		uint32_t i;
		uint64_t l;
	} u;
} Packet;

typedef struct Client Client;
struct Client {
	int socket;
	enum {
		STATE_CONNECTED,
		STATE_ATTACHED,
		STATE_DETACHED,
		STATE_DISCONNECTED,
	} state;
	bool need_resize;
	enum {
		CLIENT_READONLY = 1 << 0,
		CLIENT_LOWPRIORITY = 1 << 1,
	} flags;
	Client *next;
};

struct entry {
	char *data;
	int len;
	bool complete;
	TAILQ_ENTRY(entry) entries;
};

TAILQ_HEAD(screenhead, entry);

typedef struct {
	Client *clients;
	int socket;
	int pty;
	int exit_status;
	struct termios term;
	struct winsize winsize;
	struct screenhead screen;
	int screen_rows;
	pid_t pid;
	volatile sig_atomic_t running;
	const char *name;
	const char *session_name;
	char host[255];
	bool read_pty;
} Server;

extern Server server;
extern Client client;
extern struct termios orig_term, cur_term;
extern bool has_term, alternate_buffer, quiet, passthrough;
extern int screen_max_rows;

static struct sockaddr_un sockaddr = {
	.sun_family = AF_UNIX,
};

void client_setup_terminal(void);
int client_mainloop(void);
void client_restore_terminal(void);
void client_sigwinch_handler(int sig);
int server_set_socket_non_blocking(int sock);
void server_mainloop(void);
void server_sigterm_handler(int sig);
void server_sigusr1_handler(int sig);
void server_pty_died_handler(int sig);
bool client_recv_packet(Packet *pkt);
bool send_packet(int socket, Packet *pkt);
bool recv_packet(int socket, Packet *pkt);
int server_create_socket(const char *name);
bool set_socket_name(struct sockaddr_un *sockaddr, const char *name);
void die(const char *s);
void info(const char *str, ...);
ssize_t write_all(int fd, const char *buf, size_t len);

void debug(const char *errstr, ...);
void print_packet(const char *prefix, Packet *pkt);


#endif
