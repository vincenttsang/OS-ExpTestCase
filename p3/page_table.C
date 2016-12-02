//page_table.C
#include "page_table.H"

PageTable     * PageTable::current_page_table;
unsigned int    PageTable::paging_enabled;
FramePool     * PageTable::kernel_mem_pool;
FramePool     * PageTable::process_mem_pool;
unsigned long   PageTable::shared_size;
void PageTable::init_paging(FramePool * _kernel_mem_pool,
                          FramePool * _process_mem_pool,
                          const unsigned long _shared_size){

		kernel_mem_pool = _kernel_mem_pool;
		process_mem_pool = _process_mem_pool;
		shared_size = _shared_size;
	}

PageTable::PageTable(){
	unsigned int i;
	page_directory = (unsigned long*)(process_mem_pool->get_frame()*PAGE_SIZE);
	page_directory[1023] = (unsigned long) page_directory | 3;
	for(i = 0; i < PAGE_SIZE / 8; ++i)
	{
		page_directory[i] = 0;
	}
	unsigned long* page_table = (unsigned long*)(process_mem_pool->get_frame()*PAGE_SIZE);
	unsigned long address = 0;
	for(i = 0; i < ENTRIES_PER_PAGE; ++i){
		page_table[i] = address|3;
		address += PAGE_SIZE;
	}
	page_directory[0] = (unsigned long)page_table|3;
	for(i = 1; i < ENTRIES_PER_PAGE - 1; ++i)
	{
		page_directory[i] = 0 | 2;
	}

}
  /* Initializes a page table with a given location for the directory and the
     page table proper.
     NOTE: The PageTable object still needs to be stored somewhere! Probably it is best
           to have it on the stack, as there is no memory manager yet...
     NOTE2: It may also be simpler to create the first page table *before* paging
           has been enabled.
  */

  void PageTable::load(){
	 write_cr3((unsigned long)page_directory);
	 current_page_table = this;
  }
  /* Makes the given page table the current table. This must be done once during
     system startup and whenever the address space is switched (e.g. during
     process switching). */

  void PageTable::enable_paging(){
	write_cr0(read_cr0() | 0x80000000);
	paging_enabled = 1;
  }
  /* Enable paging on the CPU. Typically, a CPU start with paging disabled, and
     memory is accessed by addressing physical memory directly. After paging is
     enabled, memory is addressed logically. */

  void PageTable::handle_fault(REGS * _r){
	unsigned long* pdir = (unsigned long*)0xFFFFF000;
	unsigned long paddr;

	paddr = read_cr2();
	unsigned long dir_index;
	unsigned long tab_index;
	unsigned long* page_table;

	dir_index = paddr>>22;
	tab_index = paddr>>12 & 0x3FF;

	if(pdir[dir_index] & 1 != 1)
	{
		pdir[dir_index] = (unsigned long)((process_mem_pool->get_frame()*PAGE_SIZE)|3);
		page_table = (unsigned long*)(process_mem_pool->get_frame() * PAGE_SIZE);

		for(unsigned int i = 0; i < ENTRIES_PER_PAGE; ++i)
		{
			page_table[i] = 0|2;
		}
		page_table[tab_index] = (process_mem_pool->get_frame()*PAGE_SIZE)|3;
	}
	else{
		page_table = (unsigned long*)(process_mem_pool->get_frame() * PAGE_SIZE);
		page_table[tab_index] =  (process_mem_pool->get_frame()*PAGE_SIZE)|3;
	}
}
  /* The page fault handler. */
//void PageTable::register_pool(VMPool *pool)
//{
//	//start index > 10 so if it stays above 10, then the vm array are full
//	int vmpools_index = 11;
//	unsigned int i;
//	for(i = 0; i < 10; ++i)
//	{
//		//if there is an open spot in the array, set the index to it
//		if(vmpools[i] == NULL)
//		{
//			vmpools_index= i;
//			break;
//		}
//	}
//	if(vmpools_index > 10)
//	{
//		//if the index is never changed, then it is full
//		Console::puts("Error: Cannot register, array is full!\n");
//	}
//	else{
//		vmpools[vmpools_index] = pool;
//	}
//}

void PageTable::free_page(unsigned long page_no)
{
	unsigned long page_location = (page_no >> 12) & 0x3FF;
	unsigned long* table = (unsigned long*)(0xFFFFF000 | ((page_no >> 22) << 12));
	process_mem_pool->release_frame(table[page_location]);
}
