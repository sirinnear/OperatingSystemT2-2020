#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// #define DEBUG = 0

#define VALID = 0
#define INVALID = 1
#define MODIFIED = 1
#define FRAME_FREE 0
#define FRAME_NOT_FREE 1
#define NO_FREE_FRAME -1

#define RANDOM 0
#define FIFO 1
#define LRU 2

unsigned long arrival_clock = 0;
volatile unsigned long page_fault_counter = 0;

typedef struct _Fake_frame {
	int is_free; //0 == free, 1 == not free
	long page; // belongs to what page in virtual memory.

	unsigned long time_arrive; // for fifo
	unsigned int used_time;   // for lru
} Fake_frame;

int ReadLine(char *buff, int size, FILE *fp);
long HexTranslator(char* bits);
long BinaryTranslator(char* bits);
unsigned long bitsExtractor(unsigned long bits, unsigned int from, unsigned int length);
void bin(unsigned n);

Fake_frame* init_frame_table(int nframes){
	Fake_frame* frame_table = (Fake_frame*)malloc(sizeof(Fake_frame)*nframes);
	if(frame_table == NULL){
		printf("ERROR ALLOCATING FRAME TABLE!!!\n");
		exit(EXIT_FAILURE);
	}
  for (size_t i = 0; i < nframes; i++)
	{
		frame_table[i].page = 0;
    frame_table[i].time_arrive = 0;
    frame_table[i].used_time = 0;
	}
	return frame_table;
}

void free_frame_table(Fake_frame * ft){
	free(ft);
}
int evict_a_frame(Fake_frame* ft, int nframes, int mode){
  int chosen = NO_FREE_FRAME;
  unsigned long min = 0;
  switch (mode)
  {
  case RANDOM:
    chosen = rand() % nframes;
    ft[chosen].is_free = FRAME_FREE;
    break;
  case FIFO:
    min = ft[0].time_arrive;
    for (int i = 0; i < nframes; i++)
    {
      if(ft[i].time_arrive <= min){
        min = ft[i].time_arrive;
        chosen = i;
      } 
    }
#ifdef DEBUG
        printf("*FIFO SUCCESSED*\n");
#endif
    ft[chosen].is_free = FRAME_FREE;
    break;
  case LRU:
    min = ft[0].time_arrive;
    for (int i = 0; i < nframes; i++)
    {
      if(ft[i].time_arrive <= min){
        min = ft[i].time_arrive;
        chosen = i;
      } 
    }
#ifdef DEBUG
        printf("*LRU SUCCESSED*\n");
#endif
    ft[chosen].is_free = FRAME_FREE;
    break;
  }
#ifdef DEBUG
        printf("*EVICTION SUCCESSED*\n");
#endif
  return chosen;
}
int get_free_frame(Fake_frame * ft, int nframes, int mode){ // return the index of the unused frame
	for (size_t i = 0; i < nframes; i++)
	{
		if(ft[i].is_free == FRAME_FREE) {
#ifdef DEBUG
      printf("*FOUND FREE FRAME*\n");
#endif
			return i; // return frame number I
    }
	}
	// return NO_FREE_FRAME;
  return evict_a_frame(ft, nframes, mode);
}

int get_page_index_frame_table(Fake_frame * ft, int nframes, long vpt, int mode){ // return the index of the unused frame
	for (size_t i = 0; i < nframes; i++)
	{
		if(ft[i].page == vpt){
      if(mode == LRU) ft[i].time_arrive = arrival_clock;
			return i; // return frame number I
    }
	}
	return NO_FREE_FRAME;
}

void update_frame_table(Fake_frame * ft, long page, int ft_idx){
	ft[ft_idx].page = page; // update the currentpage
	ft[ft_idx].is_free  = FRAME_NOT_FREE; // mark as used
  ft[ft_idx].used_time = 1;
  ft[ft_idx].time_arrive = arrival_clock;
	arrival_clock +=1;
}

void mem_accessing(Fake_frame* ft, long vpt, long obt, int opsize, int nframes, int pagesize, int mode){
  
  int free_idx = NO_FREE_FRAME;
  int found_idx = NO_FREE_FRAME;

  if(opsize+obt > pagesize){
    page_fault_counter++;
#ifdef DEBUG
      printf("*OVERFLOW PAGE FAULT*\n");
#endif
    found_idx = get_page_index_frame_table(ft, nframes, vpt, mode);
    if(found_idx != NO_FREE_FRAME){
#ifdef DEBUG
        printf("*FOUND PAGE IN FRAME*\n");
#endif
      ft[found_idx].used_time++;
    }else{
#ifdef DEBUG
      printf("*FOUND NO PAGE, GET A FRAME*\n");
#endif
      free_idx = get_free_frame(ft, nframes, mode);
      if(free_idx != NO_FREE_FRAME) update_frame_table(ft, vpt, free_idx);
    }
#ifdef DEBUG
      printf("*GET AN OVERFLOW FRAME*\n");
#endif
    found_idx = get_page_index_frame_table(ft, nframes, vpt+1, mode);
    if(found_idx != NO_FREE_FRAME){
#ifdef DEBUG
        printf("*FOUND PAGE IN FRAME*\n");
#endif
      ft[found_idx].used_time++;
    }else{
#ifdef DEBUG
      printf("*FOUND NO PAGE, GET A FRAME*\n");
#endif
      free_idx = get_free_frame(ft, nframes, mode);
      if(free_idx != NO_FREE_FRAME) update_frame_table(ft, vpt+1, free_idx);
    }
    return;
  }

#ifdef DEBUG
        printf("*CHECK FOR PAGE IN FRAME*\n");
#endif
  found_idx = get_page_index_frame_table(ft, nframes, vpt, mode);
  if(found_idx != NO_FREE_FRAME){
#ifdef DEBUG
        printf("*FOUND PAGE IN FRAME*\n");
#endif
    ft[found_idx].used_time++;
    return;
  }
  
#ifdef DEBUG
      printf("*NO PAGE FOUND, PAGE FAULT*\n");
#endif
  page_fault_counter++;
  free_idx = get_free_frame(ft, nframes, mode);
  if(free_idx != NO_FREE_FRAME){

    update_frame_table(ft, vpt, free_idx);
#ifdef DEBUG
        printf("*SWAPPING SUCCESSED*\n");
#endif
    return;
  }
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

    int mode = RANDOM;
    char str1[] = "lru", str2[] = "fifo", str3[] = "rand";

    if(strcmp(algorithm, str1)==0){
      mode = LRU;
    }else if(strcmp(algorithm, str2)==0){
      mode = FIFO;
    }else if(strcmp(algorithm, str3)==0){
      mode = RANDOM;
    }else{
		  printf("use: virtmem <trace> <page-size> <nframes> <rand|fifo|lru>\n");
      return 1;
    }

    char buffer[50];
    int idx = 3;

    char ops[2] = "a\0";
    int ops_size = 0;
    char address[17] = "0000000000000000\0";
    int adr_idx = strlen(address);

    int offset_size = page_size;
    int count = 0;
    while(offset_size > 1){
      offset_size = offset_size /2;
      count++;
    }

    offset_size = count;
    // char offset_bits[offset_size+1];
    // offset_bits[offset_size--] = '\0';
    // int offset_idx = offset_size;

    int virtual_size = 15-offset_size;
    // char virtual_bits[virtual_size+1];
    // virtual_bits[virtual_size--] = '\0';
    // int virtual_idx = virtual_size;

    long vp_bits_translated = 0;
    long off_bits_translated = 0;

    Fake_frame* frame_table = init_frame_table(nframes);

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
        while(idx != 3){
          address[adr_idx-1] = buffer[--idx];
          adr_idx--;
        }
        unsigned long address_binary = HexTranslator(address);

        off_bits_translated = bitsExtractor(address_binary, 0, offset_size);
        vp_bits_translated = bitsExtractor(address_binary, offset_size, 64-offset_size);

#ifdef DEBUG
        printf("*Copied*\n");
        printf( "ADDRESS = %s\n", address );
        printf( "OPS = %s\n", ops );
        printf( "SIZE = %d\n", ops_size );
        printf( "address_binary = %ld\n", address_binary );
        printf( "OFFSET BITS T = %ld\n", off_bits_translated );
        printf( "VP NO T = %ld\n", vp_bits_translated );
        bin(address_binary);
                printf("*AB*\n");

        bin(off_bits_translated);
                printf("*OBT*\n");

        bin(vp_bits_translated);
                printf("*VBT*\n");
#endif
      mem_accessing(
        frame_table, 
        vp_bits_translated,
        off_bits_translated, 
        ops_size, 
        nframes, 
        page_size, 
        mode);
#ifdef DEBUG
        printf("*NEW LINE*\n\n");
#endif
    }

    /*
    * remember to close the file.
    */
    free_frame_table(frame_table);
    fclose(trace_file);
    printf("Total page fault is %ld.", page_fault_counter);
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
long BinaryTranslator(char* bits){
  long number = strtol(bits, NULL, 2);
  return number;
}
unsigned long bitsExtractor(unsigned long bits, unsigned int from, unsigned int length){
  unsigned long temp = bits;
  char mask_raw[length];
  for (size_t i = 0; i < length; i++)
  {
    mask_raw[i]='1';
  }
  mask_raw[length] = '\0';
#ifdef DEBUG
  printf("%s\n",mask_raw);
#endif
  unsigned long mask = BinaryTranslator(mask_raw);
  for (size_t i = 0; i < from; i++)
  {
    mask <<= 1;
  }
  return temp&mask;
}

void bin(unsigned n) 
{ 
    /* step 1 */
    if (n > 1) 
        bin(n/2); 
  
    /* step 2 */
    printf("%d", n % 2); 
} 
  