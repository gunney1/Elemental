/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_DISTMATRIX_STAR_MR_DECL_HPP
#define ELEM_DISTMATRIX_STAR_MR_DECL_HPP

namespace elem {

// Partial specialization to A[* ,MR].
//
// The columns of these distributed matrices will be replicated on all 
// processes (*), and the rows will be distributed like "Matrix Rows" (MR).
// Thus the rows will be distributed among rows of the process grid.
template<typename T>
class DistMatrix<T,STAR,MR> : public GeneralDistMatrix<T,STAR,MR>
{
public:
    // Typedefs
    // ========
    typedef AbstractDistMatrix<T> absType;
    typedef GeneralDistMatrix<T,STAR,MR> genType;
    typedef DistMatrix<T,STAR,MR> type;

    // Constructors and destructors
    // ============================

    // Inherited constructors are part of C++11 but not yet widely supported.
    //using GeneralDistMatrix<T,STAR,MR>::GeneralDistMatrix;

    // Create a 0 x 0 distributed matrix
    DistMatrix( const elem::Grid& g=DefaultGrid(), Int root=0 );
    // Create a height x width distributed matrix
    DistMatrix
    ( Int height, Int width, const elem::Grid& g=DefaultGrid(), Int root=0 );
    // Create a copy of distributed matrix A
    DistMatrix( const type& A );
    template<Dist U,Dist V> DistMatrix( const DistMatrix<T,U,V>& A );
#ifndef SWIG
    // Move constructor
    DistMatrix( type&& A ) noexcept;
#endif
    ~DistMatrix();

    // Assignment and reconfiguration
    // ==============================
    type& operator=( const DistMatrix<T,MC,  MR  >& A );
    type& operator=( const DistMatrix<T,MC,  STAR>& A );
    type& operator=( const DistMatrix<T,STAR,MR  >& A );
    type& operator=( const DistMatrix<T,MD,  STAR>& A );
    type& operator=( const DistMatrix<T,STAR,MD  >& A );
    type& operator=( const DistMatrix<T,MR,  MC  >& A );
    type& operator=( const DistMatrix<T,MR,  STAR>& A );
    type& operator=( const DistMatrix<T,STAR,MC  >& A );
    type& operator=( const DistMatrix<T,VC,  STAR>& A );
    type& operator=( const DistMatrix<T,STAR,VC  >& A );
    type& operator=( const DistMatrix<T,VR,  STAR>& A );
    type& operator=( const DistMatrix<T,STAR,VR  >& A );
    type& operator=( const DistMatrix<T,STAR,STAR>& A );
    type& operator=( const DistMatrix<T,CIRC,CIRC>& A );
#ifndef SWIG
    // Move assignment
    type& operator=( type&& A );
#endif

    // Basic queries
    // =============
    elem::DistData DistData() const override;
    mpi::Comm DistComm() const override;
    mpi::Comm CrossComm() const override;
    mpi::Comm RedundantComm() const override;
    mpi::Comm ColComm() const override;
    mpi::Comm RowComm() const override;
    Int RowStride() const override;
    Int ColStride() const override;

private:
    // Friend declarations
    // ===================
#ifndef SWIG
    template<typename S,Dist U,Dist V> friend class DistMatrix;
    template<typename S,Dist U,Dist V> friend class BlockDistMatrix;
#endif 
};

} // namespace elem

#endif // ifndef ELEM_DISTMATRIX_STAR_MR_DECL_HPP
