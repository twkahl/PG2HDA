/*Dining philosophers*/

int chopstick[5] = 1;

#define think skip
#define pick_l atomic {(chopstick[(_pid+4)%5] > 0) -> chopstick[(_pid+4)%5]--}
#define pick_r atomic {(chopstick[_pid] > 0) -> chopstick[_pid]--}
#define eat skip
#define put_l chopstick[(_pid+4)%5]++
#define put_r chopstick[_pid]++

active [5] proctype philosopher() 
{
end:
	do
	::	think;
		pick_l;
		pick_r;    
		eat;
		put_l;
		put_r
	od 	
}


