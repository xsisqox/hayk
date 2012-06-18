#ifndef __CRB_READER_H__
#define __CRB_READER_H__ 1

#include "crb_worker.h"
#include "crb_hash.h"
#include "crb_client.h"
#include "crb_channel.h"

#define CRB_READER_CLIENTS_SCALE 4

#define CRB_PARSE_HEADER_DONE			1
#define CRB_PARSE_HEADER_INCOMPLETE		10
#define CRB_ERROR_INVALID_METHOD		11
#define CRB_ERROR_INVALID_REQUEST		12

typedef struct crb_reader_s crb_reader_t;

struct crb_reader_s {
	pthread_t thread_id;
	crb_hash_t *clients;
	unsigned client_count;
	int epoll_fd;
	unsigned running:1;
};


crb_reader_t *crb_reader_init();
void crb_reader_free(crb_reader_t *reader);

void crb_reader_run(crb_reader_t *reader);
void crb_reader_stop(crb_reader_t *reader);
void crb_reader_add_client(crb_reader_t *reader, crb_client_t *client);
void crb_reader_drop_client(crb_reader_t *reader, crb_client_t *client);
void crb_reader_drop_all_clients(crb_reader_t *reader);

#endif /* __CRB_READER_H__ */
