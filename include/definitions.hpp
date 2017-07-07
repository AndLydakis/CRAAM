#pragma once

#include "config.hpp"
#include <vector>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <assert.h>

#ifdef IS_DEBUG
// TODO: this is DEBUG ONLY
#include <iostream>
#include <string>
#endif

namespace craam {

using namespace std;

/** Default precision used throughout the code.*/
typedef double prec_t;

/** Default numerical vector */
typedef vector<prec_t> numvec;

/** Default index vector */
typedef vector<long> indvec;

/** Pair of a vector and a scalar */
typedef pair<numvec, prec_t> vec_scal_t;

/** Tuple of a index, vector and a scalar */
typedef tuple<prec_t, numvec, prec_t> ind_vec_scal_t;

/** Default solution precision */
const prec_t SOLPREC = 0.0001;
/** Default number of iterations */
const unsigned long MAXITER = 100000;



/** \brief Sort indices by values in ascending order
 *
 * \param v List of values
 * \return Sorted indices
 */
template <typename T> 
inline
vector<size_t> sort_indexes(vector<T> const& v){
    // initialize original index locations
    vector<size_t> idx(v.size());
    for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;
    // sort indexes based on comparing values in v
    sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});
    return idx;
}


/** \brief Sort indices by values in descending order
 *
 * \param v List of values
 * \return Sorted indices
 */
template <typename T> 
inline
vector<size_t> sort_indexes_desc(vector<T> const& v){
    // initialize original index locations
    vector<size_t> idx(v.size());
    for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;
    // sort indexes based on comparing values in v
    sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

    return idx;
}

/**
Computes the solution of:
min_p   p^T * z
s.t.    ||p - q|| <= t
        1^T p = 1
        p >= 0

Notes
-----
This implementation works in O(n log n) time because of the sort. Using
quickselect to choose the right quantile would work in O(n) time.

This function does not check whether the probability distribution sums to 1.
**/
inline 
pair<numvec,prec_t> worstcase_l1(numvec const& z, numvec const& q, prec_t t){
    assert(*min_element(q.begin(), q.end()) >= 0 && *max_element(q.begin(), q.end()) <= 1);
    assert(z.size() > 0);
    assert(t >= 0.0 && t <= 2.0);
    assert(z.size() == q.size());

    size_t sz = z.size();

    vector<size_t> smallest = sort_indexes<prec_t>(z);
    numvec o(q);

    auto k = smallest[0];
    auto epsilon = min(t/2, 1-q[k]);

    o[k] += epsilon;

    auto i = sz - 1;
    while(epsilon > 0){
        k = smallest[i];
        auto diff = min( epsilon, o[k] );
        o[k] -= diff;
        epsilon -= diff;
        i -= 1;
    }

    auto r = inner_product(o.begin(),o.end(),z.begin(), (prec_t) 0.0);
    return make_pair(move(o),r);
}


#ifdef IS_DEBUG
/** This is a useful functionality for debugging.  */
template<class T>
std::ostream & operator<<(std::ostream &os, const std::vector<T>& vec)
{
    for(const auto& p : vec){
        cout << p << " ";
    }
    return os;
}
#endif

}
