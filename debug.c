#include "abduco.h"
#ifdef NDEBUG
void debug(const char *errstr, ...) { }
void print_packet(const char *prefix, Packet *pkt) { }
#else

void debug(const char *errstr, ...) {
	va_list ap;
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
}

void print_packet(const char *prefix, Packet *pkt) {
	static const char *msgtype[] = {
		[MSG_CONTENT] = "CONTENT",
		[MSG_ATTACH]  = "ATTACH",
		[MSG_DETACH]  = "DETACH",
		[MSG_RESIZE]  = "RESIZE",
		[MSG_EXIT]    = "EXIT",
		[MSG_PID]     = "PID",
	};
	const char *type = "UNKNOWN";
	if (pkt->type < countof(msgtype) && msgtype[pkt->type])
		type = msgtype[pkt->type];

	fprintf(stderr, "%s: %s ", prefix, type);
	switch (pkt->type) {
	case MSG_CONTENT:
		fwrite(pkt->u.msg, pkt->len, 1, stderr);
		break;
	case MSG_RESIZE:
		fprintf(stderr, "%"PRIu16"x%"PRIu16, pkt->u.ws.cols, pkt->u.ws.rows);
		break;
	case MSG_ATTACH:
		fprintf(stderr, "readonly: %d low-priority: %d",
			pkt->u.flags & CLIENT_READONLY,
			pkt->u.flags & CLIENT_LOWPRIORITY);
		break;
	case MSG_EXIT:
		fprintf(stderr, "status: %"PRIu32, pkt->u.pid);
		break;
	case MSG_PID:
		fprintf(stderr, "pid: %"PRIu32, pkt->u.pid);
		break;
	default:
		fprintf(stderr, "len: %"PRIu32, pkt->len);
		break;
	}
	fprintf(stderr, "\n");
}

#endif /* NDEBUG */
