#include <iostream>
#include "TemplateMemoryAllocator.h"
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("Testing memory allocator") {

	MemoryAllocator mAloc;

	void* dude1 = mAloc.allocate(5);
	void* dude2 = mAloc.allocate(3);
	void* dude3 = mAloc.allocate(70);
	void* dude4 = mAloc.allocate(50);
	void* dude5 = mAloc.allocate(8);

	(*static_cast<int*>(dude1)) = 11184810;
	(*static_cast<int*>(dude2)) = 11184810;
	(*static_cast<int*>(dude3)) = 11184810;
	(*static_cast<int*>(dude4)) = 11184810;
	(*static_cast<int*>(dude5)) = 11184810;

	CHECK(mAloc.getUsedCells() == 5);
	CHECK(mAloc.getFreeCells() == 1);

	mAloc.deallocate(dude3);
	mAloc.deallocate(dude4);

	CHECK(mAloc.getUsedCells() == 3);
	CHECK(mAloc.getFreeCells() == 2);

	void* dude6 = mAloc.allocate(3);
	void* dude7 = mAloc.allocate(3);
	void* dude8 = mAloc.allocate(3);

	CHECK(mAloc.getUsedCells() == 6);
	CHECK(mAloc.getFreeCells() == 2);

	mAloc.deallocate(dude1); /// VSICHKO E TOCHNO TYKA, LISTA E PERFEKTEN.
	mAloc.deallocate(dude2);
	mAloc.deallocate(dude4);
	mAloc.deallocate(dude5);
	mAloc.deallocate(dude6);
 	mAloc.deallocate(dude7);
	mAloc.deallocate(dude8);

	CHECK(mAloc.getUsedCells() == 0);
	CHECK(mAloc.getFreeCells() == 1);

	dude1 = mAloc.allocate(50);
	dude2 = mAloc.allocate(50);
	dude3 = mAloc.allocate(50);
	mAloc.deallocate(dude1);
	mAloc.deallocate(dude3);

	CHECK(mAloc.getUsedCells() == 1);
	CHECK(mAloc.getFreeCells() == 2);

	mAloc.deallocate(dude2);

	CHECK(mAloc.getUsedCells() == 0);
	CHECK(mAloc.getFreeCells() == 1);

	//MallocAllocator<int> lala;
	//std::vector<int, MallocAllocator<int>> dude(1000, 5, lala);
	//CHECK(dude.get_allocator().m_allocator.getUsedCells() == 0);
	//dude.push_back(200);
	//dude.push_back(210);

}