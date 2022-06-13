#ifndef __OPI_STACK_H_
#define __OPI_STACK_H_

#include <algorithm>
#include <vector>
#include <set>
#include <mutex>
#include <execution>
#include <functional>

template <class T>
class OPI_Stack
{
public:
	std::vector<T> values;
	int position = -1;

	//OPI_Stack<T>(){}
	//virtual ~OPI_Stack<T>() {};
	T& current()
	{
		return values.at(position);
	};
	T& forward()
	{
		if (position < values.size() - 1)
			position++;
		return current();
	};
	T& back()
	{
		if (position > 0) position--;
		return current();
	};
	void init(const T& item)
	{
		clear();
		values.push_back(item);
		position = 0;
	};

	void saveHistory()
	{
		if (position < values.size() - 1)
		{
			values.erase(values.begin()+position+1, values.end());
		}

		T item = current();
		values.push_back(item);
		position++;

		if (values.size() > 25) {
			values.erase(values.begin());
			position--;
		}
	};

	// Get rid of everything and reset the position
	void clear()
	{
		this->values.clear();
		this->position = -1;
	};
private:
};

#endif

