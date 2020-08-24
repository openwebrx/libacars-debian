/*
 *  This file is a part of libacars
 *
 *  Copyright (c) 2018-2020 Tomasz Lemiech <szpajder@gmail.com>
 */

#include <libacars/macros.h>    // la_assert
#include <libacars/list.h>      // la_list
#include <libacars/util.h>      // LA_XCALLOC, LA_XFREE

la_list *la_list_next(la_list const * const l) {
	if(l == NULL) {
		return NULL;
	}
	return l->next;
}

la_list *la_list_append(la_list *l, void *data) {
	LA_NEW(la_list, new);
	new->data = data;
	if(l == NULL) {
		return new;
	} else {
		la_list *ptr;
		for(ptr = l; ptr->next != NULL; ptr = la_list_next(ptr))
			;
		ptr->next = new;
		return l;
	}
}

size_t la_list_length(la_list const *l) {
	size_t len = 0;
	for(; l != NULL; l = la_list_next(l), len++)
		;
	return len;
}

void la_list_foreach(la_list *l, void (*cb)(), void *ctx) {
	la_assert(cb);
	while(l != NULL) {
		cb(l->data, ctx);
		l = la_list_next(l);
	}
}

void la_list_free_full_with_ctx(la_list *l, void (*node_free)(), void *ctx) {
	if(l == NULL) {
		return;
	}
	la_list_free_full_with_ctx(l->next, node_free, ctx);
	l->next = NULL;
	if(node_free != NULL) {
		node_free(l->data, ctx);
	} else {
		LA_XFREE(l->data);
	}
	LA_XFREE(l);
}

void la_list_free_full(la_list *l, void (*node_free)()) {
	if(l == NULL) {
		return;
	}
	la_list_free_full(l->next, node_free);
	l->next = NULL;
	if(node_free != NULL) {
		node_free(l->data);
	} else {
		LA_XFREE(l->data);
	}
	LA_XFREE(l);
}

void la_list_free(la_list *l) {
	la_list_free_full(l, NULL);
}
