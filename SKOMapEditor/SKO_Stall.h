
#ifndef __SKO_STALL_H_
#define __SKO_STALL_H_

#include <string>

class SKO_Stall
{
      //to hold shops and banks...maybe more!
      public:
      
      //where is this stand at?
	  int x = 0;
	  int y = 0;

      //how wide is it? how tall? I want a button for it man.
	  int w = 0;
	  int h = 0;
      
            
      // "bank" is the bank
	  // other is a "shop"
      // maybe more yet etc
      std::string type = "";
      

      int shopId = 0;

      SKO_Stall(int shopId, int x_in, int y_in, int w_in, int h_in);
      SKO_Stall();
};

#endif
