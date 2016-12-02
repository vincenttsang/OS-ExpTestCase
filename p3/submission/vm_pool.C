//vm_pool.cpp
#include "vm_pool.H"
VMPool::VMPool(unsigned long _base_address, unsigned long _size, FramePool *_frame_pool, PageTable *_page_table)
{
	base_address = _base_address;
	size = _size;
	frame_pool = _frame_pool;
	page_table = _page_table;
	page_table->register_pool(this);
	reg_number = 0;
}
   /* Initializes the data structures needed for the management of this
    * virtual-memory pool.
    * _base_address is the logical start address of the pool.
    * _size is the size of the pool in bytes.
    * _frame_pool points to the frame pool that provides the virtual
    * memory pool with physical memory frames.
    * _page_table points to the page table that maps the logical memory
    * references to physical addresses. */

unsigned long VMPool::allocate(unsigned long _size)
{
	Region* current_region = (Region*) base_address;

	if(reg_number == 0)
	{
		reg_number += 1;
		current_region->size = size - sizeof(Region);
		current_region->is_available = TRUE;
	}
	BOOLEAN is_empty_slot = FALSE;
	for(unsigned int i = 0; i < reg_number; ++i)
	{
		//first check if the region is big enough
		if(current_region->size >= _size)
		{
			//then check if the region is actually available
			if(current_region->is_available)
			{
				is_empty_slot = TRUE;
				break;
			}
		}
		current_region = (Region*)((unsigned long)current_region + sizeof(Region) + current_region->size);
	}
	//fail if there is no empty region
	if(!is_empty_slot)
	{
		return 0;
	}

	unsigned long allocate_address = (unsigned long)current_region + sizeof(Region);
	current_region->is_available = FALSE;

	return allocate_address;

}
   /* Allocates a region of _size bytes of memory from the virtual
    * memory pool. If successful, returns the virtual address of the
    * start of the allocated region of memory. If fails, returns 0. */

void VMPool::release(unsigned long _start_address)
{
	Region* current_region = (Region*) base_address;
	unsigned long release_address;
	unsigned int i;
	for(i = 0; i < reg_number; ++i)
	{
		release_address = (unsigned long)current_region + sizeof(Region);
		if(release_address != _start_address)
		{
			current_region = (Region*)(unsigned long)(release_address + current_region->size);
			continue;
		}
		current_region->is_available = TRUE;
		return;
	}
}
   /* Releases a region of previously allocated memory. The region
    * is identified by its start address, which was returned when the
    * region was allocated. */

BOOLEAN VMPool::is_legitimate(unsigned long _address)
{
	if(_address < base_address)
	{
		return FALSE;
	}
	if(_address > base_address + size)
	{
		return FALSE;
	}
	else{
		return TRUE;
	}
}
