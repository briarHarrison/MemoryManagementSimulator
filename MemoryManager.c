//Lisa Chen and Briar Harrison
//MHC CS 322 - Operating Systems
//2017

//MemoryManager simulates the way that an operating system might handle a process' memory heap. 
//It can allocate and free memory. It cannot currently support defragmentation.

#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "mem.h"
#include <fcntl.h>

void* head;


//each header should be 16 bytes.
//addresses require 6 bytes -> pad with 0s to make 8 each
//magic number requires 2 bytes -> 0x072D -> pad with 0s to make 8
//size require ? bytes


//Mem_Init 
void *Mem_Init (int sizeOfRegion){
	
	//calculate the request size from the sizeOfRegion, rounding up if needed to fit evenly on some number of pages.
	double numPages = ceil((double) sizeOfRegion / (double) getpagesize());
	int requestSize = numPages * getpagesize();
	printf("The requestSize is %d\n", requestSize);

	// open the /dev/zero device
	int fd = open("/dev/zero", O_RDWR);

	// requestSize (in bytes) needs to be evenly divisible by the page size
	void *ptr = mmap(NULL, requestSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (ptr == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	close(fd);

	head = ptr;

	//set memory header
		//track how large the maximum size is
	int headerSize = 8;

	//set header of current free chunk
	int size = requestSize - headerSize;
	*(int*) head = size;

	int next = 0;
	void* nextPtr = head + (headerSize/2);
	*(int*) nextPtr = next;

	return ptr;
}

//helper function iterates through the memory until it finds a chunk flagged as
//free that is at least as large as the specified size
//i.e., the chunk's header does not contain the magic number 1837.
void* findFree(int size){
	printf("at the begining of findFree, and Head is found at %p with size %d.\n", head, *(int*)head);
	//for now, max size is one page = 4096
	//int maxSize = getpagesize();
	//4096 = 0b1000000000000 = 0x1000
	int offset = 4;

	void* nextHeader = head;
	
	printf("set nextHeader, and Head is found at %p with size %d.\n", head, *(int*)head);

	printf("The magic number value is %d.\n", *(int*)(nextHeader+offset));
	while ((*(int*)(nextHeader + offset)) == 1837){
		printf("So we're looking at the next chunk.\n");
		nextHeader = nextHeader + *(int*)nextHeader + (offset*2);
	}

	//find the first chunk that is at least as large as the specified size
	while (*(int*)nextHeader < size){
		nextHeader = nextHeader + *(int*)nextHeader + (offset*2);
	}

	printf("Head is found at %p with size %d.\n", head, *(int*)head);
	printf("First free memory found at %p with size %d.\n", nextHeader, *(int*)nextHeader);
	return nextHeader;
}

void Mem_Dump(){
	//find the first free chunk
	void* firstChunk = findFree(0);
	//printf("Size of first free chunk is %d\n", *(int*)head);
	//printf("Next free chunk is %d\n", *(int*)(head+4));
	printf("Free memory:\n");

	//loop until next is 0
	void* nextChunk = firstChunk;
	do{
		int* size = (int*)nextChunk;
		void* next = nextChunk+4;
		
		printf("address: %p\n", nextChunk+8);
		printf("size: %d\n", *size);
		//printf("next: %d\n", *(int*)next);

		nextChunk = next;
	} while (*(int*) nextChunk != 0);
}

void* Mem_Alloc (int size){
	int headerSize = 8;

	//find a free memory chunk that can fit the new allocation
	void* free = findFree(size);
	
	//only want to create a new header if there is enough space for it
	if (*(int*) free <= size + headerSize){
		size += headerSize;
	} else {
		//create new header after the allocated memory
			//set new header's size = oldSize - (size + header)
			//set new header's next = oldNext
		void* newHeader = free + size + headerSize;

		int newSize = *(int*)free - (size + headerSize);
		*(int*) newHeader = newSize;

		void* nextPtr = newHeader + (headerSize/2);
		if (*(int*)(free + (headerSize/2)) == 0){
			*(int*)nextPtr = 0;
		} else {
			nextPtr = (free + (headerSize/2));
		}
	}

	//change head size field to new size
	//change head next field to magic number
	*(int*) free = size;

	void* magicNumberLocation = free + (headerSize/2);
	*(int*) magicNumberLocation = 1837;

	return free;
}

//main is just for testing purposes
int main (int argc, char **argv){
	//printf("%p\n", Mem_Init(2000));
	Mem_Init(2000);
	printf("Head is found at %p with size %d.\n", head, *(int*)head);
	Mem_Dump();
	Mem_Alloc(100);
	printf("Head is found at %p with size %d.\n", head, *(int*)head);
	Mem_Dump();
	Mem_Alloc(200);
	printf("Head is found at %p with size %d.\n", head, *(int*)head);
	printf("Second chunk is found at %p with size %d.\n", (head+(*(int*)head)+8), *(int*)(head+(*(int*)head)+8));
	Mem_Dump();

	return 0;
}
