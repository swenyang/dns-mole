/* query.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * $Id$
 */

#include <stdlib.h>
#include "../include/query.h"
#include "../include/error.h"

void query_empty(query * q) {
	query * p = q->next;
	
	while (q != NULL) {
		p = q->next;
		free(q->answers);
		free(q);
		q = p;
	}
}

void query_insert_before(query * q1, query * q2) {
	query * prev = q1->prev;
	if (prev != NULL) {
		prev->next = q2;
		q2->prev = prev;
		q2->next = q1;
		q1->prev = q2;
	}
	else {
		q1->prev = q2;
		q2->next = q1;
	}
}

void query_insert_after(query * q1, query * q2) {
	query * next = q1->next;
	if (next != NULL) {
		q1->next = q2;
		q2->prev = q1;
		q2->next = next;
		next->prev = q2;
	}
	else {
		q1->next = q2;
		q2->prev = q1;
	}
}

void query_remove(query * q) {
	query * prev = q->prev;
	query * next = q->next;
	prev->next = next;
	next->prev = prev;
	free(q->answers);
	free(q);
}

