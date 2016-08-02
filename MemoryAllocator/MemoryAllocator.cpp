#include "MemoryAllocator.h"
#include <iostream>

const int BUFFER_SIZE = 1000000;
const int SPLIT_THRESHOLD = 40;
const int MIN_SPACE_ALLOCATED = sizeof(void*) * 2;
const size_t headerSize = sizeof(info_header);

MemoryAllocator::MemoryAllocator() : m_buffer(new char[BUFFER_SIZE])
{
	int totalSizeLeft = BUFFER_SIZE - (2 * headerSize);
	info_header* head = reinterpret_cast<info_header*>(m_buffer);
	info_header* tail = reinterpret_cast<info_header*>(m_buffer + BUFFER_SIZE - headerSize);

	head->m_amount = totalSizeLeft;
	head->m_isFree = true;
	
	tail->m_amount = totalSizeLeft;
	tail->m_isFree = true;
}

MemoryAllocator::~MemoryAllocator()
{
	delete [] m_buffer;
}

void * MemoryAllocator::allocate(size_type n)
{
	if (n < MIN_SPACE_ALLOCATED)
	{
		n = MIN_SPACE_ALLOCATED;
	}

	void* result = nullptr;
	info_header* currentHeader = reinterpret_cast<info_header*>(m_buffer);
	char* c_currentHeader = m_buffer;

	while (reinterpret_cast<char*>(currentHeader) < (m_buffer + BUFFER_SIZE) && ( !currentHeader->m_isFree || currentHeader->m_amount < n ))
	{
		c_currentHeader = c_currentHeader +  currentHeader->m_amount + 2 * headerSize;
		currentHeader = reinterpret_cast<info_header*>(c_currentHeader);
	}

	if (c_currentHeader < (m_buffer + BUFFER_SIZE))
	{
		if (currentHeader->m_amount > n + SPLIT_THRESHOLD)
		{
			info_header* newBegin = reinterpret_cast<info_header*>(c_currentHeader + (headerSize * 2) + n);
			info_header* newEnd = reinterpret_cast<info_header*>(c_currentHeader + headerSize + currentHeader->m_amount);
			info_header* end = reinterpret_cast<info_header*>(c_currentHeader + headerSize + n);
			
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
			info_header* end = reinterpret_cast<info_header*>(c_currentHeader + headerSize + n);
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

	info_header* end = reinterpret_cast<info_header*>(c_begin + headerSize + begin->m_amount);
	char* c_end = reinterpret_cast<char*>(end);

	begin->m_isFree = true;
	end->m_isFree = true;

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
		}
	}
	  
	if (c_end + headerSize < (m_buffer + BUFFER_SIZE))
	{
		info_header* rightBegin = reinterpret_cast<info_header*>(c_end + headerSize);
		char* c_rightBegin = reinterpret_cast<char*>(rightBegin);

		if (rightBegin->m_isFree)
		{
			info_header* rightEnd = reinterpret_cast<info_header*>(c_rightBegin + rightBegin->m_amount + headerSize);

			rightEnd->m_amount = begin->m_amount + rightEnd->m_amount + (headerSize * 2);
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
