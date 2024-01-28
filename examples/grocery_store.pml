/*Grocery store*/

int basket = 3, checkout = 2;

#define take_basket atomic {(basket > 0) ->  basket--}
#define shop_and_pay atomic {(checkout > 0) -> checkout--}
#define leave atomic {checkout++; basket++}

active [4] proctype customer() 
{ 
again: 	
	take_basket;   
	shop_and_pay;    
	leave;
	goto again  	
}




