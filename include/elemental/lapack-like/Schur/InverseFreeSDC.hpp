/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_LAPACK_SCHUR_INVERSEFREESDC_HPP
#define ELEM_LAPACK_SCHUR_INVERSEFREESDC_HPP

#include "elemental/blas-like/level1/Axpy.hpp"
#include "elemental/blas-like/level1/AxpyTriangle.hpp"
#include "elemental/blas-like/level1/Scale.hpp"
#include "elemental/lapack-like/Norm/One.hpp"
#include "elemental/lapack-like/QR.hpp"
#include "elemental/lapack-like/RQ.hpp"
#include "elemental/lapack-like/Trace.hpp"
#include "elemental/matrices/Identity.hpp"
#include "elemental/matrices/Zeros.hpp"

// See Z. Bai, J. Demmel, J. Dongarra, A. Petitet, H. Robinson, and K. Stanley's
// "The spectral decomposition of nonsymmetric matrices on distributed memory
// parallel computers". Currently available at:
// www.netlib.org/lapack/lawnspdf/lawn91.pdf

namespace elem {
namespace schur {

//
// X = [B;A]
//

template<typename F>
inline int
InverseFreeSign( Matrix<F>& X, Int maxIts=100, BASE(F) tau=0 )
{
#ifndef RELEASE
    CallStackEntry cse("schur::InverseFreeSign");
#endif
    typedef BASE(F) Real;
    const Int n = X.Width();
    if( X.Height() != 2*n )
        LogicError("X must be 2n x n");
    // Compute the tolerance if it is unset
    if( tau == Real(0) )
        tau = n*lapack::MachineEpsilon<Real>();

    // Expose A and B in the original and temporary
    Matrix<F> XAlt( 2*n, n );
    Matrix<F> A, B, AAlt, BAlt;
    PartitionDown( X, B, A, n );
    PartitionDown( XAlt, BAlt, AAlt, n );

    // Flip the sign of A
    Scale( F(-1), A );

    // Set up the space for explicitly computing the left half of Q
    Matrix<F> t;
    Matrix<F> Q( 2*n, n ), Q12, Q22;
    PartitionDown( Q, Q12, Q22, n );

    // Run the iterative algorithm
    Int numIts=0;
    Matrix<F> R, RLast;
    while( numIts < maxIts )
    {
        XAlt = X;
        QR( XAlt, t );
 
        // Form the left half of Q
        MakeZeros( Q12 );
        MakeIdentity( Q22 );
        qr::ApplyQ( LEFT, NORMAL, XAlt, t, Q );

        // Save a copy of R
        R = BAlt;
        MakeTriangular( UPPER, R );
        
        // Form the new iterate
        Gemm( ADJOINT, NORMAL, F(1), Q12, A, F(0), AAlt );
        Gemm( ADJOINT, NORMAL, F(1), Q22, B, F(0), BAlt );
        X = XAlt;

        // Use the difference in the iterates to test for convergence
        ++numIts;
        if( numIts > 1 )
        {
            const Real oneRLast = OneNorm(RLast);     
            AxpyTriangle( UPPER, F(-1), R, RLast );
            const Real oneRDiff = OneNorm(RLast);
            if( oneRDiff <= tau*oneRLast )
                break;
        }
        RLast = R;
    }

    // Revert the sign of A and return
    Scale( F(-1), A );
    return numIts;
}

template<typename F>
inline int
InverseFreeSign( DistMatrix<F>& X, Int maxIts=100, BASE(F) tau=0 )
{
#ifndef RELEASE
    CallStackEntry cse("schur::InverseFreeSign");
#endif
    typedef BASE(F) Real;
    const Grid& g = X.Grid();
    const Int n = X.Width();
    if( X.Height() != 2*n )
        LogicError("X must be 2n x n");
    // Compute the tolerance if it is unset
    if( tau == Real(0) )
        tau = n*lapack::MachineEpsilon<Real>();

    // Expose A and B in the original and temporary
    DistMatrix<F> XAlt( 2*n, n, g );
    DistMatrix<F> A(g), B(g), AAlt(g), BAlt(g);
    PartitionDown( X, B, A, n );
    PartitionDown( XAlt, BAlt, AAlt, n );

    // Flip the sign of A
    Scale( F(-1), A );

    // Set up the space for explicitly computing the left half of Q
    DistMatrix<F,MD,STAR> t(g);
    DistMatrix<F> Q( 2*n, n, g ), Q12(g), Q22(g);
    PartitionDown( Q, Q12, Q22, n );

    // Run the iterative algorithm
    Int numIts=0;
    DistMatrix<F> R(g), RLast(g);
    while( numIts < maxIts )
    {
        XAlt = X;
        QR( XAlt, t );
 
        // Form the left half of Q
        MakeZeros( Q12 );
        MakeIdentity( Q22 );
        qr::ApplyQ( LEFT, NORMAL, XAlt, t, Q );

        // Save a copy of R
        R = BAlt;
        MakeTriangular( UPPER, R );
        
        // Form the new iterate
        Gemm( ADJOINT, NORMAL, F(1), Q12, A, F(0), AAlt );
        Gemm( ADJOINT, NORMAL, F(1), Q22, B, F(0), BAlt );
        X = XAlt;

        // Use the difference in the iterates to test for convergence
        ++numIts;
        if( numIts > 1 )
        {
            const Real oneRLast = OneNorm(RLast);     
            AxpyTriangle( UPPER, F(-1), R, RLast );
            const Real oneRDiff = OneNorm(RLast);
            if( oneRDiff <= tau*oneRLast )
                break;
        }
        RLast = R;
    }

    // Revert the sign of A and return
    Scale( F(-1), A );
    return numIts;
}

template<typename F>
inline BASE(F)
InverseFreeSpectralDivide( Matrix<F>& X )
{
#ifndef RELEASE
    CallStackEntry cse("schur::InverseFreeSpectralDivide");
#endif
    const Int n = X.Width();
    if( X.Height() != 2*n )
        LogicError("Matrix should be 2n x n");
   
    // Expose A and B, and then copy A
    Matrix<F> A, B;
    PartitionDown( X, B, A, n );
    Matrix<F> ACopy( A );

    // Run the inverse-free alternative to Sign
    InverseFreeSign( X );

    // Compute the pivoted QR decomp of inv(A + B) A [See LAWN91]
    // 1) B := A + B 
    // 2) [Q,R,Pi] := QRP(A)
    // 3) B := Q^H B
    // 4) [R,Q] := RQ(B)
    Axpy( F(1), A, B );
    Matrix<F> t;
    Matrix<Int> p;
    QR( A, t, p );
    qr::ApplyQ( LEFT, ADJOINT, A, t, B );
    RQ( B, t );

    // TODO: Compute rank more carefully
    const F trace = Trace(B);
    const Int roundedTrace = Int(round(RealPart(trace)));
    const Int rank = Max( Min( roundedTrace, n ), 0 );

    // A := Q^H A Q
    A = ACopy;
    rq::ApplyQ( LEFT, ADJOINT, B, t, A );
    rq::ApplyQ( RIGHT, NORMAL, B, t, A );

    // Return || E21 ||1 / || A ||1
    Matrix<F> E21;
    LockedView( E21, A, rank, 0, n-rank, rank );
    return OneNorm(E21)/OneNorm(ACopy);
}

template<typename F>
inline BASE(F)
InverseFreeSpectralDivide( DistMatrix<F>& X )
{
#ifndef RELEASE
    CallStackEntry cse("schur::InverseFreeSpectralDivide");
#endif
    const Grid& g = X.Grid();
    const Int n = X.Width();
    if( X.Height() != 2*n )
        LogicError("Matrix should be 2n x n");
   
    // Expose A and B, and then copy A
    DistMatrix<F> A(g), B(g);
    PartitionDown( X, B, A, n );
    DistMatrix<F> ACopy( A );

    // Run the inverse-free alternative to Sign
    InverseFreeSign( X );

    // Compute the pivoted QR decomp of inv(A + B) A [See LAWN91]
    // 1) B := A + B 
    // 2) [Q,R,Pi] := QRP(A)
    // 3) B := Q^H B
    // 4) [R,Q] := RQ(B)
    Axpy( F(1), A, B );
    DistMatrix<F,MD,STAR> t(g);
    DistMatrix<Int,VR,STAR> p(g);
    QR( A, t, p );
    qr::ApplyQ( LEFT, ADJOINT, A, t, B );
    RQ( B, t );

    // TODO: Compute rank more carefully
    const F trace = Trace(B);
    const Int roundedTrace = Int(round(RealPart(trace)));
    const Int rank = Max( Min( roundedTrace, n ), 0 );

    // A := Q^H A Q
    A = ACopy;
    rq::ApplyQ( LEFT, ADJOINT, B, t, A );
    rq::ApplyQ( RIGHT, NORMAL, B, t, A );

    // Return || E21 ||1 / || A ||1
    DistMatrix<F> E21(g);
    LockedView( E21, A, rank, 0, n-rank, rank );
    return OneNorm(E21)/OneNorm(ACopy);
}

} // namespace schur
} // namespace elem

#endif // ifndef ELEM_LAPACK_SCHUR_INVERSEFREESDC_HPP
