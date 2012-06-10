#ifndef __CRB_LIST_H
#define __CRB_LIST_H 1

typedef struct crb_list_item_s crb_list_item_t;
struct crb_list_item_s {
	int ref;
	crb_list_item_t *prev;
	crb_list_item_t *next;
	void *data;
};

typedef struct crb_list_s crb_list_t;
struct crb_list_s {
	crb_list_item_t *first;
	crb_list_item_t *last;
	int length;
};

crb_list_t *crb_list_init();
void crb_list_push(crb_list_t *list, void *data);
void crb_list_unshift(crb_list_t *list, void *data);
void *crb_list_pop(crb_list_t *list);
//void *crb_list_shift(crb_list_t *list);
void crb_list_remove(crb_list_t *list, void *data);

void crb_list_item_ref(crb_list_item_t *item);
void crb_list_item_unref(crb_list_item_t *item);


#endif /* __CRB_LIST_H */
