# Producer-Consumer problem

The makefile will compile all the programs. 

(A) To execute the makefile run the command:

=> make

(B) To execute the kernel program and insert module execute the following command:

=> sudo insmod pc.ko buffer_size="size"

where, 
buffer_size : size of buffer.


(C) To execute producer and consumer program execute the following command:


=> sudo ./producer.o /dev/lp
=> sudo ./consumer.o /dev/lp






Referred from following website 
1) http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/
2) https://oscourse.github.io/slides/concurrency_semaphores_pc.pdf
3) http://www.tldp.org/LDP/lkmpg/2.6/html/
4) http://www.makelinux.net/books/lkd2/ch18lev1sec3
5) https://www.youtube.com/watch?v=TNINjHL9Utk
6) https://www.youtube.com/watch?v=2pTLehAj9fA
7) https://www.youtube.com/watch?v=NuvAjMk9bZ8&t=21s

