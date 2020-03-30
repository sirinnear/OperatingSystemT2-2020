#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "page_table.h"

#define DEBUG = 0

#define VALID = 0
#define INVALID = 1
#define MODIFIED = 1
#define FRAME_FREE 0
#define FRAME_NOT_FREE 1
#define NO_FREE_FRAME -1

unsigned int arrival_clock = 0;
volatile unsigned long page_fault_counter = 0;

typedef struct _Fake_frame {
	int is_free; //0 == free, 1 == not free
	long page; // belongs to what page in virtual memory.

	unsigned int time_arrive; // for fifo
	unsigned int used_time;   // for lru
} Fake_frame;

int ReadLine(char *buff, int size, FILE *fp);
long HexTranslator(char* bits);

Fake_frame* init_frame_table(int nframes){
	Fake_frame* frame_table = (Fake_frame*)malloc(sizeof(Fake_frame)*nframes);
	if(frame_table == NULL){
		printf("ERROR ALLOCATING FRAME TABLE!!!\n");
		exit(EXIT_FAILURE);
	}
	return frame_table;
}

void free_frame_table(Fake_frame * ft){
	free(ft);
}

int get_free_frame(Fake_frame * ft, int nframes){ // return the index of the unused frame
	for (size_t i = 0; i < nframes; i++)
	{
		if(ft[i].is_free == FRAME_FREE) 
			return i; // return frame number I
	}
	return NO_FREE_FRAME;
}

int get_page_index_frame_table(Fake_frame * ft, int nframes, long vpt){ // return the index of the unused frame
	for (size_t i = 0; i < nframes; i++)
	{
		if(ft[i].page == vpt) 
			return i; // return frame number I
	}
	return NO_FREE_FRAME;
}

void update_frame_table(Fake_frame * ft, long page, int ft_idx){
	ft[ft_idx].page = page; // update the currentpage
	ft[ft_idx].is_free  = FRAME_NOT_FREE; // mark as used
	arrival_clock +=1;
}

void mem_accessing(Fake_frame* ft, long vpt, long obt, int opsize, int nframes, int pagesize){
  
  int free_idx = get_free_frame(ft, nframes);

  if(opsize+obt > nframes){

#ifdef DEBUG
      printf("*OVERFLOW PAGE FAULT*\n");
#endif

    if(get_page_index_frame_table(ft, nframes, vpt) == NO_FREE_FRAME){
#ifdef DEBUG
      printf("*FOUND NO PAGE, GET A FRAME*\n");
#endif
      
    }

#ifdef DEBUG
      printf("*GET AN OVERFLOW FRAME*\n");
#endif
    update_frame_table(ft, vpt+1, free_idx);
  }

#ifdef DEBUG
        printf("*CHECK FOR PAGE IN FRAME*\n");
#endif

  if(get_page_index_frame_table(ft, nframes, vpt) != NO_FREE_FRAME){
#ifdef DEBUG
        printf("*FOUND PAGE IN FRAME*\n");
#endif
    return;
  }
  
#ifdef DEBUG
      printf("*FOUND NO PAGE, PAGE FAULT*\n");
#endif

  if(free_idx != NO_FREE_FRAME){
#ifdef DEBUG
      printf("*FOUND FREE FRAME*\n");
#endif
    update_frame_table(ft, vpt, free_idx);
    return;
  }else{

  }


#ifdef DEBUG
        printf("*NOT FOUND, PAGE FAULT*\n");
#endif

#ifdef DEBUG
        printf("*EVICTION SUCCESSED*\n");
#endif

#ifdef DEBUG
        printf("*SWAPPING SUCCESSED*\n");
#endif

}

int main(int argc, char** argv){
    
    if(argc!=5) {
		printf("use: virtmem <trace> <page-size> <nframes> <rand|fifo|lru>\n");
		return 1;
	}
    char *trace_name = argv[1];
    int page_size = atoi(argv[2]);
    int nframes = atoi(argv[3]);
    char *algorithm = argv[4];
    FILE *trace_file;

    char buffer[50];
    ssize_t read;
    int idx = 3;

    char ops[2] = "a\0";
    int ops_size = 0;
    char address[17] = "0000000000000000\0";
    int adr_idx = strlen(address);

    int offset_size = page_size;
    int count = 0;
    while(offset_size > 1){
      offset_size = offset_size /16;
      count++;
    }
    offset_size = count;
    char offset_bits[offset_size+1];
    offset_bits[offset_size--] = '\0';
    int offset_idx = offset_size;

    int virtual_size = 15-offset_size;
    char virtual_bits[virtual_size+1];
    virtual_bits[virtual_size--] = '\0';
    int virtual_idx = virtual_size;

    long vp_bits_translated = 0;
    long off_bits_translated = 0;

    if((trace_file = fopen(trace_name, "r")) == NULL){
            exit(1);
    }
#ifdef DEBUG
        printf( "offset size is %d\n", offset_size+1 );
        printf( "virtual size is %d\n", virtual_size+1 );
#endif
    while( ReadLine(buffer, sizeof(buffer), trace_file) ) {
#ifdef DEBUG
        printf( "%s\n", buffer );
#endif
        if(buffer[0] == 'I'){
          ops[0] = buffer[0];
        }else{
          ops[0] = buffer[1];
        }
        idx = 3;
        while(buffer[idx]!=','){
          idx++;
        }
        ops_size = atoi(&buffer[idx+1]);

        adr_idx = strlen(address);
        offset_idx = offset_size;
        virtual_idx = virtual_size;
        while(idx != 3){
          address[adr_idx-1] = buffer[--idx];
          adr_idx--;
          if(offset_idx >= 0){
            offset_bits[offset_idx--] = buffer[idx];
          }else{
            virtual_bits[virtual_idx--] = buffer[idx];
          }
        }
        while(offset_idx >= 0){
          offset_bits[offset_idx--] = '0';
        }
        while(virtual_idx >= 0){
          virtual_bits[virtual_idx--] = '0';
        }
        vp_bits_translated = HexTranslator(virtual_bits);
        off_bits_translated = HexTranslator(offset_bits);

#ifdef DEBUG
        printf("*Copied*\n");
        printf( "ADDRESS = %s\n", address );
        printf( "OPS = %s\n", ops );
        printf( "SIZE = %d\n", ops_size );
        printf( "OFFSET BITS = %s\n", offset_bits );
        printf( "OFFSET BITS T = %ld\n", off_bits_translated );
        printf( "VP NO = %s\n", virtual_bits );
        printf( "VP NO T = %ld\n", vp_bits_translated );
        printf("*NEW LINE*\n\n");
#endif
    }
    /*
    * remember to close the file.
    */
    fclose(trace_file);
}

int ReadLine(char *buff, int size, FILE *fp)
{
  buff[0] = '\0';
  buff[size - 1] = '\0';             /* mark end of buffer */
  char *tmp;

  if (fgets(buff, size, fp) == NULL) {
      *buff = '\0';                   /* EOF */
      return 0;
  }
  else {
      /* remove newline */
      if ((tmp = strrchr(buff, '\n')) != NULL) {
        *tmp = '\0';
      }
  }
  return 1;
}
long HexTranslator(char* bits){
  long number = strtol(bits, NULL, 16);
  return number;
}