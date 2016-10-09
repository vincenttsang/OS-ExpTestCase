
/*
    File: kernel.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 12/09/03


    This file has the main entry point to the operating system.

*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
#define KERNEL_POOL_START_FRAME ((2 MB) / (4 KB))
#define KERNEL_POOL_SIZE ((2 MB) / (4 KB))
#define PROCESS_POOL_START_FRAME ((4 MB) / (4 KB))
#define PROCESS_POOL_SIZE ((28 MB) / (4 KB))
/* definition of the kernel and process memory pools */

#define MEM_HOLE_START_FRAME ((15 MB) / (4 KB))
#define MEM_HOLE_SIZE ((1 MB) / (4 KB))
/* we have a 1 MB hole in physical memory starting at address 15 MB */

#define FAULT_ADDR (4 MB)
/* used in the code later as address referenced to cause page faults. */
#define NACCESS ((1 MB) / 4)
/* NACCESS integer access (i.e. 4 bytes in each access) are made starting at address FAULT_ADDR */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"     /* LOW-LEVEL STUFF   */
#include "console.H"
//#include "gdt.H"
//#include "idt.H"          /* LOW-LEVEL EXCEPTION MGMT. */
//#include "irq.H"
//#include "exceptions.H"
#include "interrupts.H"
#include "frame_pool.H"
//#include "simple_timer.H" /* TIMER MANAGEMENT */

//#include "page_table.H"
//#include "paging_low.H"

/*--------------------------------------------------------------------------*/
/* MAIN ENTRY INTO THE OS */
/*--------------------------------------------------------------------------*/

int main() {

    Console::init();




    /* -- INITIALIZE FRAME POOLS -- */

    FramePool kernel_mem_pool(KERNEL_POOL_START_FRAME,
                              KERNEL_POOL_SIZE,
                              0);
    unsigned long process_mem_pool_info_frame = kernel_mem_pool.get_frame();
    FramePool process_mem_pool(PROCESS_POOL_START_FRAME,
                               PROCESS_POOL_SIZE,
                               process_mem_pool_info_frame);
    process_mem_pool.mark_inaccessible(MEM_HOLE_START_FRAME, MEM_HOLE_SIZE);

	//test


    /* -- MOST OF WHAT WE NEED IS SETUP. THE KERNEL CAN START. */

    Console::puts("Hello World, we are conquering P2!\n");

    /* HERE WRITE YOUR OWN CODE TO TEST THE FRAME POOL CLASS
     * IMPLEMENTATION WITH THE TWO OBJECTS WE ALLOCATED - ON THE
     * STACK, WE DO NOT HAVE "NEW" YET - ABOVE */

	int allocated = 0;
	unsigned long x = 0xffffffff;
	kernel_mem_pool.mark_inaccessible(555, 22);
	Console::puts("kernel marked inaccessible: 555-576\n");

	for(int i = 512; i < 1024; i++) //allocate everything
	{
		if(kernel_mem_pool.get_frame() != x)
			++allocated;
		else{
			//print failed attempts to check if make_inaccessible works
			Console::puti(i);
			Console::puts("th kernel mem frame allocation is failed!\n");
		}
	}
	//kernel mem pool shall be all allocated except for inaccessible ones
 	Console::puts("Kernel ALLOCATED: ");
	Console::puti(allocated);
	Console::puts("\n");

	if(kernel_mem_pool.get_frame() == x) //test logic
		Console::puts("kernel RUN OUT OF MEMORY\n");//expected
	else
		Console::puts("kernel get_frame - success\n");

	if(kernel_mem_pool.release_frame(5000) == 0) //test boundries
		Console::puts("kernel released - success\n");
	else
		Console::puts("kernel released - Failed to release\n");//expected

	if(kernel_mem_pool.release_frame(516) == 0) //test release
		Console::puts("kernel released - success\n");//expected
	else
		Console::puts("kernel released - Failed to release\n");

	if(kernel_mem_pool.release_frame(556) == 0) //test release inaccessible
		Console::puts("kernel release inaccessible - success\n");
	else
		Console::puts("kernel release inaccessible - failure\n");//expected


	Console::puts("it released: \n");
	unsigned long k = kernel_mem_pool.get_frame();
	Console::putui(k);
	Console::puts("\n");

	int released = 0;
	for(int i = 512; i < 1024; i++) // release everything
	{
		if(kernel_mem_pool.release_frame(i) != x)
			++released;
	}

	Console::puts("Kernel RELEASED: ");
	Console::puti(released);
	Console::puts("\n");

	allocated = 0;
	for(int i = 0; i < 512; ++i) // allocate it all again
	{
		if(kernel_mem_pool.get_frame() != x)
			++allocated;
	}

 	Console::puts("Kernel ALLOCATED: ");
	Console::puti(allocated);
	Console::puts("\n");

 	if(kernel_mem_pool.get_frame() == x)
		Console::puts("Kernel RUN OUT OF MEMORY\n");//expecteed
	else
		Console::puts("Kernel get_frame - success\n");

	allocated = 0;
	for(int i = 0; i < 7680; ++i)  // same process as above.
	{
		if(process_mem_pool.get_frame() != x)
					++allocated;
	}

	Console::puts("Process ALLOCATED: ");
	Console::puti(allocated);
	Console::puts("\n");

	if(process_mem_pool.get_frame() == x)
		Console::puts("process RUN OUT OF MEMORY\n");
	else
		Console::puts("process get_frame - success\n");


	if(process_mem_pool.get_frame() == x)
		Console::puts("process RUN OUT OF MEMORY\n");
	else
		Console::puts("process get_frame - success\n");

	if(process_mem_pool.release_frame(516) == 0)
		Console::puts("process released - success\n");
	else
		Console::puts("process released - Failed to release\n");

	if(kernel_mem_pool.get_frame() == x)
		Console::puts("kernel RUN OUT OF MEMORY\n");
	else
		Console::puts("kernel get_frame - success\n");

	if(process_mem_pool.release_frame(5000) == 0)
		Console::puts("process released - success\n");
	else
		Console::puts("process released - Failed to release\n");

	if(process_mem_pool.release_frame(3841) == 0)
		Console::puts("process release hole - success\n");
	else
		Console::puts("process release hole - Failed to release\n");

	Console::puts("it released: \n");
	unsigned long z = process_mem_pool.get_frame();
	Console::putui(z);
	Console::puts("\n");

	if(process_mem_pool.get_frame() == x)
		Console::puts("process RUN OUT OF MEMORY\n");
	else
		Console::puts("process get_frame - success\n");

	released = 0;
 	for(int i = 1024; i < 8192; ++i)
	{
		if(process_mem_pool.release_frame(i) != x)
					++released;
	}

	Console::puts("process RELEASED: ");
	Console::puti(released);
	Console::puts("\n");

	process_mem_pool.mark_inaccessible(5000, 50);
	Console::puts("process marked inaccessible: 5000-5049\n");
	allocated = 0;
	for(int i = 0; i < 7680; ++i)
	{
		if(process_mem_pool.get_frame() != x)
					++allocated;
		else{
			Console::puti(i);
			Console::puts("th process mem frame allocation failed\n");//check if inaccessible frames are not allocated
		}
	}

	Console::puts("Process ALLOCATED: ");
	Console::puti(allocated);
	Console::puts("\n");


    /* -- NOW LOOP FOREVER */
    Console::puts("Testing is done. We will do nothing forever\n");






    /* -- NOW LOOP FOREVER */
    for(;;);

    /* -- WE DO THE FOLLOWING TO KEEP THE COMPILER HAPPY. */
    return 1;
}
