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
#include "gdt.H"
#include "idt.H"          /* LOW-LEVEL EXCEPTION MGMT. */
#include "irq.H"
#include "exceptions.H"
#include "interrupts.H"

#include "simple_timer.H" /* TIMER MANAGEMENT */

#include "page_table.H"
#include "paging_low.H"

/*--------------------------------------------------------------------------*/
/* MAIN ENTRY INTO THE OS */
/*--------------------------------------------------------------------------*/

int main() {

    GDT::init();
    Console::init();
    IDT::init();
    ExceptionHandler::init_dispatcher();
    IRQ::init();
    InterruptHandler::init_dispatcher();


    /* -- EXAMPLE OF AN EXCEPTION HANDLER -- */

    class DBZ_Handler : public ExceptionHandler {
      public:
      virtual void handle_exception(REGS * _regs) {
        Console::puts("DIVISION BY ZERO!\n");
        for(;;);
      }
    } dbz_handler;

    ExceptionHandler::register_handler(0, &dbz_handler);


    /* -- INITIALIZE FRAME POOLS -- */

    FramePool kernel_mem_pool(KERNEL_POOL_START_FRAME,
                              KERNEL_POOL_SIZE,
                              0);
    unsigned long process_mem_pool_info_frame = kernel_mem_pool.get_frame();
    FramePool process_mem_pool(PROCESS_POOL_START_FRAME,
                               PROCESS_POOL_SIZE,
                               process_mem_pool_info_frame);
    process_mem_pool.mark_inaccessible(MEM_HOLE_START_FRAME, MEM_HOLE_SIZE);

    /* -- INITIALIZE MEMORY (PAGING) -- */

    /* ---- INSTALL PAGE FAULT HANDLER -- */

    class PageFault_Handler : public ExceptionHandler {
      public:
      virtual void handle_exception(REGS * _regs) {
        PageTable::handle_fault(_regs);
      }
    } pagefault_handler;

    ExceptionHandler::register_handler(14, &pagefault_handler);

    /* ---- INITIALIZE THE PAGE TABLE -- */

    PageTable::init_paging(&kernel_mem_pool,
                           &process_mem_pool,
                           4 MB);

    PageTable pt;

    //TODO
    //access the first 4MB memory
    Console::puts("1. Access 1st 4MB:\t");
    int *first = (int *) (FAULT_ADDR - 1);
    *first = 1;
    if((*first) == 1)
        Console::puts("Pass\n");
    else
        Console::puts("Failed\n");

    pt.load();
    //TODO
    //test if the page directory is loaded
    //can we access the page directory?
    //can we access the first page table?
    Console::puts("2. Access 1st page directory:\t");
    unsigned long *page_dir_addr = (unsigned long*)read_cr3();
    unsigned long page_dir_1 = page_dir_addr[0];
    if((page_dir_1 & 3) == 0x00000003)
        Console::puts("Pass\n");
    else
        Console::puts("Failed\n");

    Console::puts("3. Access 1st page table:\t");
    unsigned long *page_tb_addr = (unsigned long*) page_dir_1;
    unsigned long page_tb_1 = page_tb_addr[0];
    if((page_tb_1 & 3) == 0x00000003)
        Console::puts("Pass\n");
    else
        Console::puts("Failed\n");

    PageTable::enable_paging();
    //TODO
    //test if the page enabling bit in CR0 is set
    //memory beyond first 4MB not present?
    Console::puts("4. Access Page Enable Bit:\t");
    unsigned long cr0 = read_cr0();
    if((cr0 >> 31) == 1)
        Console::puts("Pass\n");
    else
        Console::puts("Failed\n");

    //Console::puts("5. Access Beyond 4MB:\t");
    //if((page_dir_addr[1] & 2) == 0x00000002)
    //    Console::puts("Pass\n");
    //else
    //    Console::puts("Failed\n");


    /* -- INITIALIZE THE TIMER (we use a very simple timer).-- */

    SimpleTimer timer(1000); /* timer ticks every 10ms. */
    InterruptHandler::register_handler(0, &timer);

    /* NOTE: The timer chip starts periodically firing as
             soon as we enable interrupts.
             It is important to install a timer handler, as we
             would get a lot of uncaptured interrupts otherwise. */

    /* -- ENABLE INTERRUPTS -- */

    Machine::enable_interrupts();

    /* -- MOST OF WHAT WE NEED IS SETUP. THE KERNEL CAN START. */

    Console::puts("Hello World!\n");

    /* -- GENERATE MEMORY REFERENCES */

    int *foo = (int *) FAULT_ADDR;
    foo[NACCESS-1] = NACCESS - 1;
    foo[5] = 5;
    int i;

    Console::puts("Starting Contiguous Access:\n");
    for (i=0; i<NACCESS; i++) {
       foo[i] = i;
    }
    Console::puts("Passing Write Phase\n");
    for (i=0; i<NACCESS; i++) {
       if(foo[i] != i) {
          Console::puts("TEST FAILED for access number:");
          Console::putui(i);
          Console::puts("\n");
          break;
       }
    }
    Console::puts("6. Contiguous Access: \t");
    if(i == NACCESS) {
       Console::puts("PASSED\n");
    }
    else
       Console::puts("FAILED\n");

    //TODO
    //access out of boundary memory, temporarily not tested, waiting to a
    //uniform standard

    //access not present mem
    //Console::puts("6. Access out of bound:\t");
    //int *out_bound = (int *) (32 MB);


    /* -- NOW LOOP FOREVER */
    for(;;);

    /* -- WE DO THE FOLLOWING TO KEEP THE COMPILER HAPPY. */
    return 1;
}

