/* 
 * OpenVirtualization: 
 * For additional details and support contact developer@sierraware.com.
 * Additional documentation can be found at www.openvirtualization.org
 * 
 * Copyright (C) 2011 SierraWare
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

/* 
 * Sierra List implementation
 */

#ifndef __LIB_LINK_H__
#define __LIB_LINK_H__

#define LINK_POISON_PREV    0xEAEAEAEA
#define LINK_POISON_NEXT    0xCACACACA

/**
 * @brief 
 */
struct link {
	struct link *next, *prev;
	void *data;
};

static inline void link_init(struct link *head)
{
	head->next = head;
	head->prev = head;
	head->data = (void*)0;	
}

/* Queue interface */
static inline void queue_init(struct link **head)
{
    *head = (void*)0;
}

static inline void set_link_data(struct link *node, void *data)
{
	node->data = data;
}

static inline struct link * get_next_node(struct link *head)
{
	if(head->next)
		return head->next;
	else 
		return (void*) 0;
}

/**
 * @brief 
 *
 * @param head
 *
 * @return 
 */
static inline int link_empty(struct link *head)
{
	return (head->next == head);
}

#define HEAD 1
#define TAIL 2
static inline void remove_link(struct link *node)
{
	struct link *next, *prev;

	next = node->next;
	prev = node->prev;
	
	prev->next = node->next;
	next->prev = node->prev;
	node->next = (void *)LINK_POISON_NEXT;
	node->prev = (void *)LINK_POISON_PREV;
}

static inline void add_link(struct link *head, struct link *new, int pos)
{
	struct link *next, *prev;
	if(pos == HEAD) {
		prev = head;
		next = head->next;
	}
	else 
	{	
		prev = head->prev;
		next = head;		
	}

	new->prev = prev;
	new->next = next;
	prev->next = new;
	next->prev = new;
}

/* Queue interface */
static inline void queue_insert_tail(struct link **head, struct link *new)
{
    if (*head != (void*)0) {
        add_link(*head, new, TAIL);
    } else {
        link_init(new);
        *head = new;
    }
}

static inline void queue_remove_head(struct link **head)
{
    struct link *l;

    l = *head;

    if (l->next == l) {
        *head = (void*)0;
    } else {
        *head = l->next;
        remove_link(l);
    }
}

static inline int queue_empty(struct link *head)
{
    return (head == (void*)0);
}

/*
static inline struct link *find_link(struct link *head, void *data)
{
	struct link *temp = 0;
	
	temp = head;
	
	if(!data)
		return (void*) 0;
			
	while (temp->next != head) {
		if(temp->data == data)
			return temp;
		temp = temp->next;
	}
	return (void*)0;
}
*/

#endif /* __LIB_LINK_H__ */
