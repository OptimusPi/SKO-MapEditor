
#ifndef __SKO_SHOP_H_
#define __SKO_SHOP_H_

#include <string>

class SKO_Shop
{
      public:
             
      SKO_Shop();
	  SKO_Shop(std::string name);
             
      //simple shop def.
      //x  y  (item,price)
      unsigned int item[6][4][2];

	  // Title of the shop to display
	  std::string name = "Shop";

private:
	void clearItems();
};

#endif 
