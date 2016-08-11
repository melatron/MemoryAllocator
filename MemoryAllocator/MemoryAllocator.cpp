#include "MemoryAllocator.h"
#include <iostream>

const int BUFFER_SIZE = 1000000;
const int SPLIT_THRESHOLD = 40;
const int MIN_SPACE_ALLOCATED = sizeof(void*) * 2;
const size_t headerSize = sizeof(info_header);

MemoryAllocator::MemoryAllocator() : m_buffer(new char[BUFFER_SIZE])
{
	init();
}

MemoryAllocator::MemoryAllocator(const MemoryAllocator& other) : MemoryAllocator()
{}

MemoryAllocator& MemoryAllocator::operator=(const MemoryAllocator & rhs)
{
	return *this;
}

MemoryAllocator::~MemoryAllocator()
{
	delete [] m_buffer;
}

void * MemoryAllocator::allocate(size_type n)
{
	// Checks if the allocated space is bigger than the lenght of the node struct, if not make it.
	if (n < MIN_SPACE_ALLOCATED)
	{
		n = MIN_SPACE_ALLOCATED;
	}
	
	void* result = nullptr;
	
	//if (!m_freeList)
	//{
	//	return result;
	//}

	//node* currentNode = m_freeList;
	//char* c_currentHeader = reinterpret_cast<char*>(currentNode) - headerSize;
	
	char* c_currentHeader = m_buffer;

	info_header* currentHeader = reinterpret_cast<info_header*>(c_currentHeader);


	//Iterate through the free list searching for memory.
	//while (currentNode->next && c_currentHeader < (m_buffer + BUFFER_SIZE) && (!currentHeader->m_isFree || currentHeader->m_amount < n))
	//{
	//	currentNode = currentNode->next;
	//	c_currentHeader = reinterpret_cast<char*>(currentNode) - headerSize;
	//	currentHeader = reinterpret_cast<info_header*>(c_currentHeader);
	//}

	while (c_currentHeader < (m_buffer + BUFFER_SIZE) && (!currentHeader->m_isFree || currentHeader->m_amount < n))
	{
		c_currentHeader += currentHeader->m_amount + (headerSize * 2);
		currentHeader = reinterpret_cast<info_header*>(c_currentHeader);
	}


	if (c_currentHeader < (m_buffer + BUFFER_SIZE))
	{
		if (currentHeader->m_amount > n + SPLIT_THRESHOLD)
		{
			info_header* newBegin = reinterpret_cast<info_header*>(c_currentHeader + (headerSize * 2) + n);
			info_header* newEnd = reinterpret_cast<info_header*>(c_currentHeader + headerSize + currentHeader->m_amount);
			info_header* end = reinterpret_cast<info_header*>(c_currentHeader + headerSize + n);
			
			//node* newNode = reinterpret_cast<node*>(c_currentHeader + (headerSize * 3) + n);
			//this->addNode(newNode);
			//this->removeNode(currentNode);

			newBegin->m_amount = currentHeader->m_amount - n - (2 * headerSize);
			newBegin->m_isFree = true;
			newEnd->m_amount = currentHeader->m_amount - n - (2 * headerSize);
			newEnd->m_isFree = true;
			end->m_amount = n;
			end->m_isFree = false;
			currentHeader->m_amount = n;
			currentHeader->m_isFree = false;
		}
		else 
		{
			//this->removeNode(currentNode);

			info_header* end = reinterpret_cast<info_header*>(c_currentHeader + headerSize + currentHeader->m_amount);
			end->m_isFree = false;
			currentHeader->m_isFree = false;
		}

		result = c_currentHeader + headerSize;
	}

	return result;
}

void MemoryAllocator::deallocate(void* pointer)
{
	info_header* begin = static_cast<info_header*>(pointer) - 1;
	char* c_begin = reinterpret_cast<char*>(begin);
	
	if (begin->m_amount == 0)
	{
		return;
	}

	info_header* end = reinterpret_cast<info_header*>(c_begin + headerSize + begin->m_amount);
	char* c_end = reinterpret_cast<char*>(end);

	begin->m_isFree = true;
	end->m_isFree = true;
	
	//node* currentNode = reinterpret_cast<node*>(c_begin + headerSize);
	//addNode(currentNode);

	if (c_begin - headerSize > m_buffer)
	{
		info_header* leftEnd = reinterpret_cast<info_header*>(c_begin - headerSize);
		char* c_leftEnd = reinterpret_cast<char*>(leftEnd);

		if (leftEnd->m_isFree)
		{
			info_header* leftBegin = reinterpret_cast<info_header*>(c_leftEnd - leftEnd->m_amount - headerSize);

			leftBegin->m_amount = begin->m_amount + leftBegin->m_amount + (headerSize * 2);
			end->m_amount = leftBegin->m_amount;
			begin = leftBegin;
			//removeNode(currentNode);

			//currentNode = reinterpret_cast<node*>(reinterpret_cast<char*>(leftBegin) + headerSize);
		}
	}

	// Merging case current memory block with right free memory block
	if (c_end + headerSize < (m_buffer + BUFFER_SIZE))
	{
		info_header* rightBegin = reinterpret_cast<info_header*>(c_end + headerSize);
		char* c_rightBegin = reinterpret_cast<char*>(rightBegin);

		if (rightBegin->m_isFree)
		{
			info_header* rightEnd = reinterpret_cast<info_header*>(c_rightBegin + rightBegin->m_amount + headerSize);
			
			//node* rightNode = reinterpret_cast<node*>(c_rightBegin + headerSize);
			//removeNode(rightNode);
			//addNode(currentNode);

			rightEnd->m_amount = end->m_amount + rightEnd->m_amount + (headerSize * 2);
			begin->m_amount = rightEnd->m_amount;
		}
	}

}

int MemoryAllocator::getFreeCells() const
{
	int result = 0;
	info_header* currentHeader;
	char* c_currentHeader = m_buffer;

	do
	{
		currentHeader = reinterpret_cast<info_header*>(c_currentHeader);

		if (reinterpret_cast<char*>(currentHeader) < (m_buffer + BUFFER_SIZE) && currentHeader->m_isFree)
		{
			result++;
		}

		c_currentHeader = c_currentHeader + currentHeader->m_amount + 2 * headerSize;

	} while (reinterpret_cast<char*>(currentHeader) < (m_buffer + BUFFER_SIZE));


	return result;
}

int MemoryAllocator::getUsedCells() const
{
	int result = 0;
	info_header* currentHeader;
	char* c_currentHeader = m_buffer;
	
	do
	{
		currentHeader = reinterpret_cast<info_header*>(c_currentHeader);

		if (reinterpret_cast<char*>(currentHeader) < (m_buffer + BUFFER_SIZE) && !currentHeader->m_isFree)
		{
			result++;
		}

		c_currentHeader = c_currentHeader + currentHeader->m_amount + 2 * headerSize;
	} 
	while (reinterpret_cast<char*>(currentHeader) < (m_buffer + BUFFER_SIZE));


	return result;
}

int MemoryAllocator::getUsedAmount() const
{
	int result = 0;
	info_header* currentHeader;
	char* c_currentHeader = m_buffer;

	do
	{
		currentHeader = reinterpret_cast<info_header*>(c_currentHeader);

		if (reinterpret_cast<char*>(currentHeader) < (m_buffer + BUFFER_SIZE) && !currentHeader->m_isFree)
		{
			result += currentHeader->m_amount + (2 * headerSize);
		}

		c_currentHeader = c_currentHeader + currentHeader->m_amount + 2 * headerSize;

	} while (reinterpret_cast<char*>(currentHeader) < (m_buffer + BUFFER_SIZE));


	return result;
}

void MemoryAllocator::print() const
{

}

void MemoryAllocator::init()
{
	int totalSizeLeft = BUFFER_SIZE - (2 * headerSize);
	info_header* head = reinterpret_cast<info_header*>(m_buffer);
	info_header* tail = reinterpret_cast<info_header*>(m_buffer + BUFFER_SIZE - headerSize);

	head->m_amount = totalSizeLeft;
	head->m_isFree = true;

	//node* freeList = reinterpret_cast<node*>(m_buffer + headerSize);
	//freeList->previous = nullptr;
	//freeList->next = nullptr;

	//m_freeList = freeList;

	tail->m_amount = totalSizeLeft;
	tail->m_isFree = true;
}

void MemoryAllocator::addNode(node* freed)
{
	freed->previous = nullptr;
	freed->next = m_freeList;
	freed->next->previous = freed;
	m_freeList = freed;

	if (!freeListCheck()) 
	{
		int a = 0;
	}
}

void MemoryAllocator::removeNode(node* used)
{

	if (used->next) {
		if (used->previous) {
			used->previous->next = used->next;
			used->next->previous = used->previous;
		}
		else {
			used->next->previous = nullptr;
			m_freeList = used->next;
		}
	}
	else if (used->previous)
	{
		used->previous->next = nullptr;
	}
	else
	{
		m_freeList = nullptr;
	}

	if (!freeListCheck())
	{
		int a = 0;
	}
}

bool MemoryAllocator::freeListCheck()
{
	bool result = true;

	node* current = m_freeList;
	node* previous = nullptr;
	int counter = 0;

	while (current)
	{
		if ((counter != 0 && current->previous != previous) || (counter > 100))
		{
			result = false;
			break;
		}

		counter++;
		previous = current;
		current = current->next;
	}

	std::cout << "Free list length: " << counter << std::endl;

	return result;
}
