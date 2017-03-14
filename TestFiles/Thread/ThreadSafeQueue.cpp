#include"ThreadSafeQueue.h"


/*
Takes an element e of class C and pushes it 
to front of the queue

@param: e element to push into queue
@return: none
*/
template<class T>
void ThreadSafeQueue<T>::push(T e){
	std::lock_guard<std::mutex> lock(m);
	q.push(e);
}
		
/*
Removes the oldest (first in) element from the 
queue and returns it

@param: eRef reference to return element with
@return: boolean that is true on success or 
false when queue is empty, error
*/
template<class T>
bool ThreadSafeQueue<T>::pop(T& eRef){
	std::lock_guard<std::mutex> lock(m);
	if(q.empty()){
		return(false);
	}
	eRef = q.front();
	q.pop();
	return(true);
}










