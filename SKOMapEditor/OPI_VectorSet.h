#ifndef __OPI_vectorSET_
#define __OPI_vectorSET_

#include <algorithm>
#include <vector>
#include <set>
#include <mutex>
#include <execution>
#include <functional>

template <class T>
class OPI_VectorSet
{
public:
	using iterator = typename std::vector<T>::iterator;
	using const_iterator = typename std::vector<T>::const_iterator;
	iterator begin() { return vectorContainer.begin(); }
	iterator end() { return vectorContainer.end(); }
	const_iterator begin() const { return vectorContainer.begin(); }
	const_iterator end() const { return vectorContainer.end(); }
	const T& front() const { return vectorContainer.front(); }
	const T& back() const { return vectorContainer.back(); }

	const T& operator[](size_t i) const
	{
		return vectorContainer[i];
	}

	// returns
	// true if inserted
	// false if already exists
	bool insert(const T& item)
	{
		auto ret = setContainer.insert(item);

		// if it was inserted into the unique set...
		if (ret.second)
		{
			// ..also add it to the vector for the first time
			vectorContainer.push_back(item);
			return true;
		}
		return false;
	}

	void upsert(const T& item)
	{
		// update if NOT inserted
		//since it's the same value, this means remove and add it-- so it goes to the "top" of the list
		if (insert(item) == false)
		{
			// remove it
			iterator position = std::find(vectorContainer.begin(), vectorContainer.end(), item);
			if (position != vectorContainer.end())
				vectorContainer.erase(position);

			// add it again to the top
			vectorContainer.push_back(item);
			return;
		}
	}

	void upsert(const std::vector<T>& items)
	{
		for (auto item : items) {
			upsert(item);
		}
	}

	//void update(const T& item, const T& newItem)
	//{
	//	// insert
	//	if (insert(item))
	//	{
	//		return true;
	//	}
	//}

	void erase(T& item) { setContainer.erase(item); vectorContainer.erase(std::remove(vectorContainer.begin(), vectorContainer.end(), item), vectorContainer.end());}
	void clear() { setContainer.clear(); vectorContainer.clear(); }

	size_t count(const T& item) const { return setContainer.count(item); }
	bool empty() const { return setContainer.empty(); }
	size_t size() const { return setContainer.size(); }

	std::vector<T> filter(std::function<bool(T)> f)
	{
		std::vector<T> out;
		std::for_each(std::execution::seq, std::begin(this->vectorContainer), std::end(this->vectorContainer),
			[&out, f](auto&& elem) {
				if (f(elem))
				{
					out.push_back(elem);
				}
			});

		return out;
	}

	std::vector<T> filter_parallel(std::function<bool(T)> f)
	{
		std::vector<T> out;
		std::mutex m;
		std::for_each(std::execution::par, std::begin(this->vectorContainer), std::end(this->vectorContainer),
			[&out, &m, f](auto&& elem) {

				if (f(elem))
				{
					std::lock_guard<std::mutex> guard(m);
					out.push_back(elem);
				}
			});

		return out;
	}

	static std::set<T> filter_set(std::set<T> s, std::function<bool(T)> f)
	{
		std::set<T> out;
		std::mutex m;
		std::for_each(std::execution::par, std::begin(s), std::end(s),
			[&out, &m, f](auto&& elem) {

				if (f(elem))
				{
					std::lock_guard<std::mutex> guard(m);
					out.insert(elem);
				}
			});

		return out;
	}


private:
	std::vector<T> vectorContainer;
	std::set<T> setContainer;
};


#endif


