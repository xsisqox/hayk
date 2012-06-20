#ifndef __CRB_WS_H__
#define __CRB_WS_H__ 1

#include "crb_buffer.h"

#define CRB_PARSE_DONE					1
#define CRB_VALIDATE_DONE				1
#define CRB_PARSE_INCOMPLETE			10
#define CRB_ERROR_INVALID_METHOD		11
#define CRB_ERROR_INVALID_REQUEST		12
#define CRB_ERROR_INVALID_OPCODE		13

#define CRB_WS_CONT_FRAME	0
#define CRB_WS_TEXT_FRAME	1
#define CRB_WS_BIN_FRAME	2
#define CRB_WS_CLOSE_FRAME	8
#define CRB_WS_PING_FRAME	9
#define CRB_WS_PONG_FRAME	10

typedef struct crb_ws_frame_s crb_ws_frame_t;
struct crb_ws_frame_s {
	uint64_t payload_len;
	uint8_t opcode;
	union {
		uint32_t raw;
		uint8_t octets[4];
	} mask;
	char *data;
	uint64_t data_length;
	
	unsigned is_masked:1;
};

crb_ws_frame_t *crb_ws_frame_init();
int crb_ws_frame_parse_buffer(crb_ws_frame_t *frame, crb_buffer_t *buffer);
crb_ws_frame_t *crb_ws_frame_create_from_data(char *data, uint64_t data_length, int masked);
uint8_t *crb_ws_frame_head_from_data(uint8_t *data, uint64_t data_length, int *length, int masked);
void crb_ws_frame_free(crb_ws_frame_t *frame);


#endif /* __CRB_WS_H__ */