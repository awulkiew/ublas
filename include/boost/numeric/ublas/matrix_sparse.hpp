//
//  Copyright (c) 2000-2002
//  Joerg Walter, Mathias Koch
//
//  Permission to use, copy, modify, distribute and sell this software
//  and its documentation for any purpose is hereby granted without fee,
//  provided that the above copyright notice appear in all copies and
//  that both that copyright notice and this permission notice appear
//  in supporting documentation.  The authors make no representations
//  about the suitability of this software for any purpose.
//  It is provided "as is" without express or implied warranty.
//
//  The authors gratefully acknowledge the support of
//  GeNeSys mbH & Co. KG in producing this work.
//

#ifndef BOOST_UBLAS_MATRIX_SPARSE_H
#define BOOST_UBLAS_MATRIX_SPARSE_H

#include <boost/numeric/ublas/vector_sparse.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/detail/matrix_assign.hpp>

// Iterators based on ideas of Jeremy Siek

namespace boost { namespace numeric { namespace ublas {

#ifdef BOOST_UBLAS_STRICT_MATRIX_SPARSE

    template<class M>
    class sparse_matrix_element:
       public container_reference<M> {
    public:
        typedef M matrix_type;
        typedef typename M::size_type size_type;
        typedef typename M::value_type value_type;
        typedef const value_type &const_reference;
        typedef value_type *pointer;

    private:
        // Proxied element operations
        void get_d () {
            pointer p = (*this) ().find_element (i_, j_);
            if (p)
                d_ = *p;
            else
                d_ = value_type (0);
        }

        void set (const value_type &s) const {
            pointer p = (*this) ().find_element (i_, j_);
            if (!p)
                (*this) ().set_element (i_, j_, s);
            else
                *p = s;
        }
        
    public:
        // Construction and destruction
        BOOST_UBLAS_INLINE
        sparse_matrix_element (matrix_type &m, size_type i, size_type j):
            container_reference<matrix_type> (m), i_ (i), j_ (j) {
        }
        BOOST_UBLAS_INLINE
        sparse_matrix_element (const sparse_matrix_element &p):
            container_reference<matrix_type> (p), i_ (p.i_), d_ (p.d_) {}
        BOOST_UBLAS_INLINE
        ~sparse_matrix_element () {
        }

        // Assignment
        BOOST_UBLAS_INLINE
        sparse_matrix_element &operator = (const sparse_matrix_element &p) {
            // Overide the implict copy assignment
            p.get_d ();
            set (p.d_);
            return *this;
        }
        template<class D>
        BOOST_UBLAS_INLINE
        sparse_matrix_element &operator = (const D &d) {
            set (d);
            return *this;
        }
        template<class D>
        BOOST_UBLAS_INLINE
        sparse_matrix_element &operator += (const D &d) {
            get_d ();
            d_ += d;
            set (d_);
            return *this;
        }
        template<class D>
        BOOST_UBLAS_INLINE
        sparse_matrix_element &operator -= (const D &d) {
            get_d ();
            d_ -= d;
            set (d_);
            return *this;
        }
        template<class D>
        BOOST_UBLAS_INLINE
        sparse_matrix_element &operator *= (const D &d) {
            get_d ();
            d_ *= d;
            set (d_);
            return *this;
        }
        template<class D>
        BOOST_UBLAS_INLINE
        sparse_matrix_element &operator /= (const D &d) {
            get_d ();
            d_ /= d;
            set (d_);
            return *this;
        }

        // Comparison
        template<class D>
        BOOST_UBLAS_INLINE
        bool operator == (const D &d) const {
            get_d ();
            return d_ == d;
        }
        template<class D>
        BOOST_UBLAS_INLINE
        bool operator != (const D &d) const {
            get_d ();
            return d_ != d;
        }

        // Conversion - weak link in proxy as d_ is not a perfect alias for the element
        BOOST_UBLAS_INLINE
        operator const_reference () const {
            get_d ();
            return d_;
        }

    private:
        size_type i_;
        size_type j_;
        value_type d_;
        bool dirty_;
    };

    template<class M>
    struct type_traits<sparse_matrix_element<M> > {
        typedef typename M::value_type element_type;
        typedef type_traits<sparse_matrix_element<M> > self_type;
        typedef typename type_traits<element_type>::value_type value_type;
        typedef typename type_traits<element_type>::const_reference const_reference;
        typedef sparse_matrix_element<M> reference;
        typedef typename type_traits<element_type>::real_type real_type;
        typedef typename type_traits<element_type>::precision_type precision_type;

        static const unsigned plus_complexity = type_traits<element_type>::plus_complexity;
        static const unsigned multiplies_complexity = type_traits<element_type>::multiplies_complexity;

        static
        BOOST_UBLAS_INLINE
        real_type real (const_reference t) {
            return type_traits<element_type>::real (t);
        }
        static
        BOOST_UBLAS_INLINE
        real_type imag (const_reference t) {
            return type_traits<element_type>::imag (t);
        }
        static
        BOOST_UBLAS_INLINE
        value_type conj (const_reference t) {
            return type_traits<element_type>::conj (t);
        }

        static
        BOOST_UBLAS_INLINE
        real_type abs (const_reference t) {
            return type_traits<element_type>::abs (t);
        }
        static
        BOOST_UBLAS_INLINE
        value_type sqrt (const_reference t) {
            return type_traits<element_type>::sqrt (t);
        }

        static
        BOOST_UBLAS_INLINE
        real_type norm_1 (const_reference t) {
            return type_traits<element_type>::norm_1 (t);
        }
        static
        BOOST_UBLAS_INLINE
        real_type norm_2 (const_reference t) {
            return type_traits<element_type>::norm_2 (t);
        }
        static
        BOOST_UBLAS_INLINE
        real_type norm_inf (const_reference t) {
            return type_traits<element_type>::norm_inf (t);
        }

        static
        BOOST_UBLAS_INLINE
        bool equals (const_reference t1, const_reference t2) {
            return type_traits<element_type>::equals (t1, t2);
        }
    };

    template<class M1, class T2>
    struct promote_traits<sparse_matrix_element<M1>, T2> {
        typedef typename promote_traits<typename sparse_matrix_element<M1>::value_type, T2>::promote_type promote_type;
    };
    template<class T1, class M2>
    struct promote_traits<T1, sparse_matrix_element<M2> > {
        typedef typename promote_traits<T1, typename sparse_matrix_element<M2>::value_type>::promote_type promote_type;
    };
    template<class M1, class M2>
    struct promote_traits<sparse_matrix_element<M1>, sparse_matrix_element<M2> > {
        typedef typename promote_traits<typename sparse_matrix_element<M1>::value_type,
                                        typename sparse_matrix_element<M2>::value_type>::promote_type promote_type;
    };

#endif


    // Index map based sparse matrix class
    template<class T, class L, class A>
    class sparse_matrix:
        public matrix_expression<sparse_matrix<T, L, A> > {

        typedef T &true_reference;
        typedef T *pointer;
        typedef const T * const_pointer;
        typedef L layout_type;
        typedef sparse_matrix<T, L, A> self_type;
    public:
#ifndef BOOST_UBLAS_NO_PROXY_SHORTCUTS
        using matrix_expression<self_type>::operator ();
#endif
        typedef typename A::size_type size_type;
        typedef typename A::difference_type difference_type;
        typedef T value_type;
        typedef A array_type;
        typedef const T &const_reference;
#ifndef BOOST_UBLAS_STRICT_MATRIX_SPARSE
        typedef typename detail::map_traits<A, T>::reference reference;
#else
        typedef sparse_matrix_element<self_type> reference;
#endif
        typedef const matrix_reference<const self_type> const_closure_type;
        typedef matrix_reference<self_type> closure_type;
        typedef sparse_vector<T, A> vector_temporary_type;
        typedef self_type matrix_temporary_type;
        typedef sparse_tag storage_category;
        typedef typename L::orientation_category orientation_category;

        // Construction and destruction
        BOOST_UBLAS_INLINE
        sparse_matrix ():
            matrix_expression<self_type> (),
            size1_ (0), size2_ (0), data_ () {}
        BOOST_UBLAS_INLINE
        sparse_matrix (size_type size1, size_type size2, size_type non_zeros = 0):
            matrix_expression<self_type> (),
            size1_ (size1), size2_ (size2), data_ () {
            detail::map_reserve (data (), max_nz (non_zeros));
        }
        BOOST_UBLAS_INLINE
        sparse_matrix (const sparse_matrix &m):
            matrix_expression<self_type> (),
            size1_ (m.size1_), size2_ (m.size2_), data_ (m.data_) {}
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_matrix (const matrix_expression<AE> &ae, size_type non_zeros = 0):
            matrix_expression<self_type> (),
            size1_ (ae ().size1 ()), size2_ (ae ().size2 ()), data_ () {
            detail::map_reserve (data (), max_nz (non_zeros));
            matrix_assign<scalar_assign> (*this, ae);
        }

        // Accessors
        BOOST_UBLAS_INLINE
        size_type size1 () const {
            return size1_;
        }
        BOOST_UBLAS_INLINE
        size_type size2 () const {
            return size2_;
        }
        BOOST_UBLAS_INLINE
        size_type non_zeros () const {
            return data_.size ();
        }
        BOOST_UBLAS_INLINE
        const array_type &data () const {
            return data_;
        }
        BOOST_UBLAS_INLINE
        array_type &data () {
            return data_;
        }

        // Resizing
    private:
        BOOST_UBLAS_INLINE
        size_type max_nz (size_type non_zeros) const {
            non_zeros = (std::max) (non_zeros, (std::min) (size1_, size2_));
            // Guarding against overflow.
            // Thanks to Alexei Novakov for the hint.
            // non_zeros_ = (std::min) (non_zeros, size1_ * size2_);
            if (size1_ > 0 && non_zeros / size1_ >= size2_)
                non_zeros = size1_ * size2_;
            return non_zeros;
        }
    public:
        BOOST_UBLAS_INLINE
        void resize (size_type size1, size_type size2, bool preserve = true) {
            // FIXME preserve unimplemented
            BOOST_UBLAS_CHECK (!preserve, internal_logic ());
            size1_ = size1;
            size2_ = size2;
            data ().clear ();
        }

        // Reserving
        BOOST_UBLAS_INLINE
        void reserve (size_type non_zeros, bool preserve = true) {
            detail::map_reserve (data (), max_nz (non_zeros));
        }

        // Element support
        BOOST_UBLAS_INLINE
        pointer find_element (size_type i, size_type j) {
            return const_cast<pointer> (const_cast<const self_type&>(*this).find_element (i, j));
        }
        BOOST_UBLAS_INLINE
        const_pointer find_element (size_type i, size_type j) const {
            const size_type element = layout_type::element (i, size1_, j, size2_);
            const_subiterator_type it (data ().find (element));
            if (it == data ().end ())
                return 0;
            BOOST_UBLAS_CHECK ((*it).first == element, internal_logic ());   // Broken map
            return &(*it).second;
        }

        // Element access
        BOOST_UBLAS_INLINE
        const_reference operator () (size_type i, size_type j) const {
            const size_type element = layout_type::element (i, size1_, j, size2_);
            const_subiterator_type it (data ().find (element));
            if (it == data ().end ())
                return zero_;
            BOOST_UBLAS_CHECK ((*it).first == element, internal_logic ());   // Broken map
            return (*it).second;
        }
        BOOST_UBLAS_INLINE
        true_reference at_element (size_type i, size_type j) {
            const size_type element = layout_type::element (i, size1_, j, size2_);
            subiterator_type it (data ().find (element));
            BOOST_UBLAS_CHECK (it != data ().end(), bad_index ());
            BOOST_UBLAS_CHECK ((*it).first == element, internal_logic ());   // Broken map
            return it->second;
        }
        BOOST_UBLAS_INLINE
        reference operator () (size_type i, size_type j) {
#ifndef BOOST_UBLAS_STRICT_MATRIX_SPARSE
            const size_type element = layout_type::element (i, size1_, j, size2_);
            std::pair<subiterator_type, bool> ii (data ().insert (typename array_type::value_type (element, value_type (0))));
            BOOST_UBLAS_CHECK ((ii.first)->first == element, internal_logic ());   // Broken map
            return (ii.first)->second;
#else
            return reference (*this, i, j);
#endif
        }

        // Element assingment
        BOOST_UBLAS_INLINE
        true_reference set_element (size_type i, size_type j, const_reference t) {
            const size_type element = layout_type::element (i, size1_, j, size2_);
            std::pair<subiterator_type, bool> ii (data ().insert (typename array_type::value_type (element, t)));
            BOOST_UBLAS_CHECK ((ii.first)->first == element, internal_logic ());   // Broken map
            if (!ii.second)     // existing element
                (ii.first)->second = t;
            return (ii.first)->second;
        }
        BOOST_UBLAS_INLINE
        void zero_element (size_type i, size_type j) {
            subiterator_type it = data ().find (layout_type::element (i, size1_, j, size2_));
            if (it == data ().end ())
                return;
            data ().erase (it);
        }
        
        // Zeroing
        BOOST_UBLAS_INLINE
        void zero () {
            data ().clear ();
        }

        // Assignment
        BOOST_UBLAS_INLINE
        sparse_matrix &operator = (const sparse_matrix &m) {
            if (this != &m) {
                size1_ = m.size1_;
                size2_ = m.size2_;
                data () = m.data ();
            }
            return *this;
        }
        BOOST_UBLAS_INLINE
        sparse_matrix &assign_temporary (sparse_matrix &m) {
            swap (m);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_matrix &operator = (const matrix_expression<AE> &ae) {
            self_type temporary (ae, detail::map_capacity (data ()));
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_matrix &assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_assign> (*this, ae);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_matrix& operator += (const matrix_expression<AE> &ae) {
            self_type temporary (*this + ae, detail::map_capacity (data ()));
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_matrix &plus_assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_plus_assign> (*this, ae);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_matrix& operator -= (const matrix_expression<AE> &ae) {
            self_type temporary (*this - ae, detail::map_capacity (data ()));
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_matrix &minus_assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_minus_assign> (*this, ae);
            return *this;
        }
        template<class AT>
        BOOST_UBLAS_INLINE
        sparse_matrix& operator *= (const AT &at) {
            matrix_assign_scalar<scalar_multiplies_assign> (*this, at);
            return *this;
        }
        template<class AT>
        BOOST_UBLAS_INLINE
        sparse_matrix& operator /= (const AT &at) {
            matrix_assign_scalar<scalar_divides_assign> (*this, at);
            return *this;
        }

        // Swapping
        BOOST_UBLAS_INLINE
        void swap (sparse_matrix &m) {
            if (this != &m) {
                std::swap (size1_, m.size1_);
                std::swap (size2_, m.size2_);
                data ().swap (m.data ());
            }
        }
        BOOST_UBLAS_INLINE
        friend void swap (sparse_matrix &m1, sparse_matrix &m2) {
            m1.swap (m2);
        }

        // Iterator types
    private:
        // Use storage iterator
        typedef typename A::const_iterator const_subiterator_type;
        typedef typename A::iterator subiterator_type;

    public:
        class const_iterator1;
        class iterator1;
        class const_iterator2;
        class iterator2;
        typedef reverse_iterator_base1<const_iterator1> const_reverse_iterator1;
        typedef reverse_iterator_base1<iterator1> reverse_iterator1;
        typedef reverse_iterator_base2<const_iterator2> const_reverse_iterator2;
        typedef reverse_iterator_base2<iterator2> reverse_iterator2;

        // Element lookup
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        const_iterator1 find1 (int rank, size_type i, size_type j, int direction = 1) const {
            const_subiterator_type it (data ().lower_bound (layout_type::address (i, size1_, j, size2_)));
            const_subiterator_type it_end (data ().end ());
            size_type index1 = size_type (-1);
            size_type index2 = size_type (-1);
            while (rank == 1 && it != it_end) {
                index1 = layout_type::index1 ((*it).first, size1_, size2_);
                index2 = layout_type::index2 ((*it).first, size1_, size2_);
                if (direction > 0) {
                    if ((index1 >= i && index2 == j) || (i >= size1_))
                        break;
                    ++ i;
                } else /* if (direction < 0) */ {
                    if ((index1 <= i && index2 == j) || (i == 0))
                        break;
                    -- i;
                }
                it = data ().lower_bound (layout_type::address (i, size1_, j, size2_));
            }
            if (rank == 1 && index2 != j) {
                if (direction > 0)
                    i = size1_;
                else /* if (direction < 0) */
                    i = 0;
                rank = 0;
            }
            return const_iterator1 (*this, rank, i, j, it);
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        iterator1 find1 (int rank, size_type i, size_type j, int direction = 1) {
            subiterator_type it (data ().lower_bound (layout_type::address (i, size1_, j, size2_)));
            subiterator_type it_end (data ().end ());
            size_type index1 = size_type (-1);
            size_type index2 = size_type (-1);
            while (rank == 1 && it != it_end) {
                index1 = layout_type::index1 ((*it).first, size1_, size2_);
                index2 = layout_type::index2 ((*it).first, size1_, size2_);
                if (direction > 0) {
                    if ((index1 >= i && index2 == j) || (i >= size1_))
                        break;
                    ++ i;
                } else /* if (direction < 0) */ {
                    if ((index1 <= i && index2 == j) || (i == 0))
                        break;
                    -- i;
                }
                it = data ().lower_bound (layout_type::address (i, size1_, j, size2_));
            }
            if (rank == 1 && index2 != j) {
                if (direction > 0)
                    i = size1_;
                else /* if (direction < 0) */
                    i = 0;
                rank = 0;
            }
            return iterator1 (*this, rank, i, j, it);
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        const_iterator2 find2 (int rank, size_type i, size_type j, int direction = 1) const {
            const_subiterator_type it (data ().lower_bound (layout_type::address (i, size1_, j, size2_)));
            const_subiterator_type it_end (data ().end ());
            size_type index1 = size_type (-1);
            size_type index2 = size_type (-1);
            while (rank == 1 && it != it_end) {
                index1 = layout_type::index1 ((*it).first, size1_, size2_);
                index2 = layout_type::index2 ((*it).first, size1_, size2_);
                if (direction > 0) {
                    if ((index2 >= j && index1 == i) || (j >= size2_))
                        break;
                    ++ j;
                } else /* if (direction < 0) */ {
                    if ((index2 <= j && index1 == i) || (j == 0))
                        break;
                    -- j;
                }
                it = data ().lower_bound (layout_type::address (i, size1_, j, size2_));
            }
            if (rank == 1 && index1 != i) {
                if (direction > 0)
                    j = size2_;
                else /* if (direction < 0) */
                    j = 0;
                rank = 0;
            }
            return const_iterator2 (*this, rank, i, j, it);
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        iterator2 find2 (int rank, size_type i, size_type j, int direction = 1) {
            subiterator_type it (data ().lower_bound (layout_type::address (i, size1_, j, size2_)));
            subiterator_type it_end (data ().end ());
            size_type index1 = size_type (-1);
            size_type index2 = size_type (-1);
            while (rank == 1 && it != it_end) {
                index1 = layout_type::index1 ((*it).first, size1_, size2_);
                index2 = layout_type::index2 ((*it).first, size1_, size2_);
                if (direction > 0) {
                    if ((index2 >= j && index1 == i) || (j >= size2_))
                        break;
                    ++ j;
                } else /* if (direction < 0) */ {
                    if ((index2 <= j && index1 == i) || (j == 0))
                        break;
                    -- j;
                }
                it = data ().lower_bound (layout_type::address (i, size1_, j, size2_));
            }
            if (rank == 1 && index1 != i) {
                if (direction > 0)
                    j = size2_;
                else /* if (direction < 0) */
                    j = 0;
                rank = 0;
            }
            return iterator2 (*this, rank, i, j, it);
        }


        class const_iterator1:
            public container_const_reference<sparse_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               const_iterator1, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename sparse_matrix::value_type value_type;
            typedef typename sparse_matrix::difference_type difference_type;
            typedef typename sparse_matrix::const_reference reference;
            typedef const typename sparse_matrix::pointer pointer;

            typedef const_iterator2 dual_iterator_type;
            typedef const_reverse_iterator2 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            const_iterator1 ():
                container_const_reference<self_type> (), rank_ (), i_ (), j_ (), it_ () {}
            BOOST_UBLAS_INLINE
            const_iterator1 (const self_type &m, int rank, size_type i, size_type j, const const_subiterator_type &it):
                container_const_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), it_ (it) {}
            BOOST_UBLAS_INLINE
            const_iterator1 (const iterator1 &it):
                container_const_reference<self_type> (it ()), rank_ (it.rank_), i_ (it.i_), j_ (it.j_), it_ (it.it_) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            const_iterator1 &operator ++ () {
                if (rank_ == 1 && layout_type::fast1 ())
                    ++ it_;
                else
                    *this = (*this) ().find1 (rank_, index1 () + 1, j_, 1);
                return *this;
            }
            BOOST_UBLAS_INLINE
            const_iterator1 &operator -- () {
                if (rank_ == 1 && layout_type::fast1 ())
                    -- it_;
                else
                    *this = (*this) ().find1 (rank_, index1 () - 1, j_, -1);
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            const_reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*it_).second;
                } else {
                    return (*this) () (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator2 begin () const {
                const self_type &m = (*this) ();
                return m.find2 (1, index1 (), 0);
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator2 end () const {
                const self_type &m = (*this) ();
                return m.find2 (1, index1 (), m.size2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator2 rbegin () const {
                return const_reverse_iterator2 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator2 rend () const {
                return const_reverse_iterator2 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    const self_type &m = (*this) ();
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*it_).first, m.size1 (), m.size2 ()) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*it_).first, m.size1 (), m.size2 ());
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    const self_type &m = (*this) ();
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*it_).first, m.size1 (), m.size2 ()) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*it_).first, m.size1 (), m.size2 ());
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            const_iterator1 &operator = (const const_iterator1 &it) {
                container_const_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const const_iterator1 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            const_subiterator_type it_;
        };

        BOOST_UBLAS_INLINE
        const_iterator1 begin1 () const {
            return find1 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        const_iterator1 end1 () const {
            return find1 (0, size1_, 0);
        }

        class iterator1:
            public container_reference<sparse_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               iterator1, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename sparse_matrix::value_type value_type;
            typedef typename sparse_matrix::difference_type difference_type;
            typedef typename sparse_matrix::true_reference reference;
            typedef typename sparse_matrix::pointer pointer;

            typedef iterator2 dual_iterator_type;
            typedef reverse_iterator2 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            iterator1 ():
                container_reference<self_type> (), rank_ (), i_ (), j_ (), it_ () {}
            BOOST_UBLAS_INLINE
            iterator1 (self_type &m, int rank, size_type i, size_type j, const subiterator_type &it):
                container_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), it_ (it) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            iterator1 &operator ++ () {
                if (rank_ == 1 && layout_type::fast1 ())
                    ++ it_;
                else
                    *this = (*this) ().find1 (rank_, index1 () + 1, j_, 1);
                return *this;
            }
            BOOST_UBLAS_INLINE
            iterator1 &operator -- () {
                if (rank_ == 1 && layout_type::fast1 ())
                    -- it_;
                else
                    *this = (*this) ().find1 (rank_, index1 () - 1, j_, -1);
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*it_).second;
                } else {
                    return (*this) ().at_element (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator2 begin () const {
                self_type &m = (*this) ();
                return m.find2 (1, index1 (), 0);
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator2 end () const {
                self_type &m = (*this) ();
                return m.find2 (1, index1 (), m.size2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator2 rbegin () const {
                return reverse_iterator2 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator2 rend () const {
                return reverse_iterator2 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    const self_type &m = (*this) ();
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*it_).first, m.size1 (), m.size2 ()) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*it_).first, m.size1 (), m.size2 ());
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    const self_type &m = (*this) ();
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*it_).first, m.size1 (), m.size2 ()) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*it_).first, m.size1 (), m.size2 ());
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            iterator1 &operator = (const iterator1 &it) {
                container_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const iterator1 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            subiterator_type it_;

            friend class const_iterator1;
        };

        BOOST_UBLAS_INLINE
        iterator1 begin1 () {
            return find1 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        iterator1 end1 () {
            return find1 (0, size1_, 0);
        }

        class const_iterator2:
            public container_const_reference<sparse_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               const_iterator2, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename sparse_matrix::value_type value_type;
            typedef typename sparse_matrix::difference_type difference_type;
            typedef typename sparse_matrix::const_reference reference;
            typedef const typename sparse_matrix::pointer pointer;

            typedef const_iterator1 dual_iterator_type;
            typedef const_reverse_iterator1 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            const_iterator2 ():
                container_const_reference<self_type> (), rank_ (), i_ (), j_ (), it_ () {}
            BOOST_UBLAS_INLINE
            const_iterator2 (const self_type &m, int rank, size_type i, size_type j, const const_subiterator_type &it):
                container_const_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), it_ (it) {}
            BOOST_UBLAS_INLINE
            const_iterator2 (const iterator2 &it):
                container_const_reference<self_type> (it ()), rank_ (it.rank_), i_ (it.i_), j_ (it.j_), it_ (it.it_) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            const_iterator2 &operator ++ () {
                if (rank_ == 1 && layout_type::fast2 ())
                    ++ it_;
                else
                    *this = (*this) ().find2 (rank_, i_, index2 () + 1, 1);
                return *this;
            }
            BOOST_UBLAS_INLINE
            const_iterator2 &operator -- () {
                if (rank_ == 1 && layout_type::fast2 ())
                    -- it_;
                else
                    *this = (*this) ().find2 (rank_, i_, index2 () - 1, -1);
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            const_reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*it_).second;
                } else {
                    return (*this) () (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator1 begin () const {
                const self_type &m = (*this) ();
                return m.find1 (1, 0, index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator1 end () const {
                const self_type &m = (*this) ();
                return m.find1 (1, m.size1 (), index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator1 rbegin () const {
                return const_reverse_iterator1 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator1 rend () const {
                return const_reverse_iterator1 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    const self_type &m = (*this) ();
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*it_).first, m.size1 (), m.size2 ()) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*it_).first, m.size1 (), m.size2 ());
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    const self_type &m = (*this) ();
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*it_).first, m.size1 (), m.size2 ()) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*it_).first, m.size1 (), m.size2 ());
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            const_iterator2 &operator = (const const_iterator2 &it) {
                container_const_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const const_iterator2 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            const_subiterator_type it_;
        };

        BOOST_UBLAS_INLINE
        const_iterator2 begin2 () const {
            return find2 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        const_iterator2 end2 () const {
            return find2 (0, 0, size2_);
        }

        class iterator2:
            public container_reference<sparse_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               iterator2, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename sparse_matrix::value_type value_type;
            typedef typename sparse_matrix::difference_type difference_type;
            typedef typename sparse_matrix::true_reference reference;
            typedef typename sparse_matrix::pointer pointer;

            typedef iterator1 dual_iterator_type;
            typedef reverse_iterator1 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            iterator2 ():
                container_reference<self_type> (), rank_ (), i_ (), j_ (), it_ () {}
            BOOST_UBLAS_INLINE
            iterator2 (self_type &m, int rank, size_type i, size_type j, const subiterator_type &it):
                container_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), it_ (it) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            iterator2 &operator ++ () {
                if (rank_ == 1 && layout_type::fast2 ())
                    ++ it_;
                else
                    *this = (*this) ().find2 (rank_, i_, index2 () + 1, 1);
                return *this;
            }
            BOOST_UBLAS_INLINE
            iterator2 &operator -- () {
                if (rank_ == 1 && layout_type::fast2 ())
                    -- it_;
                else
                    *this = (*this) ().find2 (rank_, i_, index2 () - 1, -1);
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*it_).second;
                } else {
                    return (*this) ().at_element (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator1 begin () const {
                self_type &m = (*this) ();
                return m.find1 (1, 0, index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator1 end () const {
                self_type &m = (*this) ();
                return m.find1 (1, m.size1 (), index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator1 rbegin () const {
                return reverse_iterator1 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator1 rend () const {
                return reverse_iterator1 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    const self_type &m = (*this) ();
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*it_).first, m.size1 (), m.size2 ()) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*it_).first, m.size1 (), m.size2 ());
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    const self_type &m = (*this) ();
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*it_).first, m.size1 (), m.size2 ()) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*it_).first, m.size1 (), m.size2 ());
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            iterator2 &operator = (const iterator2 &it) {
                container_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const iterator2 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            subiterator_type it_;

            friend class const_iterator2;
        };

        BOOST_UBLAS_INLINE
        iterator2 begin2 () {
            return find2 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        iterator2 end2 () {
            return find2 (0, 0, size2_);
        }

        // Reverse iterators

        BOOST_UBLAS_INLINE
        const_reverse_iterator1 rbegin1 () const {
            return const_reverse_iterator1 (end1 ());
        }
        BOOST_UBLAS_INLINE
        const_reverse_iterator1 rend1 () const {
            return const_reverse_iterator1 (begin1 ());
        }

        BOOST_UBLAS_INLINE
        reverse_iterator1 rbegin1 () {
            return reverse_iterator1 (end1 ());
        }
        BOOST_UBLAS_INLINE
        reverse_iterator1 rend1 () {
            return reverse_iterator1 (begin1 ());
        }

        BOOST_UBLAS_INLINE
        const_reverse_iterator2 rbegin2 () const {
            return const_reverse_iterator2 (end2 ());
        }
        BOOST_UBLAS_INLINE
        const_reverse_iterator2 rend2 () const {
            return const_reverse_iterator2 (begin2 ());
        }

        BOOST_UBLAS_INLINE
        reverse_iterator2 rbegin2 () {
            return reverse_iterator2 (end2 ());
        }
        BOOST_UBLAS_INLINE
        reverse_iterator2 rend2 () {
            return reverse_iterator2 (begin2 ());
        }

    private:
        size_type size1_;
        size_type size2_;
        array_type data_;
        static const value_type zero_;
    };

    template<class T, class L, class A>
    const typename sparse_matrix<T, L, A>::value_type sparse_matrix<T, L, A>::zero_ (0);


    // Vector index map based sparse matrix class
    template<class T, class L, class A>
    class sparse_vector_of_sparse_vector:
        public matrix_expression<sparse_vector_of_sparse_vector<T, L, A> > {

        typedef T &true_reference;
        typedef T *pointer;
        typedef const T * const_pointer;
        typedef A array_type;
        typedef const A const_array_type;
        typedef L layout_type;
        typedef sparse_vector_of_sparse_vector<T, L, A> self_type;
    public:
#ifndef BOOST_UBLAS_NO_PROXY_SHORTCUTS
        using matrix_expression<self_type>::operator ();
#endif
        typedef typename A::size_type size_type;
        typedef typename A::difference_type difference_type;
        typedef T value_type;
        typedef const T &const_reference;
#ifndef BOOST_UBLAS_STRICT_MATRIX_SPARSE
        typedef typename detail::map_traits<typename A::data_value_type, T>::reference reference;
#else
        typedef sparse_matrix_element<self_type> reference;
#endif
        typedef const matrix_reference<const self_type> const_closure_type;
        typedef matrix_reference<self_type> closure_type;
        typedef sparse_vector<T, typename A::value_type> vector_temporary_type;
        typedef self_type matrix_temporary_type;
        typedef typename A::value_type::second_type vector_data_value_type;
        typedef sparse_tag storage_category;
        typedef typename L::orientation_category orientation_category;

        // Construction and destruction
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector ():
            matrix_expression<self_type> (),
            size1_ (0), size2_ (0), non_zeros_ (0), data_ () {
            data_ [layout_type::size1 (size1_, size2_)] = vector_data_value_type ();
        }
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector (size_type size1, size_type size2, size_type non_zeros = 0):
            matrix_expression<self_type> (),
            size1_ (size1), size2_ (size2), non_zeros_ (non_zeros), data_ () {
            data_ [layout_type::size1 (size1_, size2_)] = vector_data_value_type ();
        }
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector (const sparse_vector_of_sparse_vector &m):
            matrix_expression<self_type> (),
            size1_ (m.size1_), size2_ (m.size2_), non_zeros_ (m.non_zeros_), data_ (m.data_) {}
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector (const matrix_expression<AE> &ae, size_type non_zeros = 0):
            matrix_expression<self_type> (),
            size1_ (ae ().size1 ()), size2_ (ae ().size2 ()), non_zeros_ (non_zeros), data_ () {
            data_ [layout_type::size1 (size1_, size2_)] = vector_data_value_type ();
            matrix_assign<scalar_assign> (*this, ae);
        }

        // Accessors
        BOOST_UBLAS_INLINE
        size_type size1 () const {
            return size1_;
        }
        BOOST_UBLAS_INLINE
        size_type size2 () const {
            return size2_;
        }
        BOOST_UBLAS_INLINE
        size_type non_zeros () const {
            size_type non_zeros = 0;
            for (vector_const_subiterator_type itv = data_ ().begin (); itv != data_ ().end (); ++ itv)
                non_zeros += (*itv).size ();
            return non_zeros;
        }
        BOOST_UBLAS_INLINE
        const_array_type &data () const {
            return data_;
        }
        BOOST_UBLAS_INLINE
        array_type &data () {
            return data_;
        }

        // Resizing
        BOOST_UBLAS_INLINE
        void resize (size_type size1, size_type size2, bool preserve = true) {
            // FIXME preserve unimplemented
            BOOST_UBLAS_CHECK (!preserve, internal_logic ());
            size1_ = size1;
            size2_ = size2;
            data ().clear ();
            data () [layout_type::size1 (size1_, size2_)] = vector_data_value_type ();
        }

        // Element support
        BOOST_UBLAS_INLINE
        pointer find_element (size_type i, size_type j) {
            return const_cast<pointer> (const_cast<const self_type&>(*this).find_element (i, j));
        }
        BOOST_UBLAS_INLINE
        const_pointer find_element (size_type i, size_type j) const {
            const size_type element1 = layout_type::element1 (i, size1_, j, size2_);
            const size_type element2 = layout_type::element2 (i, size1_, j, size2_);
            vector_const_subiterator_type itv (data ().find (element1));
            if (itv == data ().end ())
                return 0;
            BOOST_UBLAS_CHECK ((*itv).first == element1, internal_logic ());   // Broken map
            const_subiterator_type it ((*itv).second.find (element2));
            if (it == (*itv).second.end ())
                return 0;
            BOOST_UBLAS_CHECK ((*itv).first == element1, internal_logic ());   // Broken map
            return &(*it).second;
        }

        // Element access
        BOOST_UBLAS_INLINE
        const_reference operator () (size_type i, size_type j) const {
            const size_type element1 = layout_type::element1 (i, size1_, j, size2_);
            const size_type element2 = layout_type::element2 (i, size1_, j, size2_);
            vector_const_subiterator_type itv (data ().find (element1));
            if (itv == data ().end ())
                return zero_;
            BOOST_UBLAS_CHECK ((*itv).first == element1, internal_logic ());   // Broken map
            const_subiterator_type it ((*itv).second.find (element2));
            if (it == (*itv).second.end ())
                return zero_;
            BOOST_UBLAS_CHECK ((*itv).first == element1, internal_logic ());   // Broken map
            return (*it).second;
        }
        BOOST_UBLAS_INLINE
        true_reference at_element (size_type i, size_type j) {
            const size_type element1 = layout_type::element1 (i, size1_, j, size2_);
            const size_type element2 = layout_type::element2 (i, size1_, j, size2_);
            vector_subiterator_type itv (data ().find (element1));
            BOOST_UBLAS_CHECK (itv != data ().end(), bad_index ());
            BOOST_UBLAS_CHECK ((*itv).first == element1, internal_logic ());   // Broken map
            subiterator_type it ((*itv).second.find (element2));
            BOOST_UBLAS_CHECK (it != (*itv).second.end (), bad_index ());
            BOOST_UBLAS_CHECK ((*it).first == element2, internal_logic ());    // Broken map
            
            return it->second;
        }
        BOOST_UBLAS_INLINE
        reference operator () (size_type i, size_type j) {
#ifndef BOOST_UBLAS_STRICT_MATRIX_SPARSE
            const size_type element1 = layout_type::element1 (i, size1_, j, size2_);
            const size_type element2 = layout_type::element2 (i, size1_, j, size2_);
            vector_data_value_type& vd (data () [element1]);
            std::pair<subiterator_type, bool> ii (vd.insert (typename array_type::value_type::second_type::value_type (element2, value_type (0))));
            BOOST_UBLAS_CHECK ((ii.first)->first == element2, internal_logic ());   // Broken map
            return (ii.first)->second;
#else
            return reference (*this, i, j);
#endif
        }

        // Element assignment
        BOOST_UBLAS_INLINE
        true_reference set_element (size_type i, size_type j, const_reference t) {
            const size_type element1 = layout_type::element1 (i, size1_, j, size2_);
            const size_type element2 = layout_type::element2 (i, size1_, j, size2_);

            vector_data_value_type& vd (data () [element1]);
            std::pair<subiterator_type, bool> ii (vd.insert (typename array_type::value_type::second_type::value_type (element2, t)));
            BOOST_UBLAS_CHECK ((ii.first)->first == element2, internal_logic ());   // Broken map
            if (!ii.second)     // existing element
                (ii.first)->second = t;
            return (ii.first)->second;
        }
        BOOST_UBLAS_INLINE
        void zero_element (size_type i, size_type j) {
            vector_subiterator_type itv (data ().find (layout_type::element1 (i, size1_, j, size2_)));
            if (itv == data ().end ())
                return;
            subiterator_type it ((*itv).second.find (layout_type::element2 (i, size1_, j, size2_)));
            if (it == (*itv).second.end ())
                return;
            (*itv).second.erase (it);
        }
        
        // Zeroing
        BOOST_UBLAS_INLINE
        void zero () {
            data ().clear ();
            data_ [layout_type::size1 (size1_, size2_)] = vector_data_value_type ();
        }

        // Assignment
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector &operator = (const sparse_vector_of_sparse_vector &m) {
            if (this != &m) {
                size1_ = m.size1_;
                size2_ = m.size2_;
                non_zeros_ = m.non_zeros_;
                data () = m.data ();
            }
            return *this;
        }
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector &assign_temporary (sparse_vector_of_sparse_vector &m) {
            swap (m);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector &operator = (const matrix_expression<AE> &ae) {
            self_type temporary (ae, non_zeros_);
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector &assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_assign> (*this, ae);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector& operator += (const matrix_expression<AE> &ae) {
            self_type temporary (*this + ae, non_zeros_);
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector &plus_assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_plus_assign> (*this, ae);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector& operator -= (const matrix_expression<AE> &ae) {
            self_type temporary (*this - ae, non_zeros_);
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector &minus_assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_minus_assign> (*this, ae);
            return *this;
        }
        template<class AT>
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector& operator *= (const AT &at) {
            matrix_assign_scalar<scalar_multiplies_assign> (*this, at);
            return *this;
        }
        template<class AT>
        BOOST_UBLAS_INLINE
        sparse_vector_of_sparse_vector& operator /= (const AT &at) {
            matrix_assign_scalar<scalar_divides_assign> (*this, at);
            return *this;
        }

        // Swapping
        BOOST_UBLAS_INLINE
        void swap (sparse_vector_of_sparse_vector &m) {
            if (this != &m) {
                std::swap (size1_, m.size1_);
                std::swap (size2_, m.size2_);
                std::swap (non_zeros_, m.non_zeros_);
                data ().swap (m.data ());
            }
        }
        BOOST_UBLAS_INLINE
        friend void swap (sparse_vector_of_sparse_vector &m1, sparse_vector_of_sparse_vector &m2) {
            m1.swap (m2);
        }

        // Iterator types
    private:
        // Use storage iterators
        typedef typename A::const_iterator vector_const_subiterator_type;
        typedef typename A::iterator vector_subiterator_type;
        typedef typename A::value_type::second_type::const_iterator const_subiterator_type;
        typedef typename A::value_type::second_type::iterator subiterator_type;

    public:
        class const_iterator1;
        class iterator1;
        class const_iterator2;
        class iterator2;
        typedef reverse_iterator_base1<const_iterator1> const_reverse_iterator1;
        typedef reverse_iterator_base1<iterator1> reverse_iterator1;
        typedef reverse_iterator_base2<const_iterator2> const_reverse_iterator2;
        typedef reverse_iterator_base2<iterator2> reverse_iterator2;

        // Element lookup
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        const_iterator1 find1 (int rank, size_type i, size_type j, int direction = 1) const {
            BOOST_UBLAS_CHECK (data ().begin () != data ().end (), internal_logic ());
            for (;;) {
                vector_const_subiterator_type itv (data ().lower_bound (layout_type::address1 (i, size1_, j, size2_)));
                vector_const_subiterator_type itv_end (data ().end ());
                if (itv == itv_end)
                    return const_iterator1 (*this, rank, i, j, itv_end, (*(-- itv)).second.end ());

                const_subiterator_type it ((*itv).second.lower_bound (layout_type::address2 (i, size1_, j, size2_)));
                const_subiterator_type it_end ((*itv).second.end ());
                if (rank == 0)
                    return const_iterator1 (*this, rank, i, j, itv, it);
                if (it != it_end && (*it).first == layout_type::address2 (i, size1_, j, size2_))
                    return const_iterator1 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast1 ()) {
                        if (it == it_end)
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        i = (*it).first;
                    } else {
                        if (i >= size1_)
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        ++ i;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast1 ()) {
                        if (it == (*itv).second.begin ())
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        -- it;
                        i = (*it).first;
                    } else {
                        if (i == 0)
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        -- i;
                    }
                }
            }
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        iterator1 find1 (int rank, size_type i, size_type j, int direction = 1) {
            BOOST_UBLAS_CHECK (data ().begin () != data ().end (), internal_logic ());
            for (;;) {
                vector_subiterator_type itv (data ().lower_bound (layout_type::address1 (i, size1_, j, size2_)));
                vector_subiterator_type itv_end (data ().end ());
                if (itv == itv_end)
                    return iterator1 (*this, rank, i, j, itv_end, (*(-- itv)).second.end ());

                subiterator_type it ((*itv).second.lower_bound (layout_type::address2 (i, size1_, j, size2_)));
                subiterator_type it_end ((*itv).second.end ());
                if (rank == 0)
                    return iterator1 (*this, rank, i, j, itv, it);
                if (it != it_end && (*it).first == layout_type::address2 (i, size1_, j, size2_))
                    return iterator1 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast1 ()) {
                        if (it == it_end)
                            return iterator1 (*this, rank, i, j, itv, it);
                        i = (*it).first;
                    } else {
                        if (i >= size1_)
                            return iterator1 (*this, rank, i, j, itv, it);
                        ++ i;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast1 ()) {
                        if (it == (*itv).second.begin ())
                            return iterator1 (*this, rank, i, j, itv, it);
                        -- it;
                        i = (*it).first;
                    } else {
                        if (i == 0)
                            return iterator1 (*this, rank, i, j, itv, it);
                        -- i;
                    }
                }
            }
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        const_iterator2 find2 (int rank, size_type i, size_type j, int direction = 1) const {
            BOOST_UBLAS_CHECK (data ().begin () != data ().end (), internal_logic ());
            for (;;) {
                vector_const_subiterator_type itv (data ().lower_bound (layout_type::address1 (i, size1_, j, size2_)));
                vector_const_subiterator_type itv_end (data ().end ());
                if (itv == itv_end)
                    return const_iterator2 (*this, rank, i, j, itv_end, (*(-- itv)).second.end ());

                const_subiterator_type it ((*itv).second.lower_bound (layout_type::address2 (i, size1_, j, size2_)));
                const_subiterator_type it_end ((*itv).second.end ());
                if (rank == 0)
                    return const_iterator2 (*this, rank, i, j, itv, it);
                if (it != it_end && (*it).first == layout_type::address2 (i, size1_, j, size2_))
                    return const_iterator2 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast2 ()) {
                        if (it == it_end)
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        j = (*it).first;
                    } else {
                        if (j >= size2_)
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        ++ j;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast2 ()) {
                        if (it == (*itv).second.begin ())
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        -- it;
                        j = (*it).first;
                    } else {
                        if (j == 0)
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        -- j;
                    }
                }
            }
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        iterator2 find2 (int rank, size_type i, size_type j, int direction = 1) {
            BOOST_UBLAS_CHECK (data ().begin () != data ().end (), internal_logic ());
            for (;;) {
                vector_subiterator_type itv (data ().lower_bound (layout_type::address1 (i, size1_, j, size2_)));
                vector_subiterator_type itv_end (data ().end ());
                if (itv == itv_end)
                    return iterator2 (*this, rank, i, j, itv_end, (*(-- itv)).second.end ());

                subiterator_type it ((*itv).second.lower_bound (layout_type::address2 (i, size1_, j, size2_)));
                subiterator_type it_end ((*itv).second.end ());
                if (rank == 0)
                    return iterator2 (*this, rank, i, j, itv, it);
                if (it != it_end && (*it).first == layout_type::address2 (i, size1_, j, size2_))
                    return iterator2 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast2 ()) {
                        if (it == it_end)
                            return iterator2 (*this, rank, i, j, itv, it);
                        j = (*it).first;
                    } else {
                        if (j >= size2_)
                            return iterator2 (*this, rank, i, j, itv, it);
                        ++ j;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast2 ()) {
                        if (it == (*itv).second.begin ())
                            return iterator2 (*this, rank, i, j, itv, it);
                        -- it;
                        j = (*it).first;
                    } else {
                        if (j == 0)
                            return iterator2 (*this, rank, i, j, itv, it);
                        -- j;
                    }
                }
            }
        }

        class const_iterator1:
            public container_const_reference<sparse_vector_of_sparse_vector>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               const_iterator1, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename sparse_vector_of_sparse_vector::value_type value_type;
            typedef typename sparse_vector_of_sparse_vector::difference_type difference_type;
            typedef typename sparse_vector_of_sparse_vector::const_reference reference;
            typedef const typename sparse_vector_of_sparse_vector::pointer pointer;

            typedef const_iterator2 dual_iterator_type;
            typedef const_reverse_iterator2 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            const_iterator1 ():
                container_const_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            const_iterator1 (const self_type &m, int rank, size_type i, size_type j, const vector_const_subiterator_type &itv, const const_subiterator_type &it):
                container_const_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}
            BOOST_UBLAS_INLINE
            const_iterator1 (const iterator1 &it):
                container_const_reference<self_type> (it ()), rank_ (it.rank_), i_ (it.i_), j_ (it.j_), itv_ (it.itv_), it_ (it.it_) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            const_iterator1 &operator ++ () {
                if (rank_ == 1 && layout_type::fast1 ())
                    ++ it_;
                else {
                    const self_type &m = (*this) ();
                    i_ = index1 () + 1;
                    if (rank_ == 1 && ++ itv_ == m.end1 ().itv_)
                        *this = m.find1 (rank_, i_, j_, 1);
                    else if (rank_ == 1) {
                        it_ = (*itv_).second.begin ();
                        if (it_ == (*itv_).second.end () || index2 () != j_)
                            *this = m.find1 (rank_, i_, j_, 1);
                    }
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            const_iterator1 &operator -- () {
                if (rank_ == 1 && layout_type::fast1 ())
                    -- it_;
                else {
                    const self_type &m = (*this) ();
                    i_ = index1 () - 1;
                    if (rank_ == 1 && -- itv_ == m.end1 ().itv_)
                        *this = m.find1 (rank_, i_, j_, -1);
                    else if (rank_ == 1) {
                        it_ = (*itv_).second.begin ();
                        if (it_ == (*itv_).second.end () || index2 () != j_)
                            *this = m.find1 (rank_, i_, j_, -1);
                    }
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            const_reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*it_).second;
                } else {
                    return (*this) () (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator2 begin () const {
                const self_type &m = (*this) ();
                return m.find2 (1, index1 (), 0);
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator2 end () const {
                const self_type &m = (*this) ();
                return m.find2 (1, index1 (), m.size2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator2 rbegin () const {
                return const_reverse_iterator2 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator2 rend () const {
                return const_reverse_iterator2 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*itv_).first, (*it_).first) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*itv_).first, (*it_).first);
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*itv_).first, (*it_).first) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*itv_).first, (*it_).first);
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            const_iterator1 &operator = (const const_iterator1 &it) {
                container_const_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const const_iterator1 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_const_subiterator_type itv_;
            const_subiterator_type it_;
        };

        BOOST_UBLAS_INLINE
        const_iterator1 begin1 () const {
            return find1 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        const_iterator1 end1 () const {
            return find1 (0, size1_, 0);
        }

        class iterator1:
            public container_reference<sparse_vector_of_sparse_vector>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               iterator1, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename sparse_vector_of_sparse_vector::value_type value_type;
            typedef typename sparse_vector_of_sparse_vector::difference_type difference_type;
            typedef typename sparse_vector_of_sparse_vector::true_reference reference;
            typedef typename sparse_vector_of_sparse_vector::pointer pointer;

            typedef iterator2 dual_iterator_type;
            typedef reverse_iterator2 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            iterator1 ():
                container_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            iterator1 (self_type &m, int rank, size_type i, size_type j, const vector_subiterator_type &itv, const subiterator_type &it):
                container_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            iterator1 &operator ++ () {
                if (rank_ == 1 && layout_type::fast1 ())
                    ++ it_;
                else {
                    self_type &m = (*this) ();
                    i_ = index1 () + 1;
                    if (rank_ == 1 && ++ itv_ == m.end1 ().itv_)
                        *this = m.find1 (rank_, i_, j_, 1);
                    else if (rank_ == 1) {
                        it_ = (*itv_).second.begin ();
                        if (it_ == (*itv_).second.end () || index2 () != j_)
                            *this = m.find1 (rank_, i_, j_, 1);
                    }
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            iterator1 &operator -- () {
                if (rank_ == 1 && layout_type::fast1 ())
                    -- it_;
                else {
                    self_type &m = (*this) ();
                    i_ = index1 () - 1;
                    if (rank_ == 1 && -- itv_ == m.end1 ().itv_)
                        *this = m.find1 (rank_, i_, j_, -1);
                    else if (rank_ == 1) {
                        it_ = (*itv_).second.begin ();
                        if (it_ == (*itv_).second.end () || index2 () != j_)
                            *this = m.find1 (rank_, i_, j_, -1);
                    }
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*it_).second;
                } else {
                    return (*this) ().at_element (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator2 begin () const {
                self_type &m = (*this) ();
                return m.find2 (1, index1 (), 0);
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator2 end () const {
                self_type &m = (*this) ();
                return m.find2 (1, index1 (), m.size2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator2 rbegin () const {
                return reverse_iterator2 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator2 rend () const {
                return reverse_iterator2 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*itv_).first, (*it_).first) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*itv_).first, (*it_).first);
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*itv_).first, (*it_).first) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*itv_).first, (*it_).first);
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            iterator1 &operator = (const iterator1 &it) {
                container_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const iterator1 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_subiterator_type itv_;
            subiterator_type it_;

            friend class const_iterator1;
        };

        BOOST_UBLAS_INLINE
        iterator1 begin1 () {
            return find1 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        iterator1 end1 () {
            return find1 (0, size1_, 0);
        }

        class const_iterator2:
            public container_const_reference<sparse_vector_of_sparse_vector>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               const_iterator2, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename sparse_vector_of_sparse_vector::value_type value_type;
            typedef typename sparse_vector_of_sparse_vector::difference_type difference_type;
            typedef typename sparse_vector_of_sparse_vector::const_reference reference;
            typedef const typename sparse_vector_of_sparse_vector::pointer pointer;

            typedef const_iterator1 dual_iterator_type;
            typedef const_reverse_iterator1 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            const_iterator2 ():
                container_const_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            const_iterator2 (const self_type &m, int rank, size_type i, size_type j, const vector_const_subiterator_type &itv, const const_subiterator_type &it):
                container_const_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}
            BOOST_UBLAS_INLINE
            const_iterator2 (const iterator2 &it):
                container_const_reference<self_type> (it ()), rank_ (it.rank_), i_ (it.i_), j_ (it.j_), itv_ (it.itv_), it_ (it.it_) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            const_iterator2 &operator ++ () {
                if (rank_ == 1 && layout_type::fast2 ())
                    ++ it_;
                else {
                    const self_type &m = (*this) ();
                    j_ = index2 () + 1;
                    if (rank_ == 1 && ++ itv_ == m.end2 ().itv_)
                        *this = m.find2 (rank_, i_, j_, 1);
                    else if (rank_ == 1) {
                        it_ = (*itv_).second.begin ();
                        if (it_ == (*itv_).second.end () || index1 () != i_)
                            *this = m.find2 (rank_, i_, j_, 1);
                    }
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            const_iterator2 &operator -- () {
                if (rank_ == 1 && layout_type::fast2 ())
                    -- it_;
                else {
                    const self_type &m = (*this) ();
                    j_ = index2 () - 1;
                    if (rank_ == 1 && -- itv_ == m.end2 ().itv_)
                        *this = m.find2 (rank_, i_, j_, -1);
                    else if (rank_ == 1) {
                        it_ = (*itv_).second.begin ();
                        if (it_ == (*itv_).second.end () || index1 () != i_)
                            *this = m.find2 (rank_, i_, j_, -1);
                    }
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            const_reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*it_).second;
                } else {
                    return (*this) () (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator1 begin () const {
                const self_type &m = (*this) ();
                return m.find1 (1, 0, index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator1 end () const {
                const self_type &m = (*this) ();
                return m.find1 (1, m.size1 (), index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator1 rbegin () const {
                return const_reverse_iterator1 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator1 rend () const {
                return const_reverse_iterator1 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*itv_).first, (*it_).first) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*itv_).first, (*it_).first);
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*itv_).first, (*it_).first) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*itv_).first, (*it_).first);
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            const_iterator2 &operator = (const const_iterator2 &it) {
                container_const_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const const_iterator2 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_const_subiterator_type itv_;
            const_subiterator_type it_;
        };

        BOOST_UBLAS_INLINE
        const_iterator2 begin2 () const {
            return find2 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        const_iterator2 end2 () const {
            return find2 (0, 0, size2_);
        }

        class iterator2:
            public container_reference<sparse_vector_of_sparse_vector>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               iterator2, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename sparse_vector_of_sparse_vector::value_type value_type;
            typedef typename sparse_vector_of_sparse_vector::difference_type difference_type;
            typedef typename sparse_vector_of_sparse_vector::true_reference reference;
            typedef typename sparse_vector_of_sparse_vector::pointer pointer;

            typedef iterator1 dual_iterator_type;
            typedef reverse_iterator1 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            iterator2 ():
                container_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            iterator2 (self_type &m, int rank, size_type i, size_type j, const vector_subiterator_type &itv, const subiterator_type &it):
                container_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            iterator2 &operator ++ () {
                if (rank_ == 1 && layout_type::fast2 ())
                    ++ it_;
                else {
                    self_type &m = (*this) ();
                    j_ = index2 () + 1;
                    if (rank_ == 1 && ++ itv_ == m.end2 ().itv_)
                        *this = m.find2 (rank_, i_, j_, 1);
                    else if (rank_ == 1) {
                        it_ = (*itv_).second.begin ();
                        if (it_ == (*itv_).second.end () || index1 () != i_)
                            *this = m.find2 (rank_, i_, j_, 1);
                    }
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            iterator2 &operator -- () {
                if (rank_ == 1 && layout_type::fast2 ())
                    -- it_;
                else {
                    self_type &m = (*this) ();
                    j_ = index2 () - 1;
                    if (rank_ == 1 && -- itv_ == m.end2 ().itv_)
                        *this = m.find2 (rank_, i_, j_, -1);
                    else if (rank_ == 1) {
                        it_ = (*itv_).second.begin ();
                        if (it_ == (*itv_).second.end () || index1 () != i_)
                            *this = m.find2 (rank_, i_, j_, -1);
                    }
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*it_).second;
                } else {
                    return (*this) ().at_element (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator1 begin () const {
                self_type &m = (*this) ();
                return m.find1 (1, 0, index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator1 end () const {
                self_type &m = (*this) ();
                return m.find1 (1, m.size1 (), index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator1 rbegin () const {
                return reverse_iterator1 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator1 rend () const {
                return reverse_iterator1 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*itv_).first, (*it_).first) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*itv_).first, (*it_).first);
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*itv_).first, (*it_).first) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*itv_).first, (*it_).first);
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            iterator2 &operator = (const iterator2 &it) {
                container_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const iterator2 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_subiterator_type itv_;
            subiterator_type it_;

            friend class const_iterator2;
        };

        BOOST_UBLAS_INLINE
        iterator2 begin2 () {
            return find2 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        iterator2 end2 () {
            return find2 (0, 0, size2_);
        }

        // Reverse iterators

        BOOST_UBLAS_INLINE
        const_reverse_iterator1 rbegin1 () const {
            return const_reverse_iterator1 (end1 ());
        }
        BOOST_UBLAS_INLINE
        const_reverse_iterator1 rend1 () const {
            return const_reverse_iterator1 (begin1 ());
        }

        BOOST_UBLAS_INLINE
        reverse_iterator1 rbegin1 () {
            return reverse_iterator1 (end1 ());
        }
        BOOST_UBLAS_INLINE
        reverse_iterator1 rend1 () {
            return reverse_iterator1 (begin1 ());
        }

        BOOST_UBLAS_INLINE
        const_reverse_iterator2 rbegin2 () const {
            return const_reverse_iterator2 (end2 ());
        }
        BOOST_UBLAS_INLINE
        const_reverse_iterator2 rend2 () const {
            return const_reverse_iterator2 (begin2 ());
        }

        BOOST_UBLAS_INLINE
        reverse_iterator2 rbegin2 () {
            return reverse_iterator2 (end2 ());
        }
        BOOST_UBLAS_INLINE
        reverse_iterator2 rend2 () {
            return reverse_iterator2 (begin2 ());
        }

    private:
        size_type size1_;
        size_type size2_;
        size_type non_zeros_;
        array_type data_;
        static const value_type zero_;
    };

    template<class T, class L, class A>
    const typename sparse_vector_of_sparse_vector<T, L, A>::value_type sparse_vector_of_sparse_vector<T, L, A>::zero_ (0);


    // Array based sparse matrix class
    // Thanks to Kresimir Fresl for extending this to cover different index bases.
    template<class T, class L, std::size_t IB, class IA, class TA>
    class compressed_matrix:
        public matrix_expression<compressed_matrix<T, L, IB, IA, TA> > {

        typedef T &true_reference;
        typedef T *pointer;
        typedef const T * const_pointer;
        typedef L layout_type;
        typedef compressed_matrix<T, L, IB, IA, TA> self_type;
    public:
#ifndef BOOST_UBLAS_NO_PROXY_SHORTCUTS
        using matrix_expression<self_type>::operator ();
#endif
        // ISSUE require type consistency check for IA TA and IA::value_type
        typedef typename IA::size_type size_type;
        typedef typename IA::difference_type difference_type;
        typedef T value_type;
        typedef const T &const_reference;
#ifndef BOOST_UBLAS_STRICT_MATRIX_SPARSE
        typedef T &reference;
#else
        typedef sparse_matrix_element<self_type> reference;
#endif
        typedef IA index_array_type;
        typedef TA value_array_type;
        typedef const matrix_reference<const self_type> const_closure_type;
        typedef matrix_reference<self_type> closure_type;
        typedef compressed_vector<T, IB, IA, TA> vector_temporary_type;
        typedef self_type matrix_temporary_type;
        typedef sparse_tag storage_category;
        typedef typename L::orientation_category orientation_category;

        // Construction and destruction
        BOOST_UBLAS_INLINE
        compressed_matrix ():
            matrix_expression<self_type> (),
            size1_ (0), size2_ (0), non_zeros_ (max_nz (0)),
            filled1_ (1), filled2_ (0),
            index1_data_ (layout_type::size1 (size1_, size2_) + 1),
            index2_data_ (non_zeros_), value_data_ (non_zeros_) {
            index1_data_ [filled1_ - 1] = k_based (filled2_);
        }
        BOOST_UBLAS_INLINE
        compressed_matrix (size_type size1, size_type size2, size_type non_zeros = 0):
            matrix_expression<self_type> (),
            size1_ (size1), size2_ (size2), non_zeros_ (max_nz (non_zeros)),
            filled1_ (1), filled2_ (0),
            index1_data_ (layout_type::size1 (size1_, size2_) + 1),
            index2_data_ (non_zeros_), value_data_ (non_zeros_) {
            index1_data_ [filled1_ - 1] = k_based (filled2_);
        }
        BOOST_UBLAS_INLINE
        compressed_matrix (const compressed_matrix &m):
            matrix_expression<self_type> (),
            size1_ (m.size1_), size2_ (m.size2_), non_zeros_ (m.non_zeros_),
            filled1_ (m.filled1_), filled2_ (m.filled2_),
            index1_data_ (m.index1_data_),
            index2_data_ (m.index2_data_), value_data_ (m.value_data_) {
            BOOST_UBLAS_CHECK (index1_data_ [filled1_ - 1] == k_based (filled2_), internal_logic ());
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        compressed_matrix (const matrix_expression<AE> &ae, size_type non_zeros = 0):
            matrix_expression<self_type> (),
            size1_ (ae ().size1 ()), size2_ (ae ().size2 ()), non_zeros_ (max_nz (non_zeros)),
            filled1_ (1), filled2_ (0),
            index1_data_ (layout_type::size1 (ae ().size1 (), ae ().size2 ()) + 1),
            index2_data_ (non_zeros_), value_data_ (non_zeros_) {
            index1_data_ [filled1_ - 1] = k_based (filled2_);
            matrix_assign<scalar_assign> (*this, ae);
        }

        // Accessors
        BOOST_UBLAS_INLINE
        size_type size1 () const {
            return size1_;
        }
        BOOST_UBLAS_INLINE
        size_type size2 () const {
            return size2_;
        }
        BOOST_UBLAS_INLINE
        size_type non_zeros () const {
            return non_zeros_;
        }
        BOOST_UBLAS_INLINE
        size_type filled () const {
            return filled2_;
        }
        BOOST_UBLAS_INLINE
        size_type &filled1 () {
            return filled1_;
        }
        BOOST_UBLAS_INLINE
        size_type &filled2 () {
            return filled2_;
        }
        BOOST_UBLAS_INLINE
        static size_type index_base () {
            return IB;
        }
        BOOST_UBLAS_INLINE
        const index_array_type &index1_data () const {
            return index1_data_;
        }
        BOOST_UBLAS_INLINE
        index_array_type &index1_data () {
            return index1_data_;
        }
        BOOST_UBLAS_INLINE
        const index_array_type &index2_data () const {
            return index2_data_;
        }
        BOOST_UBLAS_INLINE
        index_array_type &index2_data () {
            return index2_data_;
        }
        BOOST_UBLAS_INLINE
        const value_array_type &value_data () const {
            return value_data_;
        }
        BOOST_UBLAS_INLINE
        value_array_type &value_data () {
            return value_data_;
        }

        // Resizing
    private:
        BOOST_UBLAS_INLINE
        size_type max_nz (size_type non_zeros) const {
            non_zeros = (std::max) (non_zeros, (std::min) (size1_, size2_));
            // Guarding against overflow.
            // Thanks to Alexei Novakov for the hint.
            // non_zeros_ = (std::min) (non_zeros, size1_ * size2_);
            if (size1_ > 0 && non_zeros / size1_ >= size2_)
                non_zeros = size1_ * size2_;
            return non_zeros;
        }
    public:
        BOOST_UBLAS_INLINE
        void resize (size_type size1, size_type size2, bool preserve = true) {
            // FIXME preserve unimplemented
            BOOST_UBLAS_CHECK (!preserve, internal_logic ());
            size1_ = size1;
            size2_ = size2;
            non_zeros_ = max_nz (non_zeros_);
            filled1_ = 1;
            filled2_ = 0;
            index1_data ().resize (layout_type::size1 (size1_, size2_) + 1);
            index2_data ().resize (non_zeros_);
            value_data ().resize (non_zeros_);
            index1_data_ [filled1_ - 1] = k_based (filled2_);
        }

        // Reserving
        BOOST_UBLAS_INLINE
        void reserve (size_type non_zeros, bool preserve = true) {
            non_zeros_ = max_nz (non_zeros);
            if (preserve) {
                index2_data ().resize (non_zeros_, size_type ());
                value_data ().resize (non_zeros_, value_type ());
                filled1_ = (std::min) (non_zeros_ + 1, filled1_);
                filled2_ = (std::min) (non_zeros_, filled2_);
            }
            else {
                index2_data ().resize (non_zeros_);
                value_data ().resize (non_zeros_);
                filled1_ = 1;
                filled2_ = 0;
            }
            BOOST_UBLAS_CHECK (index1_data () [filled1_ - 1] == k_based (filled2_), internal_logic ());
       }

        // Element support
        BOOST_UBLAS_INLINE
        pointer find_element (size_type i, size_type j) {
            return const_cast<pointer> (const_cast<const self_type&>(*this).find_element (i, j));
        }
        BOOST_UBLAS_INLINE
        const_pointer find_element (size_type i, size_type j) const {
            size_type element1 (layout_type::element1 (i, size1_, j, size2_));
            size_type element2 (layout_type::element2 (i, size1_, j, size2_));
            if (filled1_ <= element1 + 1)
                return 0;
            vector_const_subiterator_type itv (index1_data ().begin () + element1);
            const_subiterator_type it_begin (index2_data ().begin () + zero_based (*itv));
            const_subiterator_type it_end (index2_data ().begin () + zero_based (*(itv + 1)));
            const_subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (element2), std::less<size_type> ()));
            if (it == it_end || *it != k_based (element2))
                return 0;
            return &value_data () [it - index2_data ().begin ()];
        }

        // Element access
        BOOST_UBLAS_INLINE
        const_reference operator () (size_type i, size_type j) const {
            const_pointer p = find_element (i, j);
            if (p)
                return *p;
            else
                return zero_;
        }
        BOOST_UBLAS_INLINE
        true_reference at_element (size_type i, size_type j) {
            pointer p = find_element (i, j);
            BOOST_UBLAS_CHECK (p, bad_index ());
            return *p;
        }
        BOOST_UBLAS_INLINE
        reference operator () (size_type i, size_type j) {
#ifndef BOOST_UBLAS_STRICT_MATRIX_SPARSE
            // FIXME more new element handler to operator ()
            size_type element1 (layout_type::element1 (i, size1_, j, size2_));
            size_type element2 (layout_type::element2 (i, size1_, j, size2_));
            if (filled1_ <= element1 + 1)
                return set_element (i, j, value_type (0));
            pointer p = find_element (i, j);
            if (p)
                return *p;
            else
                return set_element (i, j, value_type (0));
#else
            return reference (*this, i, j);
#endif
        }

        // Element assignment
        BOOST_UBLAS_INLINE
        true_reference set_element (size_type i, size_type j, const_reference t) {
            BOOST_UBLAS_CHECK (index1_data () [filled1_ - 1] == k_based (filled2_), internal_logic ());
            if (filled2_ >= non_zeros_)
                reserve (2 * non_zeros_, true);
            size_type element1 = layout_type::element1 (i, size1_, j, size2_);
            size_type element2 = layout_type::element2 (i, size1_, j, size2_);
            while (filled1_ < element1 + 2) {
                index1_data () [filled1_] = k_based (filled2_);
                ++ filled1_;
            }
            vector_subiterator_type itv (index1_data ().begin () + element1);
            subiterator_type it_begin (index2_data ().begin () + zero_based (*itv));
            subiterator_type it_end (index2_data ().begin () + zero_based (*(itv + 1)));
            subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (element2), std::less<size_type> ()));
            difference_type n = it - index2_data ().begin ();
            BOOST_UBLAS_CHECK (it == it_end || *it != k_based (element2), external_logic ());
            ++ filled2_;
            it = index2_data ().begin () + n;
            std::copy_backward (it, index2_data ().begin () + filled2_ - 1, index2_data ().begin () + filled2_);
            *it = k_based (element2);
            typename value_array_type::iterator itt (value_data ().begin () + n);
            std::copy_backward (itt, value_data ().begin () + filled2_ - 1, value_data ().begin () + filled2_);
            *itt = t;
            while (element1 + 1 < filled1_) {
                ++ index1_data () [element1 + 1];
                ++ element1;
            }
            BOOST_UBLAS_CHECK (index1_data () [filled1_ - 1] == k_based (filled2_), internal_logic ());
            return *itt;
        }
        BOOST_UBLAS_INLINE
        void zero_element (size_type i, size_type j) {
            BOOST_UBLAS_CHECK (index1_data () [filled1_ - 1] == k_based (filled2_), internal_logic ());
            size_type element1 = layout_type::element1 (i, size1_, j, size2_);
            size_type element2 = layout_type::element2 (i, size1_, j, size2_);
            if (element1 + 1 > filled1_)
                return;
            vector_subiterator_type itv (index1_data ().begin () + element1);
            subiterator_type it_begin (index2_data ().begin () + zero_based (*itv));
            subiterator_type it_end (index2_data ().begin () + zero_based (*(itv + 1)));
            subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (element2), std::less<size_type> ()));
            if (it != it_end && *it == k_based (element2)) {
                difference_type n = it - index2_data ().begin ();
                std::copy (it + 1, index2_data ().begin () + filled2_, it);
                typename value_array_type::iterator itt (value_data ().begin () + n);
                std::copy (itt + 1, value_data ().begin () + filled2_, itt);
                -- filled2_;
                while (index1_data () [filled1_ - 2] > k_based (filled2_)) {
                    index1_data () [filled1_ - 1] = 0;
                    -- filled1_;
                }
                while (element1 + 1 < filled1_) {
                    -- index1_data () [element1 + 1];
                    ++ element1;
                }
            }
            BOOST_UBLAS_CHECK (index1_data () [filled1_ - 1] == k_based (filled2_), internal_logic ());
        }
        
        // Zeroing
        BOOST_UBLAS_INLINE
        void zero () {
            filled1_ = 1;
            filled2_ = 0;
            index1_data_ [filled1_ - 1] = k_based (filled2_);
        }

        // Assignment
        BOOST_UBLAS_INLINE
        compressed_matrix &operator = (const compressed_matrix &m) {
            if (this != &m) {
                size1_ = m.size1_;
                size2_ = m.size2_;
                non_zeros_ = m.non_zeros_;
                filled1_ = m.filled1_;
                filled2_ = m.filled2_;
                index1_data () = m.index1_data ();
                index2_data () = m.index2_data ();
                value_data () = m.value_data ();
                BOOST_UBLAS_CHECK (layout_type::size1 (size1_, size2_) + 1 == index1_data ().size (), internal_logic ());
                BOOST_UBLAS_CHECK (non_zeros_ == index2_data ().size (), internal_logic ());
                BOOST_UBLAS_CHECK (non_zeros_ == value_data ().size (), internal_logic ());
            }
            return *this;
        }
        BOOST_UBLAS_INLINE
        compressed_matrix &assign_temporary (compressed_matrix &m) {
            swap (m);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        compressed_matrix &operator = (const matrix_expression<AE> &ae) {
            self_type temporary (ae, non_zeros_);
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        compressed_matrix &assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_assign> (*this, ae);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        compressed_matrix& operator += (const matrix_expression<AE> &ae) {
            self_type temporary (*this + ae, non_zeros_);
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        compressed_matrix &plus_assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_plus_assign> (*this, ae);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        compressed_matrix& operator -= (const matrix_expression<AE> &ae) {
            self_type temporary (*this - ae, non_zeros_);
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        compressed_matrix &minus_assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_minus_assign> (*this, ae);
            return *this;
        }
        template<class AT>
        BOOST_UBLAS_INLINE
        compressed_matrix& operator *= (const AT &at) {
            matrix_assign_scalar<scalar_multiplies_assign> (*this, at);
            return *this;
        }
        template<class AT>
        BOOST_UBLAS_INLINE
        compressed_matrix& operator /= (const AT &at) {
            matrix_assign_scalar<scalar_divides_assign> (*this, at);
            return *this;
        }

        // Swapping
        BOOST_UBLAS_INLINE
        void swap (compressed_matrix &m) {
            if (this != &m) {
                std::swap (size1_, m.size1_);
                std::swap (size2_, m.size2_);
                std::swap (non_zeros_, m.non_zeros_);
                std::swap (filled1_, m.filled1_);
                std::swap (filled2_, m.filled2_);
                index1_data ().swap (m.index1_data ());
                index2_data ().swap (m.index2_data ());
                value_data ().swap (m.value_data ());
            }
        }
        BOOST_UBLAS_INLINE
        friend void swap (compressed_matrix &m1, compressed_matrix &m2) {
            m1.swap (m2);
        }

        // Back element insertion and erasure
        BOOST_UBLAS_INLINE
        void push_back (size_type i, size_type j, const_reference t) {
            BOOST_UBLAS_CHECK (index1_data () [filled1_ - 1] == k_based (filled2_), internal_logic ());
            if (filled2_ >= non_zeros_)
                reserve (2 * non_zeros_, true);
            size_type element1 = layout_type::element1 (i, size1_, j, size2_);
            size_type element2 = layout_type::element2 (i, size1_, j, size2_);
            while (filled1_ < element1 + 2) {
                index1_data () [filled1_] = k_based (filled2_);
                ++ filled1_;
            }
            if (filled1_ == element1 + 2 &&
                (filled2_ == zero_based (index1_data () [filled1_ - 2]) ||
                 index2_data () [filled2_ - 1] < k_based (element2))) {
                ++ filled2_;
                index1_data () [filled1_ - 1] = k_based (filled2_);
                index2_data () [filled2_ - 1] = k_based (element2);
                value_data () [filled2_ - 1] = t;
                BOOST_UBLAS_CHECK (index1_data () [filled1_ - 1] == k_based (filled2_), internal_logic ());
                return;
            }
            external_logic ().raise ();
        }
        BOOST_UBLAS_INLINE
        void pop_back () {
            BOOST_UBLAS_CHECK (filled1_ > 0 && filled2_ > 0, external_logic ());
            BOOST_UBLAS_CHECK (index1_data () [filled1_ - 1] == k_based (filled2_), internal_logic ());
            -- filled2_;
            while (index1_data () [filled1_ - 2] > k_based (filled2_)) {
                index1_data () [filled1_ - 1] = 0;
                -- filled1_;
            }
            -- index1_data () [filled1_ - 1];
            BOOST_UBLAS_CHECK (index1_data () [filled1_ - 1] == k_based (filled2_), internal_logic ());
        }

        // Iterator types
    private:
        // Use index array iterator
        typedef typename IA::const_iterator vector_const_subiterator_type;
        typedef typename IA::iterator vector_subiterator_type;
        typedef typename IA::const_iterator const_subiterator_type;
        typedef typename IA::iterator subiterator_type;

    public:
        class const_iterator1;
        class iterator1;
        class const_iterator2;
        class iterator2;
        typedef reverse_iterator_base1<const_iterator1> const_reverse_iterator1;
        typedef reverse_iterator_base1<iterator1> reverse_iterator1;
        typedef reverse_iterator_base2<const_iterator2> const_reverse_iterator2;
        typedef reverse_iterator_base2<iterator2> reverse_iterator2;

        // Element lookup
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        const_iterator1 find1 (int rank, size_type i, size_type j, int direction = 1) const {
            for (;;) {
                size_type address1 (layout_type::address1 (i, size1_, j, size2_));
                size_type address2 (layout_type::address2 (i, size1_, j, size2_));
                vector_const_subiterator_type itv (index1_data ().begin () + (std::min) (filled1_ - 1, address1));
                if (filled1_ <= address1 + 1)
                    return const_iterator1 (*this, rank, i, j, itv, index2_data ().begin () + filled2_);

                const_subiterator_type it_begin (index2_data ().begin () + zero_based (*itv));
                const_subiterator_type it_end (index2_data ().begin () + zero_based (*(itv + 1)));

                const_subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (address2), std::less<size_type> ()));
                if (rank == 0)
                    return const_iterator1 (*this, rank, i, j, itv, it);
                if (it != it_end && zero_based (*it) == address2)
                    return const_iterator1 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast1 ()) {
                        if (it == it_end)
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        i = zero_based (*it);
                    } else {
                        if (i >= size1_)
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        ++ i;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast1 ()) {
                        if (it == index2_data ().begin () + zero_based (*itv))
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        i = zero_based (*(it - 1));
                    } else {
                        if (i == 0)
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        -- i;
                    }
                }
            }
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        iterator1 find1 (int rank, size_type i, size_type j, int direction = 1) {
            for (;;) {
                size_type address1 (layout_type::address1 (i, size1_, j, size2_));
                size_type address2 (layout_type::address2 (i, size1_, j, size2_));
                vector_subiterator_type itv (index1_data ().begin () + (std::min) (filled1_ - 1, address1));
                if (filled1_ <= address1 + 1)
                    return iterator1 (*this, rank, i, j, itv, index2_data ().begin () + filled2_);

                subiterator_type it_begin (index2_data ().begin () + zero_based (*itv));
                subiterator_type it_end (index2_data ().begin () + zero_based (*(itv + 1)));

                subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (address2), std::less<size_type> ()));
                if (rank == 0)
                    return iterator1 (*this, rank, i, j, itv, it);
                if (it != it_end && zero_based (*it) == address2)
                    return iterator1 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast1 ()) {
                        if (it == it_end)
                            return iterator1 (*this, rank, i, j, itv, it);
                        i = zero_based (*it);
                    } else {
                        if (i >= size1_)
                            return iterator1 (*this, rank, i, j, itv, it);
                        ++ i;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast1 ()) {
                        if (it == index2_data ().begin () + zero_based (*itv))
                            return iterator1 (*this, rank, i, j, itv, it);
                        i = zero_based (*(it - 1));
                    } else {
                        if (i == 0)
                            return iterator1 (*this, rank, i, j, itv, it);
                        -- i;
                    }
                }
            }
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        const_iterator2 find2 (int rank, size_type i, size_type j, int direction = 1) const {
            for (;;) {
                size_type address1 (layout_type::address1 (i, size1_, j, size2_));
                size_type address2 (layout_type::address2 (i, size1_, j, size2_));
                vector_const_subiterator_type itv (index1_data ().begin () + (std::min) (filled1_ - 1, address1));
                if (filled1_ <= address1 + 1)
                    return const_iterator2 (*this, rank, i, j, itv, index2_data ().begin () + filled2_);

                const_subiterator_type it_begin (index2_data ().begin () + zero_based (*itv));
                const_subiterator_type it_end (index2_data ().begin () + zero_based (*(itv + 1)));

                const_subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (address2), std::less<size_type> ()));
                if (rank == 0)
                    return const_iterator2 (*this, rank, i, j, itv, it);
                if (it != it_end && zero_based (*it) == address2)
                    return const_iterator2 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast2 ()) {
                        if (it == it_end)
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        j = zero_based (*it);
                    } else {
                        if (j >= size2_)
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        ++ j;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast2 ()) {
                        if (it == index2_data ().begin () + zero_based (*itv))
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        j = zero_based (*(it - 1));
                    } else {
                        if (j == 0)
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        -- j;
                    }
                }
            }
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        iterator2 find2 (int rank, size_type i, size_type j, int direction = 1) {
            for (;;) {
                size_type address1 (layout_type::address1 (i, size1_, j, size2_));
                size_type address2 (layout_type::address2 (i, size1_, j, size2_));
                vector_subiterator_type itv (index1_data ().begin () + (std::min) (filled1_ - 1, address1));
                if (filled1_ <= address1 + 1)
                    return iterator2 (*this, rank, i, j, itv, index2_data ().begin () + filled2_);

                subiterator_type it_begin (index2_data ().begin () + zero_based (*itv));
                subiterator_type it_end (index2_data ().begin () + zero_based (*(itv + 1)));

                subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (address2), std::less<size_type> ()));
                if (rank == 0)
                    return iterator2 (*this, rank, i, j, itv, it);
                if (it != it_end && zero_based (*it) == address2)
                    return iterator2 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast2 ()) {
                        if (it == it_end)
                            return iterator2 (*this, rank, i, j, itv, it);
                        j = zero_based (*it);
                    } else {
                        if (j >= size2_)
                            return iterator2 (*this, rank, i, j, itv, it);
                        ++ j;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast2 ()) {
                        if (it == index2_data ().begin () + zero_based (*itv))
                            return iterator2 (*this, rank, i, j, itv, it);
                        j = zero_based (*(it - 1));
                    } else {
                        if (j == 0)
                            return iterator2 (*this, rank, i, j, itv, it);
                        -- j;
                    }
                }
            }
        }


        class const_iterator1:
            public container_const_reference<compressed_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               const_iterator1, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename compressed_matrix::value_type value_type;
            typedef typename compressed_matrix::difference_type difference_type;
            typedef typename compressed_matrix::const_reference reference;
            typedef const typename compressed_matrix::pointer pointer;

            typedef const_iterator2 dual_iterator_type;
            typedef const_reverse_iterator2 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            const_iterator1 ():
                container_const_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            const_iterator1 (const self_type &m, int rank, size_type i, size_type j, const vector_const_subiterator_type &itv, const const_subiterator_type &it):
                container_const_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}
            BOOST_UBLAS_INLINE
            const_iterator1 (const iterator1 &it):
                container_const_reference<self_type> (it ()), rank_ (it.rank_), i_ (it.i_), j_ (it.j_), itv_ (it.itv_), it_ (it.it_) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            const_iterator1 &operator ++ () {
                if (rank_ == 1 && layout_type::fast1 ())
                    ++ it_;
                else {
                    i_ = index1 () + 1;
                    if (rank_ == 1)
                        *this = (*this) ().find1 (rank_, i_, j_, 1);
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            const_iterator1 &operator -- () {
                if (rank_ == 1 && layout_type::fast1 ())
                    -- it_;
                else {
                    i_ = index1 () - 1;
                    if (rank_ == 1)
                        *this = (*this) ().find1 (rank_, i_, j_, -1);
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            const_reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*this) ().value_data () [it_ - (*this) ().index2_data ().begin ()];
                } else {
                    return (*this) () (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator2 begin () const {
                const self_type &m = (*this) ();
                return m.find2 (1, index1 (), 0);
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator2 end () const {
                const self_type &m = (*this) ();
                return m.find2 (1, index1 (), m.size2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator2 rbegin () const {
                return const_reverse_iterator2 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator2 rend () const {
                return const_reverse_iterator2 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_)) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_));
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_)) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_));
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            const_iterator1 &operator = (const const_iterator1 &it) {
                container_const_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const const_iterator1 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_const_subiterator_type itv_;
            const_subiterator_type it_;
        };

        BOOST_UBLAS_INLINE
        const_iterator1 begin1 () const {
            return find1 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        const_iterator1 end1 () const {
            return find1 (0, size1_, 0);
        }

        class iterator1:
            public container_reference<compressed_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               iterator1, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename compressed_matrix::value_type value_type;
            typedef typename compressed_matrix::difference_type difference_type;
            typedef typename compressed_matrix::true_reference reference;
            typedef typename compressed_matrix::pointer pointer;

            typedef iterator2 dual_iterator_type;
            typedef reverse_iterator2 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            iterator1 ():
                container_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            iterator1 (self_type &m, int rank, size_type i, size_type j, const vector_subiterator_type &itv, const subiterator_type &it):
                container_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            iterator1 &operator ++ () {
                if (rank_ == 1 && layout_type::fast1 ())
                    ++ it_;
                else {
                    i_ = index1 () + 1;
                    if (rank_ == 1)
                        *this = (*this) ().find1 (rank_, i_, j_, 1);
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            iterator1 &operator -- () {
                if (rank_ == 1 && layout_type::fast1 ())
                    -- it_;
                else {
                    i_ = index1 () - 1;
                    if (rank_ == 1)
                        *this = (*this) ().find1 (rank_, i_, j_, -1);
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*this) ().value_data () [it_ - (*this) ().index2_data ().begin ()];
                } else {
                    return (*this) ().at_element (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator2 begin () const {
                self_type &m = (*this) ();
                return m.find2 (1, index1 (), 0);
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator2 end () const {
                self_type &m = (*this) ();
                return m.find2 (1, index1 (), m.size2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator2 rbegin () const {
                return reverse_iterator2 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator2 rend () const {
                return reverse_iterator2 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_)) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_));
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_)) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_));
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            iterator1 &operator = (const iterator1 &it) {
                container_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const iterator1 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_subiterator_type itv_;
            subiterator_type it_;

            friend class const_iterator1;
        };

        BOOST_UBLAS_INLINE
        iterator1 begin1 () {
            return find1 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        iterator1 end1 () {
            return find1 (0, size1_, 0);
        }

        class const_iterator2:
            public container_const_reference<compressed_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               const_iterator2, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename compressed_matrix::value_type value_type;
            typedef typename compressed_matrix::difference_type difference_type;
            typedef typename compressed_matrix::const_reference reference;
            typedef const typename compressed_matrix::pointer pointer;

            typedef const_iterator1 dual_iterator_type;
            typedef const_reverse_iterator1 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            const_iterator2 ():
                container_const_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            const_iterator2 (const self_type &m, int rank, size_type i, size_type j, const vector_const_subiterator_type itv, const const_subiterator_type &it):
                container_const_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}
            BOOST_UBLAS_INLINE
            const_iterator2 (const iterator2 &it):
                container_const_reference<self_type> (it ()), rank_ (it.rank_), i_ (it.i_), j_ (it.j_), itv_ (it.itv_), it_ (it.it_) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            const_iterator2 &operator ++ () {
                if (rank_ == 1 && layout_type::fast2 ())
                    ++ it_;
                else {
                    j_ = index2 () + 1;
                    if (rank_ == 1)
                        *this = (*this) ().find2 (rank_, i_, j_, 1);
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            const_iterator2 &operator -- () {
                if (rank_ == 1 && layout_type::fast2 ())
                    -- it_;
                else {
                    j_ = index2 () - 1;
                    if (rank_ == 1)
                        *this = (*this) ().find2 (rank_, i_, j_, -1);
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            const_reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*this) ().value_data () [it_ - (*this) ().index2_data ().begin ()];
                } else {
                    return (*this) () (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator1 begin () const {
                const self_type &m = (*this) ();
                return m.find1 (1, 0, index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator1 end () const {
                const self_type &m = (*this) ();
                return m.find1 (1, m.size1 (), index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator1 rbegin () const {
                return const_reverse_iterator1 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator1 rend () const {
                return const_reverse_iterator1 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_)) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_));
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_)) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_));
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            const_iterator2 &operator = (const const_iterator2 &it) {
                container_const_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const const_iterator2 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_const_subiterator_type itv_;
            const_subiterator_type it_;
        };

        BOOST_UBLAS_INLINE
        const_iterator2 begin2 () const {
            return find2 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        const_iterator2 end2 () const {
            return find2 (0, 0, size2_);
        }

        class iterator2:
            public container_reference<compressed_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               iterator2, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename compressed_matrix::value_type value_type;
            typedef typename compressed_matrix::difference_type difference_type;
            typedef typename compressed_matrix::true_reference reference;
            typedef typename compressed_matrix::pointer pointer;

            typedef iterator1 dual_iterator_type;
            typedef reverse_iterator1 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            iterator2 ():
                container_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            iterator2 (self_type &m, int rank, size_type i, size_type j, const vector_subiterator_type &itv, const subiterator_type &it):
                container_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            iterator2 &operator ++ () {
                if (rank_ == 1 && layout_type::fast2 ())
                    ++ it_;
                else {
                    j_ = index2 () + 1;
                    if (rank_ == 1)
                        *this = (*this) ().find2 (rank_, i_, j_, 1);
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            iterator2 &operator -- () {
                if (rank_ == 1 && layout_type::fast2 ())
                    -- it_;
                else {
                    j_ = index2 ();
                    if (rank_ == 1)
                        *this = (*this) ().find2 (rank_, i_, j_, -1);
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*this) ().value_data () [it_ - (*this) ().index2_data ().begin ()];
                } else {
                    return (*this) ().at_element (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator1 begin () const {
                self_type &m = (*this) ();
                return m.find1 (1, 0, index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator1 end () const {
                self_type &m = (*this) ();
                return m.find1 (1, m.size1 (), index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator1 rbegin () const {
                return reverse_iterator1 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator1 rend () const {
                return reverse_iterator1 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_)) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_));
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_)) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 (itv_ - (*this) ().index1_data ().begin (), (*this) ().zero_based (*it_));
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            iterator2 &operator = (const iterator2 &it) {
                container_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const iterator2 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_subiterator_type itv_;
            subiterator_type it_;

            friend class const_iterator2;
        };

        BOOST_UBLAS_INLINE
        iterator2 begin2 () {
            return find2 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        iterator2 end2 () {
            return find2 (0, 0, size2_);
        }

        // Reverse iterators

        BOOST_UBLAS_INLINE
        const_reverse_iterator1 rbegin1 () const {
            return const_reverse_iterator1 (end1 ());
        }
        BOOST_UBLAS_INLINE
        const_reverse_iterator1 rend1 () const {
            return const_reverse_iterator1 (begin1 ());
        }

        BOOST_UBLAS_INLINE
        reverse_iterator1 rbegin1 () {
            return reverse_iterator1 (end1 ());
        }
        BOOST_UBLAS_INLINE
        reverse_iterator1 rend1 () {
            return reverse_iterator1 (begin1 ());
        }

        BOOST_UBLAS_INLINE
        const_reverse_iterator2 rbegin2 () const {
            return const_reverse_iterator2 (end2 ());
        }
        BOOST_UBLAS_INLINE
        const_reverse_iterator2 rend2 () const {
            return const_reverse_iterator2 (begin2 ());
        }

        BOOST_UBLAS_INLINE
        reverse_iterator2 rbegin2 () {
            return reverse_iterator2 (end2 ());
        }
        BOOST_UBLAS_INLINE
        reverse_iterator2 rend2 () {
            return reverse_iterator2 (begin2 ());
        }

    private:
        size_type size1_;
        size_type size2_;
        size_type non_zeros_;
        size_type filled1_;
        size_type filled2_;
        index_array_type index1_data_;
        index_array_type index2_data_;
        value_array_type value_data_;
        static const value_type zero_;

        BOOST_UBLAS_INLINE
        static size_type zero_based (size_type k_based_index) {
            return k_based_index - IB;
        }
        BOOST_UBLAS_INLINE
        static size_type k_based (size_type zero_based_index) {
            return zero_based_index + IB;
        }

        friend class iterator1;
        friend class iterator2;
        friend class const_iterator1;
        friend class const_iterator2;
    };

    template<class T, class L, std::size_t IB, class IA, class TA>
    const typename compressed_matrix<T, L, IB, IA, TA>::value_type compressed_matrix<T, L, IB, IA, TA>::zero_ (0);


    // Array based sparse matrix class
    // Thanks to Kresimir Fresl for extending this to cover different index bases.
    template<class T, class L, std::size_t IB, class IA, class TA>
    class coordinate_matrix:
        public matrix_expression<coordinate_matrix<T, L, IB, IA, TA> > {

        typedef T &true_reference;
        typedef T *pointer;
        typedef const T * const_pointer;
        typedef L layout_type;
        typedef coordinate_matrix<T, L, IB, IA, TA> self_type;
    public:
#ifndef BOOST_UBLAS_NO_PROXY_SHORTCUTS
        using matrix_expression<self_type>::operator ();
#endif
        // ISSUE require type consistency check for IA TA and IA::value_type
        typedef typename IA::size_type size_type;
        typedef typename IA::difference_type difference_type;
        typedef T value_type;
        typedef const T &const_reference;
#ifndef BOOST_UBLAS_STRICT_MATRIX_SPARSE
        typedef T &reference;
#else
        typedef sparse_matrix_element<self_type> reference;
#endif
        typedef IA index_array_type;
        typedef TA value_array_type;
        typedef const matrix_reference<const self_type> const_closure_type;
        typedef matrix_reference<self_type> closure_type;
        typedef coordinate_vector<T, IB, IA, TA> vector_temporary_type;
        typedef self_type matrix_temporary_type;
        typedef sparse_tag storage_category;
        typedef typename L::orientation_category orientation_category;

        // Construction and destruction
        BOOST_UBLAS_INLINE
        coordinate_matrix ():
            matrix_expression<self_type> (),
            size1_ (0), size2_ (0), non_zeros_ (max_nz (0)),
            filled_ (0),
            sorted_ (true), index1_data_ (non_zeros_),
            index2_data_ (non_zeros_), value_data_ (non_zeros_) {}
        BOOST_UBLAS_INLINE
        coordinate_matrix (size_type size1, size_type size2, size_type non_zeros = 0):
            matrix_expression<self_type> (),
            size1_ (size1), size2_ (size2), non_zeros_ (max_nz (non_zeros)),
            filled_ (0),
            sorted_ (true), index1_data_ (non_zeros_),
            index2_data_ (non_zeros_), value_data_ (non_zeros_) {
        }
        BOOST_UBLAS_INLINE
        coordinate_matrix (const coordinate_matrix &m):
            matrix_expression<self_type> (),
            size1_ (m.size1_), size2_ (m.size2_), non_zeros_ (m.non_zeros_),
            filled_ (m.filled_),
            sorted_ (m.sorted_), index1_data_ (m.index1_data_),
            index2_data_ (m.index2_data_), value_data_ (m.value_data_) {
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        coordinate_matrix (const matrix_expression<AE> &ae, size_type non_zeros = 0):
            matrix_expression<self_type> (),
            size1_ (ae ().size1 ()), size2_ (ae ().size2 ()), non_zeros_ (max_nz (non_zeros)),
            filled_ (0),
            sorted_ (true), index1_data_ (non_zeros_),
            index2_data_ (non_zeros_), value_data_ (non_zeros_) {
            matrix_assign<scalar_assign> (*this, ae);
        }

        // Accessors
        BOOST_UBLAS_INLINE
        size_type size1 () const {
            return size1_;
        }
        BOOST_UBLAS_INLINE
        size_type size2 () const {
            return size2_;
        }
        BOOST_UBLAS_INLINE
        size_type non_zeros () const {
            return non_zeros_;
        }
        BOOST_UBLAS_INLINE
        size_type filled () const {
            return filled_;
        }
        BOOST_UBLAS_INLINE
        static size_type index_base () {
            return IB;
        }
        BOOST_UBLAS_INLINE
        const index_array_type &index1_data () const {
            return index1_data_;
        }
        BOOST_UBLAS_INLINE
        index_array_type &index1_data () {
            return index1_data_;
        }
        BOOST_UBLAS_INLINE
        const index_array_type &index2_data () const {
            return index2_data_;
        }
        BOOST_UBLAS_INLINE
        index_array_type &index2_data () {
            return index2_data_;
        }
        BOOST_UBLAS_INLINE
        const value_array_type &value_data () const {
            return value_data_;
        }
        BOOST_UBLAS_INLINE
        value_array_type &value_data () {
            return value_data_;
        }

        // Resizing
    private:
        BOOST_UBLAS_INLINE
        size_type max_nz (size_type non_zeros) const {
            non_zeros = (std::max) (non_zeros, (std::min) (size1_, size2_));
            // Guarding against overflow.
            // Thanks to Alexei Novakov for the hint.
            // non_zeros_ = (std::min) (non_zeros, size1_ * size2_);
            if (size1_ > 0 && non_zeros / size1_ >= size2_)
                non_zeros = size1_ * size2_;
            return non_zeros;
        }
    public:
        BOOST_UBLAS_INLINE
        void resize (size_type size1, size_type size2, bool preserve = true) {
            // FIXME preserve unimplemented
            BOOST_UBLAS_CHECK (!preserve, internal_logic ());
            size1_ = size1;
            size2_ = size2;
            non_zeros_ = max_nz (non_zeros_);
            index1_data ().resize (non_zeros_);
            index2_data ().resize (non_zeros_);
            value_data ().resize (non_zeros_);
            filled_ = 0;
        }

        // Reserving
        BOOST_UBLAS_INLINE
        void reserve (size_type non_zeros, bool preserve = true) {
            sort ();    // remove duplicate elements
            non_zeros_ = max_nz (non_zeros);
            if (preserve) {
                index1_data ().resize (non_zeros_, size_type ());
                index2_data ().resize (non_zeros_, size_type ());
                value_data ().resize (non_zeros_, value_type ());
                filled_ = (std::min) (non_zeros_, filled_);
            }
            else {
                index1_data ().resize (non_zeros_);
                index2_data ().resize (non_zeros_);
                value_data ().resize (non_zeros_);
                filled_ = 0;
            }
        }

        // Element support
        BOOST_UBLAS_INLINE
        pointer find_element (size_type i, size_type j) {
            return const_cast<pointer> (const_cast<const self_type&>(*this).find_element (i, j));
        }
        BOOST_UBLAS_INLINE
        const_pointer find_element (size_type i, size_type j) const {
            sort ();
            size_type element1 (layout_type::element1 (i, size1_, j, size2_));
            size_type element2 (layout_type::element2 (i, size1_, j, size2_));
            vector_const_subiterator_type itv_begin (detail::lower_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (element1), std::less<size_type> ()));
            vector_const_subiterator_type itv_end (detail::upper_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (element1), std::less<size_type> ()));
            if (itv_begin == itv_end)
                return 0;
            const_subiterator_type it_begin (index2_data ().begin () + (itv_begin - index1_data ().begin ()));
            const_subiterator_type it_end (index2_data ().begin () + (itv_end - index1_data ().begin ()));
            const_subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (element2), std::less<size_type> ()));
            if (it == it_end || *it != k_based (element2))
                return 0;
            return &value_data () [it - index2_data ().begin ()];
        }

        // Element access
        BOOST_UBLAS_INLINE
        const_reference operator () (size_type i, size_type j) const {
            const_pointer p = find_element (i, j);
            if (p)
                return *p;
            else 
                return zero_;
        }
        BOOST_UBLAS_INLINE
        true_reference at_element (size_type i, size_type j) {
            pointer p = find_element (i, j);
            BOOST_UBLAS_CHECK (p, bad_index ());
            return *p;
        }
        BOOST_UBLAS_INLINE
        reference operator () (size_type i, size_type j) {
#ifndef BOOST_UBLAS_STRICT_MATRIX_SPARSE
            pointer p = find_element (i, j);
            if (p)
                return *p;
            else
                return set_element (i, j, value_type (0));
#else
            return reference (*this, i, j);
#endif
        }

        // Element assignment
        BOOST_UBLAS_INLINE
        true_reference set_element (size_type i, size_type j, const_reference t) {
            if (filled_ >= non_zeros_)
                reserve (2 * non_zeros_, true);
            size_type element1 = layout_type::element1 (i, size1_, j, size2_);
            size_type element2 = layout_type::element2 (i, size1_, j, size2_);
            ++ filled_;
            index1_data () [filled_ - 1] = k_based (element1);
            index2_data () [filled_ - 1] = k_based (element2);
            value_data () [filled_ - 1] = t;
            sorted_ = false;
            return value_data () [filled_ - 1];
        }
        BOOST_UBLAS_INLINE
        void zero_element (size_type i, size_type j) {
            size_type element1 = layout_type::element1 (i, size1_, j, size2_);
            size_type element2 = layout_type::element2 (i, size1_, j, size2_);
            sort ();
            vector_subiterator_type itv_begin (detail::lower_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (element1), std::less<size_type> ()));
            vector_subiterator_type itv_end (detail::upper_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (element1), std::less<size_type> ()));
            subiterator_type it_begin (index2_data ().begin () + (itv_begin - index1_data ().begin ()));
            subiterator_type it_end (index2_data ().begin () + (itv_end - index1_data ().begin ()));
            subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (element2), std::less<size_type> ()));
            if (it != it_end && *it == k_based (element2)) {
                difference_type n = it - index2_data ().begin ();
                vector_subiterator_type itv (index1_data ().begin () + n);
                std::copy (itv + 1, index1_data ().begin () + filled_, itv);
                std::copy (it + 1, index2_data ().begin () + filled_, it);
                typename value_array_type::iterator itt (value_data ().begin () + n);
                std::copy (itt + 1, value_data ().begin () + filled_, itt);
                -- filled_;
            }
        }
        
        // Zeroing
        BOOST_UBLAS_INLINE
        void zero () {
            filled_ = 0;
        }

        // Assignment
        BOOST_UBLAS_INLINE
        coordinate_matrix &operator = (const coordinate_matrix &m) {
            if (this != &m) {
                size1_ = m.size1_;
                size2_ = m.size2_;
                non_zeros_ = m.non_zeros_;
                filled_ = m.filled_;
                sorted_ = m.sorted_;
                index1_data () = m.index1_data ();
                index2_data () = m.index2_data ();
                value_data () = m.value_data ();
                BOOST_UBLAS_CHECK (non_zeros_ == index1_data ().size (), internal_logic ());
                BOOST_UBLAS_CHECK (non_zeros_ == index2_data ().size (), internal_logic ());
                BOOST_UBLAS_CHECK (non_zeros_ == value_data ().size (), internal_logic ());
            }
            return *this;
        }
        BOOST_UBLAS_INLINE
        coordinate_matrix &assign_temporary (coordinate_matrix &m) {
            swap (m);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        coordinate_matrix &operator = (const matrix_expression<AE> &ae) {
            self_type temporary (ae, non_zeros_);
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        coordinate_matrix &assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_assign> (*this, ae);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        coordinate_matrix& operator += (const matrix_expression<AE> &ae) {
            self_type temporary (*this + ae, non_zeros_);
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        coordinate_matrix &plus_assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_plus_assign> (*this, ae);
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        coordinate_matrix& operator -= (const matrix_expression<AE> &ae) {
            self_type temporary (*this - ae, non_zeros_);
            return assign_temporary (temporary);
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        coordinate_matrix &minus_assign (const matrix_expression<AE> &ae) {
            matrix_assign<scalar_minus_assign> (*this, ae);
            return *this;
        }
        template<class AT>
        BOOST_UBLAS_INLINE
        coordinate_matrix& operator *= (const AT &at) {
            matrix_assign_scalar<scalar_multiplies_assign> (*this, at);
            return *this;
        }
        template<class AT>
        BOOST_UBLAS_INLINE
        coordinate_matrix& operator /= (const AT &at) {
            matrix_assign_scalar<scalar_divides_assign> (*this, at);
            return *this;
        }

        // Swapping
        BOOST_UBLAS_INLINE
        void swap (coordinate_matrix &m) {
            if (this != &m) {
                std::swap (size1_, m.size1_);
                std::swap (size2_, m.size2_);
                std::swap (non_zeros_, m.non_zeros_);
                std::swap (filled_, m.filled_);
                std::swap (sorted_, m.sorted_);
                index1_data ().swap (m.index1_data ());
                index2_data ().swap (m.index2_data ());
                value_data ().swap (m.value_data ());
            }
        }
        BOOST_UBLAS_INLINE
        friend void swap (coordinate_matrix &m1, coordinate_matrix &m2) {
            m1.swap (m2);
        }

        // Sorting and remove old duplicates
        BOOST_UBLAS_INLINE
        void sort () const {
            if (! sorted_ && filled_ > 0) {
                index_triple_array<index_array_type, index_array_type, value_array_type>
                    ita (filled_, index1_data_, index2_data_, value_data_);
                //FIXME use stable_sort when index_xx_array works on all compilers
                std::sort (ita.begin (), ita.end ());    // retain new elements at end
                // remove duplicate element that are earlier in the array.
                size_type filled = 0;
                for (size_type i = 1; i < filled_; ++ i) {
                    if (index1_data_ [filled] != index1_data_ [i] ||
                        index2_data_ [filled] != index2_data_ [i]) {
                        ++ filled;
                        if (filled != i) {
                            index1_data_ [filled] = index1_data_ [i];
                            index2_data_ [filled] = index2_data_ [i];
                            value_data_ [filled] = value_data_ [i];
                        }
                    } else {
                        value_data_ [filled] = value_data_ [i];
                    }
                }
                filled_ = filled + 1;
                sorted_ = true;
            }
        }

        // Back element insertion and erasure
        BOOST_UBLAS_INLINE
        void push_back (size_type i, size_type j, const_reference t) {
            if (filled_ >= non_zeros_)
                reserve (2 * non_zeros_, true);
            size_type element1 = layout_type::element1 (i, size1_, j, size2_);
            size_type element2 = layout_type::element2 (i, size1_, j, size2_);
            if (filled_ == 0 ||
                index1_data () [filled_ - 1] < k_based (element1) ||
                (index1_data () [filled_ - 1] == k_based (element1) && index2_data () [filled_ - 1] < k_based (element2))) {
                ++ filled_;
                index1_data () [filled_ - 1] = k_based (element1);
                index2_data () [filled_ - 1] = k_based (element2);
                value_data () [filled_ - 1] = t;
                return;
            }
            external_logic ().raise ();
        }
        BOOST_UBLAS_INLINE
        void pop_back () {
            BOOST_UBLAS_CHECK (filled_ > 0, external_logic ());
            -- filled_;
        }

        // Iterator types
    private:
        // Use index array iterator
        typedef typename IA::const_iterator vector_const_subiterator_type;
        typedef typename IA::iterator vector_subiterator_type;
        typedef typename IA::const_iterator const_subiterator_type;
        typedef typename IA::iterator subiterator_type;

    public:
        class const_iterator1;
        class iterator1;
        class const_iterator2;
        class iterator2;
        typedef reverse_iterator_base1<const_iterator1> const_reverse_iterator1;
        typedef reverse_iterator_base1<iterator1> reverse_iterator1;
        typedef reverse_iterator_base2<const_iterator2> const_reverse_iterator2;
        typedef reverse_iterator_base2<iterator2> reverse_iterator2;

        // Element lookup
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        const_iterator1 find1 (int rank, size_type i, size_type j, int direction = 1) const {
            sort ();
            for (;;) {
                size_type address1 (layout_type::address1 (i, size1_, j, size2_));
                size_type address2 (layout_type::address2 (i, size1_, j, size2_));
                vector_const_subiterator_type itv_begin (detail::lower_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (address1), std::less<size_type> ()));
                vector_const_subiterator_type itv_end (detail::upper_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (address1), std::less<size_type> ()));

                const_subiterator_type it_begin (index2_data ().begin () + (itv_begin - index1_data ().begin ()));
                const_subiterator_type it_end (index2_data ().begin () + (itv_end - index1_data ().begin ()));

                const_subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (address2), std::less<size_type> ()));
                vector_const_subiterator_type itv (index1_data ().begin () + (it - index2_data ().begin ()));
                if (rank == 0)
                    return const_iterator1 (*this, rank, i, j, itv, it);
                if (it != it_end && zero_based (*it) == address2)
                    return const_iterator1 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast1 ()) {
                        if (it == it_end)
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        i = zero_based (*it);
                    } else {
                        if (i >= size1_)
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        ++ i;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast1 ()) {
                        if (it == index2_data ().begin () + zero_based (*itv))
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        i = zero_based (*(it - 1));
                    } else {
                        if (i == 0)
                            return const_iterator1 (*this, rank, i, j, itv, it);
                        -- i;
                    }
                }
            }
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        iterator1 find1 (int rank, size_type i, size_type j, int direction = 1) {
            sort ();
            for (;;) {
                size_type address1 (layout_type::address1 (i, size1_, j, size2_));
                size_type address2 (layout_type::address2 (i, size1_, j, size2_));
                vector_subiterator_type itv_begin (detail::lower_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (address1), std::less<size_type> ()));
                vector_subiterator_type itv_end (detail::upper_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (address1), std::less<size_type> ()));

                subiterator_type it_begin (index2_data ().begin () + (itv_begin - index1_data ().begin ()));
                subiterator_type it_end (index2_data ().begin () + (itv_end - index1_data ().begin ()));

                subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (address2), std::less<size_type> ()));
                vector_subiterator_type itv (index1_data ().begin () + (it - index2_data ().begin ()));
                if (rank == 0)
                    return iterator1 (*this, rank, i, j, itv, it);
                if (it != it_end && zero_based (*it) == address2)
                    return iterator1 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast1 ()) {
                        if (it == it_end)
                            return iterator1 (*this, rank, i, j, itv, it);
                        i = zero_based (*it);
                    } else {
                        if (i >= size1_)
                            return iterator1 (*this, rank, i, j, itv, it);
                        ++ i;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast1 ()) {
                        if (it == index2_data ().begin () + zero_based (*itv))
                            return iterator1 (*this, rank, i, j, itv, it);
                        i = zero_based (*(it - 1));
                    } else {
                        if (i == 0)
                            return iterator1 (*this, rank, i, j, itv, it);
                        -- i;
                    }
                }
            }
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        const_iterator2 find2 (int rank, size_type i, size_type j, int direction = 1) const {
            sort ();
            for (;;) {
                size_type address1 (layout_type::address1 (i, size1_, j, size2_));
                size_type address2 (layout_type::address2 (i, size1_, j, size2_));
                vector_const_subiterator_type itv_begin (detail::lower_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (address1), std::less<size_type> ()));
                vector_const_subiterator_type itv_end (detail::upper_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (address1), std::less<size_type> ()));

                const_subiterator_type it_begin (index2_data ().begin () + (itv_begin - index1_data ().begin ()));
                const_subiterator_type it_end (index2_data ().begin () + (itv_end - index1_data ().begin ()));

                const_subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (address2), std::less<size_type> ()));
                vector_const_subiterator_type itv (index1_data ().begin () + (it - index2_data ().begin ()));
                if (rank == 0)
                    return const_iterator2 (*this, rank, i, j, itv, it);
                if (it != it_end && zero_based (*it) == address2)
                    return const_iterator2 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast2 ()) {
                        if (it == it_end)
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        j = zero_based (*it);
                    } else {
                        if (j >= size2_)
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        ++ j;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast2 ()) {
                        if (it == index2_data ().begin () + zero_based (*itv))
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        j = zero_based (*(it - 1));
                    } else {
                        if (j == 0)
                            return const_iterator2 (*this, rank, i, j, itv, it);
                        -- j;
                    }
                }
            }
        }
        // BOOST_UBLAS_INLINE This function seems to be big. So we do not let the compiler inline it.    
        iterator2 find2 (int rank, size_type i, size_type j, int direction = 1) {
            sort ();
            for (;;) {
                size_type address1 (layout_type::address1 (i, size1_, j, size2_));
                size_type address2 (layout_type::address2 (i, size1_, j, size2_));
                vector_subiterator_type itv_begin (detail::lower_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (address1), std::less<size_type> ()));
                vector_subiterator_type itv_end (detail::upper_bound (index1_data ().begin (), index1_data ().begin () + filled_, k_based (address1), std::less<size_type> ()));

                subiterator_type it_begin (index2_data ().begin () + (itv_begin - index1_data ().begin ()));
                subiterator_type it_end (index2_data ().begin () + (itv_end - index1_data ().begin ()));

                subiterator_type it (detail::lower_bound (it_begin, it_end, k_based (address2), std::less<size_type> ()));
                vector_subiterator_type itv (index1_data ().begin () + (it - index2_data ().begin ()));
                if (rank == 0)
                    return iterator2 (*this, rank, i, j, itv, it);
                if (it != it_end && zero_based (*it) == address2)
                    return iterator2 (*this, rank, i, j, itv, it);
                if (direction > 0) {
                    if (layout_type::fast2 ()) {
                        if (it == it_end)
                            return iterator2 (*this, rank, i, j, itv, it);
                        j = zero_based (*it);
                    } else {
                        if (j >= size2_)
                            return iterator2 (*this, rank, i, j, itv, it);
                        ++ j;
                    }
                } else /* if (direction < 0)  */ {
                    if (layout_type::fast2 ()) {
                        if (it == index2_data ().begin () + zero_based (*itv))
                            return iterator2 (*this, rank, i, j, itv, it);
                        j = zero_based (*(it - 1));
                    } else {
                        if (j == 0)
                            return iterator2 (*this, rank, i, j, itv, it);
                        -- j;
                    }
                }
            }
        }


        class const_iterator1:
            public container_const_reference<coordinate_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               const_iterator1, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename coordinate_matrix::value_type value_type;
            typedef typename coordinate_matrix::difference_type difference_type;
            typedef typename coordinate_matrix::const_reference reference;
            typedef const typename coordinate_matrix::pointer pointer;

            typedef const_iterator2 dual_iterator_type;
            typedef const_reverse_iterator2 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            const_iterator1 ():
                container_const_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            const_iterator1 (const self_type &m, int rank, size_type i, size_type j, const vector_const_subiterator_type &itv, const const_subiterator_type &it):
                container_const_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}
            BOOST_UBLAS_INLINE
            const_iterator1 (const iterator1 &it):
                container_const_reference<self_type> (it ()), rank_ (it.rank_), i_ (it.i_), j_ (it.j_), itv_ (it.itv_), it_ (it.it_) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            const_iterator1 &operator ++ () {
                if (rank_ == 1 && layout_type::fast1 ())
                    ++ it_;
                else {
                    i_ = index1 () + 1;
                    if (rank_ == 1)
                        *this = (*this) ().find1 (rank_, i_, j_, 1);
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            const_iterator1 &operator -- () {
                if (rank_ == 1 && layout_type::fast1 ())
                    -- it_;
                else {
                    i_ = index1 () - 1;
                    if (rank_ == 1)
                        *this = (*this) ().find1 (rank_, i_, j_, -1);
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            const_reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*this) ().value_data () [it_ - (*this) ().index2_data ().begin ()];
                } else {
                    return (*this) () (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator2 begin () const {
                const self_type &m = (*this) ();
                return m.find2 (1, index1 (), 0);
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator2 end () const {
                const self_type &m = (*this) ();
                return m.find2 (1, index1 (), m.size2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator2 rbegin () const {
                return const_reverse_iterator2 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator2 rend () const {
                return const_reverse_iterator2 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_)) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_));
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_)) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_));
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            const_iterator1 &operator = (const const_iterator1 &it) {
                container_const_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const const_iterator1 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_const_subiterator_type itv_;
            const_subiterator_type it_;
        };

        BOOST_UBLAS_INLINE
        const_iterator1 begin1 () const {
            return find1 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        const_iterator1 end1 () const {
            return find1 (0, size1_, 0);
        }

        class iterator1:
            public container_reference<coordinate_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               iterator1, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename coordinate_matrix::value_type value_type;
            typedef typename coordinate_matrix::difference_type difference_type;
            typedef typename coordinate_matrix::true_reference reference;
            typedef typename coordinate_matrix::pointer pointer;

            typedef iterator2 dual_iterator_type;
            typedef reverse_iterator2 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            iterator1 ():
                container_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            iterator1 (self_type &m, int rank, size_type i, size_type j, const vector_subiterator_type &itv, const subiterator_type &it):
                container_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            iterator1 &operator ++ () {
                if (rank_ == 1 && layout_type::fast1 ())
                    ++ it_;
                else {
                    i_ = index1 () + 1;
                    if (rank_ == 1)
                        *this = (*this) ().find1 (rank_, i_, j_, 1);
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            iterator1 &operator -- () {
                if (rank_ == 1 && layout_type::fast1 ())
                    -- it_;
                else {
                    i_ = index1 () - 1;
                    if (rank_ == 1)
                        *this = (*this) ().find1 (rank_, i_, j_, -1);
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*this) ().value_data () [it_ - (*this) ().index2_data ().begin ()];
                } else {
                    return (*this) ().at_element (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator2 begin () const {
                self_type &m = (*this) ();
                return m.find2 (1, index1 (), 0);
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator2 end () const {
                self_type &m = (*this) ();
                return m.find2 (1, index1 (), m.size2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator2 rbegin () const {
                return reverse_iterator2 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator2 rend () const {
                return reverse_iterator2 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_)) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_));
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find1 (0, (*this) ().size1 (), j_), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_)) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_));
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            iterator1 &operator = (const iterator1 &it) {
                container_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const iterator1 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_subiterator_type itv_;
            subiterator_type it_;

            friend class const_iterator1;
        };

        BOOST_UBLAS_INLINE
        iterator1 begin1 () {
            return find1 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        iterator1 end1 () {
            return find1 (0, size1_, 0);
        }

        class const_iterator2:
            public container_const_reference<coordinate_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               const_iterator2, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename coordinate_matrix::value_type value_type;
            typedef typename coordinate_matrix::difference_type difference_type;
            typedef typename coordinate_matrix::const_reference reference;
            typedef const typename coordinate_matrix::pointer pointer;

            typedef const_iterator1 dual_iterator_type;
            typedef const_reverse_iterator1 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            const_iterator2 ():
                container_const_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            const_iterator2 (const self_type &m, int rank, size_type i, size_type j, const vector_const_subiterator_type itv, const const_subiterator_type &it):
                container_const_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}
            BOOST_UBLAS_INLINE
            const_iterator2 (const iterator2 &it):
                container_const_reference<self_type> (it ()), rank_ (it.rank_), i_ (it.i_), j_ (it.j_), itv_ (it.itv_), it_ (it.it_) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            const_iterator2 &operator ++ () {
                if (rank_ == 1 && layout_type::fast2 ())
                    ++ it_;
                else {
                    j_ = index2 () + 1;
                    if (rank_ == 1)
                        *this = (*this) ().find2 (rank_, i_, j_, 1);
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            const_iterator2 &operator -- () {
                if (rank_ == 1 && layout_type::fast2 ())
                    -- it_;
                else {
                    j_ = index2 () - 1;
                    if (rank_ == 1)
                        *this = (*this) ().find2 (rank_, i_, j_, -1);
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            const_reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*this) ().value_data () [it_ - (*this) ().index2_data ().begin ()];
                } else {
                    return (*this) () (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator1 begin () const {
                const self_type &m = (*this) ();
                return m.find1 (1, 0, index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_iterator1 end () const {
                const self_type &m = (*this) ();
                return m.find1 (1, m.size1 (), index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator1 rbegin () const {
                return const_reverse_iterator1 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            const_reverse_iterator1 rend () const {
                return const_reverse_iterator1 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_)) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_));
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_)) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_));
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            const_iterator2 &operator = (const const_iterator2 &it) {
                container_const_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const const_iterator2 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_const_subiterator_type itv_;
            const_subiterator_type it_;
        };

        BOOST_UBLAS_INLINE
        const_iterator2 begin2 () const {
            return find2 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        const_iterator2 end2 () const {
            return find2 (0, 0, size2_);
        }

        class iterator2:
            public container_reference<coordinate_matrix>,
            public bidirectional_iterator_base<sparse_bidirectional_iterator_tag,
                                               iterator2, value_type> {
        public:
            typedef sparse_bidirectional_iterator_tag iterator_category;
            typedef typename coordinate_matrix::value_type value_type;
            typedef typename coordinate_matrix::difference_type difference_type;
            typedef typename coordinate_matrix::true_reference reference;
            typedef typename coordinate_matrix::pointer pointer;

            typedef iterator1 dual_iterator_type;
            typedef reverse_iterator1 dual_reverse_iterator_type;

            // Construction and destruction
            BOOST_UBLAS_INLINE
            iterator2 ():
                container_reference<self_type> (), rank_ (), i_ (), j_ (), itv_ (), it_ () {}
            BOOST_UBLAS_INLINE
            iterator2 (self_type &m, int rank, size_type i, size_type j, const vector_subiterator_type &itv, const subiterator_type &it):
                container_reference<self_type> (m), rank_ (rank), i_ (i), j_ (j), itv_ (itv), it_ (it) {}

            // Arithmetic
            BOOST_UBLAS_INLINE
            iterator2 &operator ++ () {
                if (rank_ == 1 && layout_type::fast2 ())
                    ++ it_;
                else {
                    j_ = index2 () + 1;
                    if (rank_ == 1)
                        *this = (*this) ().find2 (rank_, i_, j_, 1);
                }
                return *this;
            }
            BOOST_UBLAS_INLINE
            iterator2 &operator -- () {
                if (rank_ == 1 && layout_type::fast2 ())
                    -- it_;
                else {
                    j_ = index2 ();
                    if (rank_ == 1)
                        *this = (*this) ().find2 (rank_, i_, j_, -1);
                }
                return *this;
            }

            // Dereference
            BOOST_UBLAS_INLINE
            reference operator * () const {
                BOOST_UBLAS_CHECK (index1 () < (*this) ().size1 (), bad_index ());
                BOOST_UBLAS_CHECK (index2 () < (*this) ().size2 (), bad_index ());
                if (rank_ == 1) {
                    return (*this) ().value_data () [it_ - (*this) ().index2_data ().begin ()];
                } else {
                    return (*this) ().at_element (i_, j_);
                }
            }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator1 begin () const {
                self_type &m = (*this) ();
                return m.find1 (1, 0, index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            iterator1 end () const {
                self_type &m = (*this) ();
                return m.find1 (1, m.size1 (), index2 ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator1 rbegin () const {
                return reverse_iterator1 (end ());
            }
            BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
            typename self_type::
#endif
            reverse_iterator1 rend () const {
                return reverse_iterator1 (begin ());
            }
#endif

            // Indices
            BOOST_UBLAS_INLINE
            size_type index1 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index1 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_)) < (*this) ().size1 (), bad_index ());
                    return layout_type::index1 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_));
                } else {
                    return i_;
                }
            }
            BOOST_UBLAS_INLINE
            size_type index2 () const {
                BOOST_UBLAS_CHECK (*this != (*this) ().find2 (0, i_, (*this) ().size2 ()), bad_index ());
                if (rank_ == 1) {
                    BOOST_UBLAS_CHECK (layout_type::index2 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_)) < (*this) ().size2 (), bad_index ());
                    return layout_type::index2 ((*this) ().zero_based (*itv_), (*this) ().zero_based (*it_));
                } else {
                    return j_;
                }
            }

            // Assignment
            BOOST_UBLAS_INLINE
            iterator2 &operator = (const iterator2 &it) {
                container_reference<self_type>::assign (&it ());
                rank_ = it.rank_;
                i_ = it.i_;
                j_ = it.j_;
                itv_ = it.itv_;
                it_ = it.it_;
                return *this;
            }

            // Comparison
            BOOST_UBLAS_INLINE
            bool operator == (const iterator2 &it) const {
                BOOST_UBLAS_CHECK (&(*this) () == &it (), external_logic ());
                // BOOST_UBLAS_CHECK (rank_ == it.rank_, internal_logic ());
                if (rank_ == 1 || it.rank_ == 1) {
                    return it_ == it.it_;
                } else {
                    return i_ == it.i_ && j_ == it.j_;
                }
            }

        private:
            int rank_;
            size_type i_;
            size_type j_;
            vector_subiterator_type itv_;
            subiterator_type it_;

            friend class const_iterator2;
        };

        BOOST_UBLAS_INLINE
        iterator2 begin2 () {
            return find2 (0, 0, 0);
        }
        BOOST_UBLAS_INLINE
        iterator2 end2 () {
            return find2 (0, 0, size2_);
        }

        // Reverse iterators

        BOOST_UBLAS_INLINE
        const_reverse_iterator1 rbegin1 () const {
            return const_reverse_iterator1 (end1 ());
        }
        BOOST_UBLAS_INLINE
        const_reverse_iterator1 rend1 () const {
            return const_reverse_iterator1 (begin1 ());
        }

        BOOST_UBLAS_INLINE
        reverse_iterator1 rbegin1 () {
            return reverse_iterator1 (end1 ());
        }
        BOOST_UBLAS_INLINE
        reverse_iterator1 rend1 () {
            return reverse_iterator1 (begin1 ());
        }

        BOOST_UBLAS_INLINE
        const_reverse_iterator2 rbegin2 () const {
            return const_reverse_iterator2 (end2 ());
        }
        BOOST_UBLAS_INLINE
        const_reverse_iterator2 rend2 () const {
            return const_reverse_iterator2 (begin2 ());
        }

        BOOST_UBLAS_INLINE
        reverse_iterator2 rbegin2 () {
            return reverse_iterator2 (end2 ());
        }
        BOOST_UBLAS_INLINE
        reverse_iterator2 rend2 () {
            return reverse_iterator2 (begin2 ());
        }

    private:
        size_type size1_;
        size_type size2_;
        size_type non_zeros_;
        mutable size_type filled_;
        mutable bool sorted_;
        mutable index_array_type index1_data_;
        mutable index_array_type index2_data_;
        mutable value_array_type value_data_;
        static const value_type zero_;

        BOOST_UBLAS_INLINE
        static size_type zero_based (size_type k_based_index) {
            return k_based_index - IB;
        }
        BOOST_UBLAS_INLINE
        static size_type k_based (size_type zero_based_index) {
            return zero_based_index + IB;
        }

        friend class iterator1;
        friend class iterator2;
        friend class const_iterator1;
        friend class const_iterator2;
    };

    template<class T, class L, std::size_t IB, class IA, class TA>
    const typename coordinate_matrix<T, L, IB, IA, TA>::value_type coordinate_matrix<T, L, IB, IA, TA>::zero_ (0);

}}}

#endif
