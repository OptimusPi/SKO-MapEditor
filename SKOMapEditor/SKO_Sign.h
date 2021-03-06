/*
 * SKO_Sign.h
 *
 *  Created on: Mar 13, 2013
 *      Author: midnite
 */

#ifndef SKO_SIGN_H_
#define SKO_SIGN_H_

#include "TextComponent.h"
#include <vector>

class SKO_Sign {
public:
	static const int NUM_LINES = 10;
	static const int NUM_PAGES = 2;
	SKO_Sign();
	std::vector<OPI_Text::TextComponent*> lines = std::vector<OPI_Text::TextComponent*>();;

	int x, y, w, h;
	bool triggered, hasBeenClosed;
};


#endif /* SKO_SIGN_H_ */
