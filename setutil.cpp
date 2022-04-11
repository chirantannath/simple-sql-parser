#ifndef __SETUTIL_IMPL__
#define __SETUTIL_IMPL__

#include "setutil.hpp"
#include <algorithm>

namespace SetUtil {
template<class T> void setify(std::vector<T> &v) {
    std::sort(v.begin(), v.end());
    auto end = std::unique(v.begin(), v.end());
    v.erase(end, v.end());
    v.shrink_to_fit();
}
template<class T> std::vector<T> difference(std::vector<T> &v1, std::vector<T> &v2) {
    setify(v1); setify(v2);
    std::vector<T> result; result.resize(v1.size());
    auto end = std::set_difference(v1.cbegin(), v1.cend(), v2.cbegin(), v2.cend(), result.begin());
    result.erase(end, result.end());
    setify(result);
    return result;
}
template<class T> std::vector<T> addVectors(const std::vector<T> &v1, const std::vector<T> &v2) {
    std::vector<T> result(v1); result.resize(v1.size() + v2.size());
    std::copy(v2.cbegin(), v2.cend(), result.begin() + v1.size());
    result.shrink_to_fit();
    return result;
}
//This one requires a type argument
template<class T, class In> std::vector<T> addVectors(In v1_begin, In v1_end, In v2_begin, In v2_end) {
    std::vector<T> result(v1_begin, v1_end); const auto sz = v1_end - v1_begin;
    result.resize(sz + (v2_end - v2_begin));
    std::copy(v2_begin, v2_end, result.begin() + sz);
    result.shrink_to_fit();
    return result;
}
template<class T, class Itr> Itr bsearch(Itr begin, Itr end, const T& val) {
    if(begin == end) return end;
    Itr result = std::lower_bound(begin, end, val);
    if(result == end || (*result) != val) return end;
    else return result;
}
template<class T> bool begins_with(const std::vector<T> &seq, const std::vector<T> &subseq) {
    if(subseq.size() > seq.size()) return false;
    for(size_t i = 0; i < subseq.size(); i++)
        if(seq[i] != subseq[i]) return false;
    return true;
}
template<class Itr> bool isdisjoint(Itr first1, Itr last1, Itr first2, Itr last2) {
    while(first1 != last1 && first2 != last2) {
        if((*first1) < (*first2)) first1++;
        else if ((*first2) < (*first1)) first2++;
        else return false; //Equal elements
    }
    return true;
}}

#endif