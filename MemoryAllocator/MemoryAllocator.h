#pragma once
#include <cstddef>

typedef std::size_t size_type;

struct info_header
{
	info_header(bool isFree, size_type amount) : m_isFree(isFree), m_amount(amount) {}

	bool m_isFree;
	size_type m_amount;
};

struct node
{
	node* previous;
	node* next;
};

class MemoryAllocator
{
public:

	MemoryAllocator();
	MemoryAllocator(const MemoryAllocator&);
	MemoryAllocator& operator=(const MemoryAllocator &rhs);
	~MemoryAllocator();

	void* allocate(size_type);
	void deallocate(void*);

	int getFreeCells() const;
	int getUsedCells() const;
	int getUsedAmount() const;
	void print() const;

private:
	char* m_buffer;
	node* m_freeList;

	void init();

	void addNode(node*);
	void removeNode(node*);

	bool freeListCheck();
};