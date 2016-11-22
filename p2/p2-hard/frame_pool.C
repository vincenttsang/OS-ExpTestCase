//frame_pool.C
//Daniel Croy
//UIN: 724008680
//09-19-16

#include "frame_pool.H"

/* 
The following code was taken directly from 
Stack Overflow: http://stackoverflow.com/questions/1225998/what-is-a-bitmap-in-c
*/

typedef unsigned long word_t;
enum { BITS_PER_WORD = sizeof(word_t) * 8 };
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

void set_bit(word_t *words, int n) { 
    words[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void clear_bit(word_t *words, int n) {
    words[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n)); 
}

int get_bit(word_t *words, int n) {
    word_t bit = words[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
    return bit != 0; 
}

/*
The remaining code was either provided by the project specification,
or written by myself.
*/

//FramePool Constructor
/* 
512 is the number of frames specified by the P2 project specification.

When info_frame_no is 0, the frame pool is free to store management 
information in any frame in the pool.
*/ 
FramePool::FramePool(unsigned long _base_frame_no, unsigned long _nframes = 512, unsigned long _info_frame_no = 0)
	: base_frame_no(_base_frame_no), nframes(_nframes), info_frame_no(_info_frame_no)
{
	if (_info_frame_no == 0){info_frame_no = _base_frame_no;}
	
	used_bitmap = (unsigned long*) (info_frame_no*FRAME_SIZE);
	inaccessible_bitmap = (unsigned long*) (info_frame_no*FRAME_SIZE)+1024;
	
	for (int i = 0; i < _nframes; ++i){
		for (int j = 0; j < BITS_PER_WORD; ++j){
			clear_bit(used_bitmap + i, j);
			clear_bit(inaccessible_bitmap + i, j);
		}
	}
	//mark_inaccessible(info_frame_no, 1);
}

//get_frame()
//Returns hex address 0xffffffff upon memory shortage, which corresponds to unsigned long -1. 
unsigned long FramePool::get_frame(){
	//iterate through bitmap until you find an unused frame
	int i, j;
	for (i = 0; i < (nframes/BITS_PER_WORD); ++i){
		int check = 0;
		for (j = 0; j < BITS_PER_WORD; ++j){
			if ((get_bit(used_bitmap + i, j) != 1) && (get_bit(inaccessible_bitmap + i, j) != 1)){
				check = 1;
				break;
			}
		}
		if (check != 0){break;}
	}
	if (i != (nframes/BITS_PER_WORD) && j != BITS_PER_WORD){
		set_bit(used_bitmap + i, j);
		return base_frame_no + i*BITS_PER_WORD + j;
	}	
	//if you get to the end of the bitmap without finding an unused frame, out of memory
	return (unsigned long) -1;
}

//release_frame()
//Returns 0 on success and -1 on failure
unsigned long FramePool::release_frame(unsigned long _frame_no){
	//take care of the easy out-of-bounds
	if (_frame_no > base_frame_no + nframes){return -1;}
	if (_frame_no < base_frame_no){return -1;}
	
	//find the frame in the bitmap and if accessible change the memory to unused
	int word = (_frame_no - info_frame_no) / BITS_PER_WORD;
	int offset = (_frame_no - info_frame_no) % BITS_PER_WORD;
	if (get_bit(inaccessible_bitmap + word, offset) != 1){
		clear_bit(used_bitmap + word, offset);
	}else{return -1;}
	
	//if something goes wrong, return failure
	if (get_bit(used_bitmap + word, offset) == 0){return 0;}
	else{return -1;}
}

//mark_inaccessible()
//Returns 0 on success and -1 on failure
unsigned long FramePool::mark_inaccessible(unsigned long _base_frame_no, unsigned long _nframes){
	//take care of the easy out-of-bounds
	if (_base_frame_no + _nframes > base_frame_no + nframes){return -1;}
	if (_base_frame_no < base_frame_no){return -1;}
	
	//mark the memory as inaccessible in the bitmap
	int i;
	int word = (_base_frame_no - info_frame_no) / BITS_PER_WORD;
	int offset = (_base_frame_no - info_frame_no) % BITS_PER_WORD;
	for (i = 0; i < _nframes; ++i){
		if ((offset + i) < 32){
			set_bit(inaccessible_bitmap + word, offset + i);
		}else{
			set_bit(inaccessible_bitmap + word + 1, ((offset + i) % BITS_PER_WORD));
		}
	}
	//if something goes wrong, return failure	
	if (i == _nframes){return 0;}
	else{return -1;}
}