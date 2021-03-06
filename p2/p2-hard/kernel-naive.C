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
#define ERROR 0xffffffff
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
    //TODO
    //test get
    int pass = 0;
    Console::puts("===================Kernel Mem Pool Test===============\n");
    Console::puts("Get:\t");
    int got = kernel_mem_pool.get_frame();
    if(got == ERROR){
        Console::puts("Failed\n");
    }
    else{
        Console::puts("Pass\n");
        pass++;
    }
    //test release
    Console::puts("Release:\t");
    if(kernel_mem_pool.release_frame(got) == ERROR){
        Console::puts("Failed\n");
    }
    else{
        Console::puts("Pass\n");
        pass++;
    }
    //test mark
    Console::puts("Mark:\t");
    if(kernel_mem_pool.mark_inaccessible(got, 1) == ERROR){
        Console::puts("Failed\n");
    }
    else{
        Console::puts("Pass\n");
        pass++;
    }
    //
    pass = 0;
    Console::puts("===================Kernel Mem Pool Test===============\n");
    Console::puts("Get:\t");
    got = process_mem_pool.get_frame();
    if(got == ERROR){
        Console::puts("Failed\n");
    }
    else{
        Console::puts("Pass\n");
        pass++;
    }
    //test release
    Console::puts("Release:\t");
    if(process_mem_pool.release_frame(got) == ERROR){
        Console::puts("Failed\n");
    }
    else{
        Console::puts("Pass\n");
        pass++;
    }
    //test mark
    Console::puts("Mark:\t");
    if(process_mem_pool.mark_inaccessible(got, 1) == ERROR){
        Console::puts("Failed\n");
    }
    else{
        Console::puts("Pass\n");
        pass++;
    }
    Console::puts("Total Pass:\t");
    Console::puti(pass);
    Console::puts("/6\n");


    for(;;);

    /* -- WE DO THE FOLLOWING TO KEEP THE COMPILER HAPPY. */
    return 1;
}
