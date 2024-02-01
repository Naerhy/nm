#include "nm.h"

void lklist_add(LkList** head, LkList* node)
{
	if (!(*head))
		*head = node;
	else
		lklist_last(*head)->next = node;
}

LkList* lklist_create(void* content)
{
	LkList* node;

	node = malloc(sizeof(LkList));
	if (!node)
		return NULL;
	node->content = content;
	node->next = NULL;
	return node;
}

LkList* lklist_last(LkList* head)
{
	while (head->next)
		head = head->next;
	return head;
}
