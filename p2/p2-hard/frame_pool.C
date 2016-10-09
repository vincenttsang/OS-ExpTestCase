#include "frame_pool.H"
#include "console.H"

#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
#define FRAME_SIZE (4 KB)

char taken = 'T'; //classifiers
char available = 'A';
char inaccessible = 'I';

/* Constructor FramePool
     *
     * Initializes the data structures needed for the management of this
     * frame pool. This function must be called before the paging system
     * is initialized.
     * _base_frame_no is the frame number at the start of the physical memory
     *                region that this frame pool manages.
     * _nframes is the number of frames in the physical memory region that this
     *          rame pool manages.
     * e.g. If _base_frame_no is 16 and _nframes is 4, this frame pool manages
     * physical frames numbered 16, 17, 18 and 19
     * _info_frame_no is the frame number (within the directly mapped region) of
     * the frame that should be used to store the management information of the
     * frame pool. However, if _info_frame_no is 0, the frame pool is free to
     * choose any frame from the pool itself to store management information.
     */
	 
FramePool::FramePool(
			  unsigned long _base_frame_no,
              unsigned long _nframes,
              unsigned long _info_frame_no)
{
	baseFrameNo = _base_frame_no;
	nFrames = _nframes;
	infoFrameNo = _info_frame_no;
	
	
	if(infoFrameNo == 0) //get a pointer to the start address
	{
		infoFrameNo = baseFrameNo;
		frameList = (unsigned char*) (baseFrameNo*FRAME_SIZE);
	}
	else
	{
		frameList = (unsigned char*) (infoFrameNo*FRAME_SIZE);	
	}
	
	for(int i = 0; i < nFrames; ++i) //mark everything available
		*(frameList + i) = available;
	
	int requireFrame; //number of frames needed to keep track of frames
					  // at most it will be 3, 1 for kernel and 2 for user
					 // but wasn't sure if  user will be split in grading script
					 // so i did this a weird way to be safe.
					 
	if(nFrames%FRAME_SIZE != 0) //handles cases like 1.5, 1.8 , etc. 
								//we need whole number frames
		requireFrame = (nFrames/FRAME_SIZE) + 1; //int division
	else
		requireFrame = nFrames/FRAME_SIZE; //int division

	if(_info_frame_no == 0) //block the requireFrame
	{
		int i = 0;
		while(i < requireFrame) //iterate through
		{
			//inaccessible because we don't want it to be released
			// since it contain important information.
			*(frameList + i) = inaccessible;
			++i;
		}
		possiblyFreeCache = frameList + i; //update cache value
	}
	else
	{
		if(requireFrame > 1)
		{
			//maps to the kernel and allocate an extra frame because 
			//it need more frame than what kernel.c provide.
			
			unsigned long y = (infoFrameNo - ((2 MB) / (4 KB))) + (2 MB);//the real location of the frame
			unsigned char* directFrame = (unsigned char *) (y);
			for(int i = 0; i < requireFrame; ++i) //mark inaccessible
			{
				*(directFrame + i) = inaccessible;
			}
		}
		else//the one received is classified as taken so we need to update to inaccessible
		{
			unsigned long y = (infoFrameNo - ((2 MB) / (4 KB))) + (2 MB);//the real location of the frame
			unsigned char* directFrame = (unsigned char *) (y);
			*(directFrame + requireFrame) = inaccessible; 
		}
	}
	
}

/* get_frame
     *
     * Allocates a frame from the frame pool. If there is a frame free to be allocated,
     * it returns the frame’snumber. If all frames in the frame pool are taken,
     * it returns 0xffffffff (this hexadecimal number conrresponds  tounsigned long(-1).
     * Notice that our machine is a 32 bit architecture)
     */		  
unsigned long FramePool::get_frame()
{
	//check cache and if available return it
	if(*possiblyFreeCache == available)
	{
		//convert to real frame number
		unsigned long frameNo = (possiblyFreeCache - frameList) + baseFrameNo;
		*possiblyFreeCache = taken;  //update the frame
		if(!(possiblyFreeCache + 1 > (frameList + nFrames - 1)))
			possiblyFreeCache += 1;
		return frameNo; //return frame number
	}
	else
	{
		unsigned long frameNo = 0xffffffff;
		for(int i = 0; i < nFrames; ++i)
		{
			if(*(frameList + i) == available)
			{
				//convert to real frame number
				frameNo = ((frameList + i) - frameList) + baseFrameNo;
				*(frameList + i) = taken; //update the frame
				if(!(frameList + i + 1 > (frameList + nFrames - 1)))
					possiblyFreeCache = frameList + i + 1;
				return frameNo;  //return frame number
			}
		}
		return frameNo; //failure
	}
	
	return 0xffffffff; //will never make it here but just in case.
}

    /* release_frame
     *
     * Releases frame back to the frame pool.
     * The frame is identified by the frame number.
     * The function returns 0 on success and -1 on failure. Think about what failure
     * cases you need to cover. If not sure, ask in class!
     */
unsigned long FramePool::release_frame(unsigned long _frame_no)
{
	//check the range of the fram and make sure it is in this pool
	if(_frame_no > (baseFrameNo + nFrames) || _frame_no < baseFrameNo)
		return 0xffffffff;
	
	//check if the frame is inaccessible
	if(*(frameList + (_frame_no-baseFrameNo)) == inaccessible)
		return 0xffffffff;
	
	//release the frame
	*(frameList + (_frame_no-baseFrameNo)) = available;
	possiblyFreeCache = frameList + (_frame_no-baseFrameNo);
	
	return 0; //success
}

    /* mark_inaccessible
     *
     * Mark the area of physical memory as inaccessible, i.e., not available for
     * consideration by get_frame/release_frame.
     •	* The arguments have the same semantics as in the constructor.
     •	* The function returns 0 on success and -1 on failure.
     */
	 
unsigned long FramePool::mark_inaccessible(
							unsigned long _base_frame_no,
                            unsigned long _nframes)
{
	//check if the frames are part of this pool 
	if(_base_frame_no + _nframes > (baseFrameNo + nFrames) || _base_frame_no < baseFrameNo)
		return 0xffffffff;
	
	unsigned long converted = _base_frame_no - baseFrameNo;
	for(unsigned long i = converted; i < converted + _nframes; ++i)
	{
		if(*(frameList + i) == inaccessible)
		{
			return 0xffffffff;
		}
	}
	
	//mark all the frames inaccessible if it makes it here
	for(unsigned long i = converted; i < converted + _nframes; ++i)
	{
		*(frameList + i) = inaccessible;
	}
	
	return 0; //success
}