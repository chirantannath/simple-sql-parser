#ifndef __SETUTIL__
#define __SETUTIL__

#include <vector>

namespace SetUtil {

template<class T> void setify(std::vector<T>&);
template<class T> std::vector<T> difference(std::vector<T>&, std::vector<T>&);
template<class T> std::vector<T> addVectors(const std::vector<T>&, const std::vector<T>&);
//Do not use without a type argument
template<class T, class In> std::vector<T> addVectors(In, In, In, In);
template<class T, class Itr> Itr bsearch(Itr, Itr, const T&);
template<class T> bool begins_with(const std::vector<T>&, const std::vector<T>&);

}

#endif