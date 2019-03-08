/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#include <El.hpp>

namespace El {

template<typename Ring>
Base<Ring> InfinityNorm( const Matrix<Ring>& A )
{
    EL_DEBUG_CSE
    typedef Base<Ring> Real;
    const Int height = A.Height();
    const Int width = A.Width();

    Real maxRowSum = 0;
    for( Int i=0; i<height; ++i )
    {
        Real rowSum = 0;
        for( Int j=0; j<width; ++j )
            rowSum += Abs(A(i,j));

        if( limits::IsFinite(rowSum) )
            maxRowSum = Max( maxRowSum, rowSum );
        else
            maxRowSum = rowSum;
    }
    return maxRowSum;
}

template<typename Ring>
Base<Ring> HermitianInfinityNorm( UpperOrLower uplo, const Matrix<Ring>& A )
{
    EL_DEBUG_CSE
    return HermitianOneNorm( uplo, A );
}

template<typename Ring>
Base<Ring> SymmetricInfinityNorm( UpperOrLower uplo, const Matrix<Ring>& A )
{
    EL_DEBUG_CSE
    return HermitianInfinityNorm( uplo, A );
}

template<typename Ring>
Base<Ring> InfinityNorm( const AbstractDistMatrix<Ring>& A )
{
    EL_DEBUG_CSE
    // Compute the partial row sums defined by our local matrix, A[U,V]
    typedef Base<Ring> Real;

    Real norm;
    if( A.Participating() )
    {
        const Int localHeight = A.LocalHeight();
        const Int localWidth = A.LocalWidth();
        const Matrix<Ring>& ALoc = A.LockedMatrix();

        vector<Real> myPartialRowSums( localHeight );
        for( Int iLoc=0; iLoc<localHeight; ++iLoc )
        {
            myPartialRowSums[iLoc] = 0;
            for( Int jLoc=0; jLoc<localWidth; ++jLoc )
                myPartialRowSums[iLoc] += Abs(ALoc(iLoc,jLoc));
        }

        // Sum our partial row sums to get the row sums over A[U,* ]
        vector<Real> myRowSums( localHeight );
        mpi::AllReduce
        ( myPartialRowSums.data(), myRowSums.data(), localHeight, A.RowComm() );

        // Find the maximum out of the row sums
        Real myMaxRowSum = 0;
        for( Int iLoc=0; iLoc<localHeight; ++iLoc )
        {
            if( limits::IsFinite(myRowSums[iLoc]) )
                myMaxRowSum = Max( myMaxRowSum, myRowSums[iLoc] );
            else
                myMaxRowSum = myRowSums[iLoc];
        }

        // Find the global maximum row sum by searching over the U team
        norm = mpi::AllReduce( myMaxRowSum, mpi::MAX, A.ColComm() );
    }
    mpi::Broadcast( norm, A.Root(), A.CrossComm() );
    return norm;
}

template<typename Ring>
Base<Ring> HermitianInfinityNorm
( UpperOrLower uplo, const AbstractDistMatrix<Ring>& A )
{
    EL_DEBUG_CSE
    return HermitianOneNorm( uplo, A );
}

template<typename Ring>
Base<Ring> SymmetricInfinityNorm
( UpperOrLower uplo, const AbstractDistMatrix<Ring>& A )
{
    EL_DEBUG_CSE
    return HermitianInfinityNorm( uplo, A );
}

template<typename Ring>
Base<Ring> HermitianTridiagInfinityNorm
( const Matrix<Base<Ring>>& d, const Matrix<Ring>& e )
{
    EL_DEBUG_CSE
    return HermitianTridiagOneNorm( d, e );
}

#define PROTO(Ring) \
  template Base<Ring> InfinityNorm( const Matrix<Ring>& A ); \
  template Base<Ring> InfinityNorm ( const AbstractDistMatrix<Ring>& A ); \
  template Base<Ring> HermitianInfinityNorm \
  ( UpperOrLower uplo, const Matrix<Ring>& A ); \
  template Base<Ring> HermitianInfinityNorm \
  ( UpperOrLower uplo, const AbstractDistMatrix<Ring>& A ); \
  template Base<Ring> SymmetricInfinityNorm \
  ( UpperOrLower uplo, const Matrix<Ring>& A ); \
  template Base<Ring> SymmetricInfinityNorm \
  ( UpperOrLower uplo, const AbstractDistMatrix<Ring>& A ); \
  template Base<Ring> HermitianTridiagInfinityNorm \
  ( const Matrix<Base<Ring>>& d, const Matrix<Ring>& e );

#define EL_ENABLE_DOUBLEDOUBLE
#define EL_ENABLE_QUADDOUBLE
#define EL_ENABLE_QUAD
#define EL_ENABLE_BIGFLOAT
#define EL_ENABLE_HALF
#include <El/macros/Instantiate.h>

} // namespace El
