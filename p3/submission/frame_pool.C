#include "frame_pool.H"
#include "console.H"
//check to see if the all memory is full
unsigned int isBlockFull(unsigned long currentBlock)
{
	return (unsigned int) currentBlock ^ 0xFFFFFFFF;
}
//power since we don't have access to standard library
int power(int x, int y)
{
	int value = x;
	for(int i = 0;  i < y; ++i)
	{
		value = value * x;
	}
	return value;
}
FramePool::FramePool(unsigned long _base_frame_no, unsigned long _nframes, unsigned long _info_frame_no)
{
	base_frame_no = _base_frame_no;
	nframes = _nframes;
	//if the info frame is zero, set it to the base frame
	if(_info_frame_no == 0)
	{
		info_frame_no = base_frame_no;
	}
	else
	{
		info_frame_no = _info_frame_no + base_frame_no;
	}
	//points to the first frame
	bitmap = (unsigned long *)(info_frame_no*FRAME_SIZE);
	inaccessible_bitmap = (unsigned long *)(info_frame_no*FRAME_SIZE);
	
	for(int i = 0; i < FRAME_SIZE / 8; ++i)
	{
		bitmap[i] = 0;
	}
	mark_inaccessible(info_frame_no, 1);
}
    
unsigned long FramePool::get_frame() 
{
	for(int i = 0; i < FRAME_SIZE / 8; ++i)
	{
		//check to make sure there are frames available first
		if(isBlockFull(bitmap[i]) != 0)
		{
			for(int j = 0; j < 8; ++j)
			{
				//make sure it is free and not inaccessible
				if(!(bitmap[i] & power(2, j) == 0) && !(inaccessible_bitmap[i] & power(2,j) == 0))
				{
					bitmap[i] = bitmap[i] | power(2,j);
					return base_frame_no + j + i * 8;
				}
			}
		}
	}
	Console::puts("All frames taken!\n");	
	return -1;
}
    
unsigned long FramePool::release_frame(unsigned long _frame_no)
{
	for (int i = 0; i < 8; ++i)
	{
		if(bitmap[_frame_no - base_frame_no] & power(2, i) == 0)
		{
			Console::puts("Frame already free!");
			return -1;
		}
		else if(inaccessible_bitmap[_frame_no - base_frame_no] & power(2,i) == 0)
		{
			Console::puts("Frame is inaccessible, cannot free!");
			return -1;
		}
	}
	int current_frame = bitmap[(_frame_no - base_frame_no) / 8];
	current_frame = current_frame & (~power(2,(_frame_no - base_frame_no) % 8));
	return 0;
}
    
unsigned long FramePool::mark_inaccessible(unsigned long _base_frame_no, unsigned long _nframes) 
{	
	if(_base_frame_no < base_frame_no)
	{
		Console::puts("Error: Cannot mark frames before base frame inaccessible\n");
		return -1;
	}
	if(_nframes > nframes)
	{
		Console::puts("Error: Trying to mark too many frames inaccessible\n");
		return -1;
	}
	if(_base_frame_no + _nframes == base_frame_no + nframes)
	{
		Console::puts("Error: Frames out of range\n");
		return -1;
	}
	for(unsigned long i = 0; i < _nframes/8; ++i)
	{
		inaccessible_bitmap[(_base_frame_no - base_frame_no + i) / 8] |= power(2, (_base_frame_no - base_frame_no) % 8); 
	}
	return 0;
}
