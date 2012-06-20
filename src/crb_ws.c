#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "crb_ws.h"
#include "crb_buffer.h"


static void
bitprint(uint8_t *data, int len)
{
	uint8_t *start = data;
	uint8_t x, y, i, z;
	for (y = 0; y < len; y += 1) {
		i = start[y];
		printf("%i-", i);
		for (x = 0; x < 8; x ++) {
			z = 48 + ((i >> x)&1);
			printf("%c", z);
		}
		printf("\n");
	}
}

crb_ws_frame_t *
crb_ws_frame_init()
{
	crb_ws_frame_t *frame;
	
	frame = malloc(sizeof(crb_ws_frame_t));
	if ( frame == NULL ) {
		return NULL;
	}
	
	frame->payload_len = 0;
	frame->opcode = 0;
	frame->mask.raw = 0;
	
	frame->is_masked = 0;
	
	frame->data = NULL;
	frame->data_length = 0;
	
	return frame;
}

crb_ws_frame_t *
crb_ws_frame_create_from_data(char *data, uint64_t data_length, int masked)
{
	crb_ws_frame_t *frame;
	
	if ( data == NULL || data_length <= 0 ) {
		return NULL;
	}
	
	frame = crb_ws_frame_init();
	
	frame->payload_len = data_length;
	frame->opcode = CRB_WS_TEXT_FRAME;
	
	if ( masked ) {
		frame->is_masked = 1;
		frame->mask.raw = rand();
	} else {
		frame->is_masked = 0;
	}
	
	frame->data = data;
}

uint8_t *
crb_ws_frame_head_from_data(uint8_t *data, uint64_t data_length, int *length, int masked)
{
	uint8_t *pos;
	
	uint64_t payload_len;
	uint8_t opcode;
	union {
		uint32_t raw;
		uint8_t octets[4];
	} mask;
	int is_masked;
	
	// define frame properties 
	payload_len = data_length;
	opcode = CRB_WS_TEXT_FRAME;
	
	if ( masked ) {
		is_masked = 1;
		mask.raw = rand();
	} else {
		is_masked = 0;
	}
	
	
	// define data length
	*length = 2;
	
	if ( payload_len < 126 ) {
		// pass
	} else if (payload_len <= 65536 ) {
		*length += 2;
	} else {
		*length += 8;
	}
	
	if ( is_masked ) {
		 *length += 4;
	}
	
	data = malloc(*length * sizeof(uint8_t));
	pos = data;
	
	// Opcode, rsv, fin
	*pos = (opcode&15) | 0b10000000;
	pos += 1;
	
	// Payload length
	if ( payload_len < 126 ) {
		*pos = (payload_len&127) | (is_masked << 7);
	} else if (payload_len <= 65536 ) {
		*pos = 126 | (is_masked << 7);
		pos += 1;
		*(uint16_t*)pos = payload_len;
		pos += 2;
	} else {
		*pos = 127 | (is_masked << 7);
		pos += 1;
		*((uint64_t*)pos) = (uint64_t)payload_len;
		pos += 8;
	}
	
	// Mask key
	
	if ( is_masked ) {
		*((uint32_t*)pos) = mask.raw;
		pos += 4;
	}
	
	return data;
}


int
crb_reader_parse_frame(crb_ws_frame_t *frame, crb_buffer_t *buffer)
{
	uint16_t raw;
	char *read_pos;
	
	read_pos = buffer->rpos;
	
	printf("Reading frame...\n");
	
	if ( buffer->used < 2 ) {
		printf("Incomplete %i.\n", buffer->used);
		return CRB_PARSE_INCOMPLETE;
	}
	
	
	raw = * (uint16_t*) read_pos;
	
	// Opcode
	switch( raw & 15 ) {
		case CRB_WS_CONT_FRAME: frame->opcode = CRB_WS_CONT_FRAME; break;
		case CRB_WS_TEXT_FRAME: frame->opcode = CRB_WS_TEXT_FRAME; break;
		case CRB_WS_BIN_FRAME: frame->opcode = CRB_WS_BIN_FRAME; break;
		case CRB_WS_CLOSE_FRAME: frame->opcode = CRB_WS_CLOSE_FRAME; break;
		case CRB_WS_PING_FRAME: frame->opcode = CRB_WS_PING_FRAME; break;
		case CRB_WS_PONG_FRAME: frame->opcode = CRB_WS_PONG_FRAME; break;
		default:
			printf("Uknown opcode!\n");
			return CRB_ERROR_INVALID_OPCODE;
	}
	
	// Mask
	if ( raw & 256 == 0 ) {
		frame->is_masked = 0; 
	} else {
		frame->is_masked = 1; 
	}
	
	bitprint((uint8_t*)&raw, 2);
	
	// Payload Length
	frame->payload_len = (raw >> 8)&127;
	printf("Payload: %i\n", frame->payload_len);
	
	if ( frame->payload_len == 126 ) {
		// 16bit length
		// require the next 2 bytes
		if ( buffer->used < 4 ) {
			printf("Incomplete %i.\n", buffer->used);
			return CRB_PARSE_INCOMPLETE;
		}
		
		frame->payload_len = ntohs(*(uint16_t *) (read_pos + 2));
		read_pos = read_pos + 4; 
	} else if( frame->payload_len == 127 ) {
		// 64bit length
		// require the next 8 bytes
		if ( buffer->used < 10 ) {
			printf("Incomplete %i.\n", buffer->used);
			return CRB_PARSE_INCOMPLETE;
		}
		
		frame->payload_len = ntohl(* (uint32_t *) (read_pos + 2));
		frame->payload_len <<= 32;
		frame->payload_len += ntohl(* (uint32_t *) (read_pos + 6));
		read_pos = read_pos + 8; 
	} else {
		read_pos = read_pos + 2; 
	}
	
	// Masking key
	if ( frame->is_masked ) {
		if ( buffer->used < (read_pos + 4) - buffer->ptr ) {
			printf("Incomplete for Mask %i, required %i.\n", buffer->used, (read_pos + 4) - buffer->ptr);
			return CRB_PARSE_INCOMPLETE;
		}
	
		frame->mask.raw = * (uint32_t *) read_pos;
		read_pos = read_pos + 4;
	}
	
	// Payload
	if ( buffer->used < (read_pos + frame->payload_len) - buffer->ptr ) {
		printf("Incomplete for Payload %i, required %i.\n", buffer->used, (read_pos + frame->payload_len) - buffer->ptr);
		return CRB_PARSE_INCOMPLETE;
	}
	
	// Unmask
	{
		int i, j;
		uint8_t ch;
		
		for (i = 0; i < frame->payload_len; i += 1) {
			j = i%4;
			ch = (*(u_char*)(read_pos+i))^frame->mask.octets[j];
			*(read_pos+i) = ch;
		}
		
		frame->data = malloc(frame->payload_len + 1);
		frame->data = memcpy(frame->data, read_pos, frame->payload_len);
		frame->data_length = frame->payload_len;
		frame->data[frame->data_length] = '\0';
	}
	
	read_pos = read_pos + frame->payload_len;
	buffer->rpos = read_pos;
	
	printf("DATA: %s\n", frame->data);
	printf("Frame read.\n");
	
	return CRB_PARSE_DONE;
}

void 
crb_ws_frame_free(crb_ws_frame_t *frame)
{
	free(frame);
}