#include <stdio.h>
#include <stdlib.h>

struct node {
  int payload;
  struct node *next;
};

void create_node(struct node **nodepp, int data) {

  struct node *nodeptr = (struct node *)malloc(sizeof(struct node));

  (*nodepp) = nodeptr;
  (*nodepp)->payload = data;
  (*nodepp)->next = NULL;
  /*    printf("New node has address %p\n", nodeptr); */
  return;
}

void show_list(struct node *nptr) {

  static int j = 0;

  /* done */
  if (!nptr)
    return;

  printf("Node %d at %p has payload %d\n", j, nptr, nptr->payload);
  nptr = nptr->next;
  j++;

  show_list(nptr);
}

void add_node(struct node *newnode, struct node **tail) {

  if (*tail == NULL) {
    *tail = newnode;
    return;
  }

  /* set *listp to tail */
  while ((*tail)->next)
    (*tail) = (*tail)->next;

  /* add nptr after current tail */
  (*tail)->next = newnode;
  /* set new tail to nptr */
  *tail = newnode;
  return;
}

void freelist(struct node **list) {

  struct node *head = *list, *save = *list;

  /* find tail */
  while ((*list)->next) {
    save = *list;
    (*list) = (*list)->next;
  }

  if (!save->next) {
    printf("Freeing %p\n", save);
    free(save);
    return;
  }

  printf("Freeing %p\n", save->next);
  free(save->next);
  *list = save;
  (*list)->next = NULL;
  freelist(&head);
}

int main(void) {

  int i;
  struct node *newnode, *list, *HEAD = NULL;

  create_node(&HEAD, 0);
  list = HEAD;

  for (i = 1; i < 5; i++) {
    create_node(&newnode, i);
    add_node(newnode, &list);
    printf("\nList is:\n");
    show_list(HEAD);
  }

  freelist(&HEAD);

  exit(0);
}
