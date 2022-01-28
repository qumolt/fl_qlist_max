# About

External for Windows version of Max 8 (Max/MSP). Acts like a queue of variable size (default maximum size: 256 elements). Receives messages of any length and stores them until bangs are received so messages are retrieved. The second outlet will show the amount of total elements in queue.
You can change the maximum size of queue with *"queue_size $1"*.
The external prepends the term *queue*.
