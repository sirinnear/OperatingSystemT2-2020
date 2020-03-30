 /*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

 typedef struct in_args
 {
	 int npages;
	 int nframes;
	 const char *policy;
	 const char *program;
} Arguments;
Arguments args;
//------elements---------------
struct disk * disk;
char *virtmem;
char *physmem;

//------collecting data-----------------------------------------------------------------------------------------------------------
typedef struct _data
{
	int page_fault;
	int disk_read;
	int disk_write;
	int eviction;

}Data;
Data data;

void init_data(){
	memset(&data, 0, args.nframes * sizeof(Data));
}

//--------------------------------------------------------------------------------------------------------------------------------

//-------Frame list--------------------------------------------------------------------------------------------------------------
#define FRAME_FREE 0
#define FRAME_NOT_FREE 1
unsigned int arrival_clock;

typedef struct _Fake_frame {
	int is_free; //0 == free, 1 == not free
	int page; // belongs to what page in virtual memory.
	int permission_bits;

	int can_write;
	int can_read;

	unsigned int time_arrive;// for fifo
	unsigned int used_time;

} Fake_frame;
Fake_frame *frame_table; // fake_table of things in physical mem 

void init_frame_table(){
	frame_table = (Fake_frame*)malloc(sizeof(Fake_frame)*args.nframes);
	if(frame_table == NULL){
		printf("ERROR ALLOCATING FRAME TABLE!!!\n");
		exit(EXIT_FAILURE);
	}
	memset(frame_table, 0, args.nframes * sizeof(Fake_frame));// set everything to 0
	return;
}

void free_frame_table(){
	free(frame_table);
}

#define NO_FREE_FRAME -1
int get_free_frame(){ // return the index of the unused frame
	for (size_t i = 0; i < args.nframes; i++)
	{
		if(frame_table[i].is_free == 0) 
			return i; // return frame number I
	}
	return NO_FREE_FRAME;
}

void update_frame_table(int page, int frame_table_index, int permission_bits){
	frame_table[frame_table_index].page = page; // update the currentpage
	frame_table[frame_table_index].permission_bits = permission_bits; // update permission
	frame_table[frame_table_index].is_free  = FRAME_NOT_FREE; // mark as used

	arrival_clock +=1;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------

void do_eviction(struct page_table *pt, int target_frame_index){
	data.eviction+=1;
	if ((frame_table[target_frame_index].permission_bits & PROT_WRITE) == PROT_WRITE) // mark as dirty
	{ 
		// write data from phymem to the disk
		disk_write(disk, frame_table[target_frame_index].page, &physmem[target_frame_index * PAGE_SIZE]);
		data.disk_write +=1;
	}
	frame_table[target_frame_index].permission_bits = PROT_NONE;
	frame_table[target_frame_index].is_free = FRAME_FREE;
	page_table_set_entry(pt, frame_table[target_frame_index].page , target_frame_index, PROT_NONE); // update page table
}
int get_rand_index(){ //for rand
	return rand() % args.nframes;
}
int get_index_arrive_first(){//for fifo
	int min_val =  ~(1<<31);
	int min_index = -1 ;

	for(size_t i = 0; i < args.nframes; i++){

		if(frame_table[i].time_arrive < min_val){
			min_index = i;
			min_val = frame_table[i].time_arrive;
		}
	}

	if(min_index < 0){
		printf("SOME THING IS WRONG WITH FIFO\n");
		exit(EXIT_FAILURE);
	}
	return min_index;
}

int get_lru_index(){
	unsigned int min_val = ~(1 << 31);
	int min_index = -1;
	
	for (size_t i = 0; i < args.nframes; i++)
	{
		if(frame_table[i].time_arrive <= min_val)
		{	
			min_index = i;
			min_val = frame_table[i].time_arrive;
		}
		// printf("at i=%d used_time= %d\n", (int)i, frame_table[i].time_arrive);
	}
	if (min_index < 0)
	{
		printf("HELLO SOME THING IS WRONG WITH LRU\n");
		exit(EXIT_FAILURE);
	}
	// printf("ret index=%d used_time= %d \n", min_index, min_val);

	return min_index;
}

void page_fault_handler_LRU( struct page_table *pt, int page )
{	
	int current_frame, permission_bits;
	int frame_table_index;
	page_table_get_entry(pt, page, &current_frame, &permission_bits);

	data.page_fault += 1;
	// printf("current frame with pert bits: %d, %d \n", current_frame, permission_bits);
	if(permission_bits == PROT_NONE)
	{
		// printf("first time getting here get READ \n");
		frame_table_index = get_free_frame();
		// printf("free index is : %d", frame_table_index);
		if(frame_table_index == NO_FREE_FRAME) // full
		{
		
			//do lru
			frame_table_index = get_lru_index();
			//do_eviction
			data.eviction += 1;
			if (frame_table[frame_table_index].can_write) // mark as dirty
			{
				// write data from phymem to the disk
				disk_write(disk, frame_table[frame_table_index].page, &physmem[frame_table_index * PAGE_SIZE]);
				data.disk_write += 1;
			}
			frame_table[frame_table_index].permission_bits = PROT_NONE;
			frame_table[frame_table_index].is_free = FRAME_FREE;
			frame_table[frame_table_index].can_write = 0;
			frame_table[frame_table_index].can_read = 0;
			page_table_set_entry(pt, frame_table[frame_table_index].page, frame_table_index, PROT_NONE); // update page table
		}
		permission_bits = PROT_READ;
		// frame_table[frame_table_index].can_read = 1;
		frame_table[frame_table_index].time_arrive = arrival_clock;
		disk_read(disk, page, &physmem[frame_table_index * PAGE_SIZE]);
		data.disk_read += 1;
		
	}
	else //if (((permission_bits&PROT_READ) == PROT_READ) && !(frame_table[current_frame].can_write))
	{
		frame_table_index = current_frame; // same place
		frame_table[frame_table_index].used_time += 1;

		if((permission_bits&PROT_READ) == PROT_READ)
		{
	
			permission_bits = PROT_WRITE;
			frame_table[frame_table_index].can_write = 1;
		}
		else // PROT WRITE
		{
			// if (frame_table[frame_table_index].can_write)
			// {
			// 	data.page_fault -= 1;
			// }
			if (frame_table[frame_table_index].can_read){
				data.page_fault-=1;
			}
			permission_bits = PROT_READ;
			frame_table[frame_table_index].can_read = 1;   
		}

		frame_table[frame_table_index].time_arrive = arrival_clock;
	}

	page_table_set_entry(pt, page, frame_table_index, permission_bits); // update page table
	update_frame_table(page, frame_table_index, permission_bits);

	return;

}
void page_fault_handler_FIFO_RAND( struct page_table *pt, int page )
{	
	data.page_fault +=1;

	int current_frame, permission_bits;
	int frame_table_index;
	page_table_get_entry(pt, page, &current_frame, &permission_bits);

	if (permission_bits == PROT_NONE) // first time geting in
	{
		frame_table_index = get_free_frame();

		if (frame_table_index == NO_FREE_FRAME)
		{ // cannot find free frame
			if (!strcmp(args.policy, "rand"))
			{
				frame_table_index = get_rand_index();
			}
			else if ((!strcmp(args.policy, "fifo")))
			{
				 frame_table_index = get_index_arrive_first();
			}
			else if ((!strcmp(args.policy, "lru")))
			{
				//not gona use here.
				//was planed to use but not gona use anymore
			}
			else
			{
				printf("something is wrong you should not see this message.\n");
				printf("unhandle page fault on page #%d\n", page);
				exit(EXIT_FAILURE);
			}
			do_eviction(pt, frame_table_index);
		}
		if((!strcmp(args.policy,"fifo"))){
			frame_table[frame_table_index].time_arrive = arrival_clock;
		}
		permission_bits = PROT_READ;
		disk_read(disk, page, &physmem[frame_table_index * PAGE_SIZE]);
		data.disk_read+=1;
	} 
	else //( (permission_bits == PROT_WRITE) && (permission_bits != (PROT_READ | PROT_WRITE))) // cannot write
	{
		permission_bits = (PROT_READ | PROT_WRITE);
		frame_table_index = current_frame; // same place
	}
	page_table_set_entry(pt, page, frame_table_index, permission_bits); // update page table
	update_frame_table(page, frame_table_index, permission_bits);
	return;
}
void page_fault_handler(struct page_table *pt, int page)
{
	if (!strcmp(args.policy, "rand") || !strcmp(args.policy, "fifo")){
		page_fault_handler_FIFO_RAND(pt, page);
		return;
	}
	else if (!strcmp(args.policy, "lru"))
	{	
		page_fault_handler_LRU(pt, page);
		return;
	}
	else{
		printf("You should not see this message something is wrong with the input policy.\n");
		exit(EXIT_FAILURE);
	}
}

int main( int argc, char *argv[] )
{
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|lru> <sort|scan|focus>\n");
		return 1;
	}

	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
	const char *policy = argv[3];
	const char *program = argv[4];

	//stoge all in put in a global struct.
	args.npages = npages;
	args.nframes = nframes;
	args.policy = policy;
	args.program = program;

	init_frame_table();
	init_data();
	arrival_clock = 0; // init time for fifo
	// srand(time(NULL));

	disk = disk_open("myvirtualdisk", npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}


	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	virtmem = page_table_get_virtmem(pt);

	physmem = page_table_get_physmem(pt);

	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[4]);

	}

	// page_table_print(pt);
	
	free_frame_table();
	page_table_delete(pt);
	disk_close(disk);

	// printf("The total number of page fault is %d \n", data.page_fault);
	// printf("The total number of disk read is %d \n", data.disk_read);
	// printf("The total number of disk write is %d \n", data.disk_write);
	// printf("The total number of ecivtion is %d \n", data.eviction);
	printf("Disk Reads: %d\n", data.disk_read);
	printf("Disk Writes: %d\n", data.disk_write);
	printf("Page Faults: %d\n", data.page_fault);

	return 0;
}
