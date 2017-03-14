#ifndef THREAD_SAFE_QUEUE_H

#define THREAD_SAFE_QUEUE_H

#include <mutex>
#include <queue>


//Queue will be a thread safe queue with elements of class C
template<class T>
class ThreadSafeQueue{

	private:
		//Internal FIFO queue to be protected 
		std::queue<T> q;
		//Mutex to prevent concurrent accesses
		std::mutex m;

	public:
		/*
		Takes an element e of class T and pushes it 
		to front of the queue
		
		@param: e element to push into queue
		@return: none
		*/
		void push(T e);
		
		/*
		Removes the oldest (first in) element from the 
		queue and returns it
		
		@param: eRef reference to return element with
		@return: boolean that is true on success or 
		false when queue is empty, error
		*/
		bool pop(T& eRef);

};







#endif
