#ifndef __CRB_REQUEST_H__
#define __CRB_REQUEST_H__ 1

#include <stdint.h>

#include "crb_buffer.h"
#include "crb_hash.h"

typedef struct crb_header_s crb_header_t;
struct crb_header_s {
    char *name;
    char *value;
};

typedef struct crb_request_s crb_request_t;

struct crb_request_s {
    char *uri;
    crb_hash_t *headers;
	
	int ref;
};

crb_header_t *crb_header_init();
void crb_header_free(crb_header_t *header);

crb_request_t *crb_request_init();
void crb_request_set_uri(crb_request_t *request, char *uri, ssize_t length);
void crb_request_add_header(crb_request_t *request, char *name, char *value);
void crb_request_ref(crb_request_t *request);
void crb_request_unref(crb_request_t *request);

#endif /* __CRB_REQUEST_H__ */
