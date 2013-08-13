/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
// NOTE: It is possible to simply include "elemental.hpp" instead
#include "elemental-lite.hpp"
#include "elemental/blas-like/level3/Gemm.hpp"
#include "elemental/lapack-like/ApplyColumnPivots.hpp"
#include "elemental/lapack-like/ID.hpp"
#include "elemental/lapack-like/Norm/Frobenius.hpp"
#include "elemental/matrices/Uniform.hpp"
#include "elemental/matrices/Zeros.hpp"
using namespace std;
using namespace elem;

typedef double Real;
typedef Complex<Real> C;

int
main( int argc, char* argv[] )
{
    Initialize( argc, argv );

    try 
    {
        const Int m = Input("--height","height of matrix",20);
        const Int n = Input("--width","width of matrix",100);
        const Int maxSteps = Input("--maxSteps","max # of steps of QR",10);
        const double tol = Input("--tol","tolerance for ID",-1.);
        const bool print = Input("--print","print matrices?",false);
        ProcessInput();
        PrintInputReport();

        DistMatrix<C> A;
        Uniform( A, m, n );
        const Real frobA = FrobeniusNorm( A );
        if( print )
            Print( A, "A" );

        const Grid& g = A.Grid();
        DistMatrix<Int,VR,STAR> p(g);
        DistMatrix<C,STAR,VR> Z(g);
        ID( A, p, Z, maxSteps, tol );
        const Int numSteps = p.Height();
        if( print )
        {
            Print( p, "p" );
            Print( Z, "Z" );
        }

        // Pivot A and form the matrix of its (hopefully) dominant columns
        ApplyColumnPivots( A, p );
        DistMatrix<C> hatA( A );
        hatA.ResizeTo( m, numSteps );
        if( print )
        {
            Print( A, "A P" );
            Print( hatA, "\\hat{A}" );
        }

        // Check || A P - \hat{A} [I, Z] ||_F / || A ||_F
        DistMatrix<C> AL(g), AR(g);
        PartitionRight( A, AL, AR, numSteps );
        MakeZeros( AL );
        {
            DistMatrix<C,MC,STAR> hatA_MC_STAR(g);
            DistMatrix<C,STAR,MR> Z_STAR_MR(g);
            hatA_MC_STAR.AlignWith( AR );
            Z_STAR_MR.AlignWith( AR );
            hatA_MC_STAR = hatA;
            Z_STAR_MR = Z;
            LocalGemm
            ( NORMAL, NORMAL, C(-1), hatA_MC_STAR, Z_STAR_MR, C(1), AR );
        }
        const Real frobError = FrobeniusNorm( A );
        if( print )
            Print( A, "A P - \\hat{A} [I, Z]" );

        if( mpi::WorldRank() == 0 )
        {
            std::cout << "|| A ||_F = " << frobA << "\n\n"
                      << "|| A P - \\hat{A} [I, Z] ||_F / || A ||_F = " 
                      << frobError/frobA << "\n" << std::endl;
        }
    }
    catch( exception& e ) { ReportException(e); }

    Finalize();
    return 0;
}
