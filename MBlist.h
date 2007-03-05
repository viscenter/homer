#ifndef _MBlist_
#define _MBlist_

#include <stdio.h>
/*

   A really simple linked list class.  Convenient for those days you need a list of sorts.
   Add data is passed in as a void pointer

    You can insert a node (to the top . . push)
    You can delete a node (from the top . . pop)
    You can search the list (passing in a compare function)

*/

class MBlist
{

public:

	struct node
	{
		void *data;
		node *next;
	};	

	// define iterator class here
	class MBlistIterator
	{

	public:

		node *iterHead;
		MBlistIterator(node *_head) { iterHead = _head; }

		void *getData() { return iterHead->data; }
		bool next()    
		{ 

			if (iterHead->next == NULL)
				return false;

			iterHead = iterHead->next;
			return true;
		}

		bool empty() 
		{ 
			if (iterHead==NULL)
			{
				printf("Return true %p \n", iterHead );
				return true;
			}
			else
			{

				printf("Return false %p \n", iterHead );
				return false;
			}

		}

	};


	node *head, *tail;
	int size;
	MBlist()       { head = NULL; size = 0; }

	node *insertNode(void *data) 
	{
		node *newNode = new node;
		newNode->data = data;
		newNode->next = head;
		head = newNode;
		size++;
		return (newNode);
	}

	void *findNode(void *data, bool compareFunc(void *, void *) )
	{
		if (size == 0)
			return NULL;

		node *t = head;
		while( t )
		{
			if ( compareFunc(data, t->data) )
				return t->data;

			t = t->next;
		}
		return NULL;
	}

	void *removeNode()
	{
		if (!head)
			return NULL;

		node *t = head;
		head = head->next;
		size--;
		return t->data;
	}

	int getSize() { return size; }
	MBlistIterator iterator() { return MBlistIterator(head); }
};

#endif
