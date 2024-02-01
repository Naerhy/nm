#include "nm.h"

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

void lklist_add(LkList** head, LkList* node)
{
	if (!(*head))
		*head = node;
	else
		lklist_last(*head)->next = node;
}

void lklist_delete(LkList** head, LkList* node, void (*delete)(void*))
{
	size_t pos;

	pos = lklist_pos(*head, node);
	if (!pos)
		*head = node->next;
	else
		lklist_nth(*head, pos - 1)->next = node->next;
	delete(node->content);
	free(node);
}

void lklist_clear(LkList** head, void (*delete)(void*))
{
	while (*head)
		lklist_delete(head, *head, delete);
}

LkList* lklist_nth(LkList* head, size_t pos)
{
	for (size_t i = 0; i != pos; i++)
		head = head->next;
	return head;
}

LkList* lklist_last(LkList* head)
{
	while (head->next)
		head = head->next;
	return head;
}

size_t lklist_pos(LkList const* head, LkList const* node)
{
	size_t pos;

	pos = 0;
	while (head != node)
	{
		head = head->next;
		pos++;
	}
	return pos;
}
