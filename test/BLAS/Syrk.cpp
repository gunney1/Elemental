/*
   Copyright 2009-2010 Jack Poulson

   This file is part of Elemental.

   Elemental is free software: you can redistribute it and/or modify it under
   the terms of the GNU Lesser General Public License as published by the
   Free Software Foundation; either version 3 of the License, or 
   (at your option) any later version.

   Elemental is distributed in the hope that it will be useful, but 
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with Elemental. If not, see <http://www.gnu.org/licenses/>.
*/
#include <cmath>
#include <ctime>
#include <sstream>
#include "Elemental.h"
#include "ElementalBLAS_Internal.h"
using namespace std;
using namespace Elemental;
using namespace Elemental::wrappers::MPI;

void Usage()
{
    cout << "SYmmetric Rank-K update." << endl << endl;;
    cout << "  Syrk <r> <c> <shape> <trans?> <m> <k> <nb> ";
    cout << "<correctness?> <print?>   " << endl << endl;;
    cout << "  r: number of process rows             " << endl;
    cout << "  c: number of process cols             " << endl;
    cout << "  shape?: {L,U}                         " << endl;
    cout << "  trans?: {N,T}                         " << endl;
    cout << "  m: height of C                        " << endl;
    cout << "  k: inner dimension                    " << endl;
    cout << "  nb: algorithmic blocksize             " << endl;
    cout << "  correctness?: false iff 0             " << endl;
    cout << "  print?: false iff 0                   " << endl;
    cout << endl;
}

template<typename T>
bool OKRelativeError( T truth, T computed );

template<>
bool OKRelativeError( float truth, float computed )
{
    return ( fabs(truth-computed) / max(fabs(truth),(float)1) <= 1e-5 );
}

template<>
bool OKRelativeError( double truth, double computed )
{
    return ( fabs(truth-computed) / max(fabs(truth),(double)1) <= 1e-13 );
}

#ifndef WITHOUT_COMPLEX
template<>
bool OKRelativeError( scomplex truth, scomplex computed )
{
    return ( norm(truth-computed) / max(norm(truth),(float)1) <= 1e-5 );
}

template<>
bool OKRelativeError( dcomplex truth, dcomplex computed )
{
    return ( norm(truth-computed) / max(norm(truth),(double)1) <= 1e-13 );
}
#endif

template<typename T>
void TestCorrectness( const Shape shape, const Orientation orientation,
                      const T alpha, 
                      const DistMatrix<T,Star,Star>& A_ref,
                      const T beta,        
                            DistMatrix<T,Star,Star>& C_ref,
                      const DistMatrix<T,MC,MR>& C,
                      const bool printMatrices                          )
{
    const Grid& grid = C.GetGrid();
    DistMatrix<T,Star,Star> C_copy(grid);

    if( grid.VCRank() == 0 )
    {
        cout << "  Gathering computed result...";
        cout.flush();
    }
    C_copy = C;
    if( grid.VCRank() == 0 )
        cout << "DONE" << endl;

    if( grid.VCRank() == 0 )
    {
        cout << "  Computing 'truth'...";
        cout.flush();
    }
    BLAS::Syrk( shape, orientation,
                alpha, A_ref.LockedLocalMatrix(),
                beta,  C_ref.LocalMatrix()       );
    if( grid.VCRank() == 0 )
        cout << "DONE" << endl;

    if( printMatrices )
        C_ref.Print("Truth:");

    if( grid.VCRank() == 0 )
    {
        cout << "  Testing correctness...";
        cout.flush();
    }
    if( shape == Lower )
    {
        for( int j=0; j<C.Width(); ++j )
        {
            for( int i=j; i<C.Height(); ++i )
            {
                T truth = C_ref.LocalEntry(i,j);
                T computed = C_copy.LocalEntry(i,j);

                if( ! OKRelativeError( truth, computed ) )
                {
                    cout << "FAILED on process " << grid.VCRank() 
                         << " at index (" << i << "," << j << "): truth="
                         << truth << ", computed=" << computed << endl;
                    throw exception();
                }
            }
        }
    }
    else
    {
        for( int j=0; j<C.Width(); ++j )
        {
            for( int i=0; i<=j; ++i )
            {
                T truth = C_ref.LocalEntry(i,j);
                T computed = C_copy.LocalEntry(i,j);

                if( ! OKRelativeError( truth, computed ) )
                {
                    cout << "FAILED on process " << grid.VCRank() 
                         << " at index (" << i << "," << j << "): truth="
                         << truth << ", computed=" << computed << endl;
                    throw exception();
                }
            }
        }
    }
    Barrier( grid.VCComm() );
    if( grid.VCRank() == 0 )
        cout << "PASSED" << endl;
}

template<typename T>
void TestSyrk
( const Shape shape, const Orientation orientation,
  const int m, const int k, const T alpha, const T beta,
  const bool testCorrectness, const bool printMatrices, const Grid& grid  )
{
    double startTime, endTime, runTime, gFlops;
    DistMatrix<T,MC,MR> A(grid);
    DistMatrix<T,MC,MR> C(grid);
    DistMatrix<T,Star,Star> A_ref(grid);
    DistMatrix<T,Star,Star> C_ref(grid);

    if( orientation == Normal )
        A.ResizeTo( m, k );
    else
        A.ResizeTo( k, m );

    C.ResizeTo( m, m );

    A.SetToRandom();
    C.SetToRandom();
    C.MakeTrapezoidal( Left, shape );
    if( testCorrectness )
    {
        if( grid.VCRank() == 0 )
        {
            cout << "  Making copies of original matrices...";
            cout.flush();
        }
        A_ref = A;
        C_ref = C;
        if( grid.VCRank() == 0 )
            cout << "DONE" << endl;
    }
    if( printMatrices )
    {
        A.Print("A");
        C.Print("C");
    }
    if( grid.VCRank() == 0 )
    {
        cout << "  Starting Syrk...";
        cout.flush();
    }
    Barrier( MPI_COMM_WORLD );
    startTime = Time();
    BLAS::Syrk( shape, orientation, alpha, A, beta, C );
    Barrier( MPI_COMM_WORLD );
    endTime = Time();
    runTime = endTime - startTime;
    gFlops = BLAS::Internal::SyrkGFlops<T>(m,k,runTime);
    if( grid.VCRank() == 0 )
        cout << "DONE. GFlops = " << gFlops << endl;
    if( printMatrices )
    {
        ostringstream msg;
        if( orientation == Normal )
            msg << "C := " << alpha << " A A' + " << beta << " C";
        else
            msg << "C := " << alpha << " A' A + " << beta << " C";
        C.Print( msg.str() );
    }
    if( testCorrectness )
    {
        TestCorrectness
        ( shape, orientation, alpha, A_ref, beta, C_ref, C, printMatrices );
    }
}

int main( int argc, char* argv[] )
{
    int rank;
    Elemental::Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    if( argc != 10 )
    {
        if( rank == 0 )
            Usage();
        Elemental::Finalize();
        return 0;
    }
    try
    {
        const int         r = atoi( argv[1] );
        const int         c = atoi( argv[2] );
        const Shape       shape = CharToShape( *argv[3] );
        const Orientation orientation = CharToOrientation( *argv[4] );
        const int         m = atoi( argv[5] );
        const int         k = atoi( argv[6] );
        const int         nb = atoi( argv[7] );
        const bool        testCorrectness = atoi( argv[8] );
        const bool        printMatrices = atoi( argv[9] );
#ifndef RELEASE
        if( rank == 0 )
        {
            cout << "==========================================" << endl;
            cout << " In debug mode! Performance will be poor! " << endl;
            cout << "==========================================" << endl;
        }
#endif
        Grid grid( MPI_COMM_WORLD, r, c );
        SetBlocksize( nb );

        if( rank == 0 )
        {
            cout << "Will test Syrk" << ShapeToChar(shape) 
                                     << OrientationToChar(orientation) << endl;
        }

        if( rank == 0 )
        {
            cout << "--------------------" << endl;
            cout << "Testing with floats:" << endl;
            cout << "--------------------" << endl;
        }
        TestSyrk<float>( shape, orientation, m, k, (float)3, (float)4,
                         testCorrectness, printMatrices, grid         );
        if( rank == 0 )
            cout << endl;

        if( rank == 0 )
        {
            cout << "---------------------" << endl;
            cout << "Testing with doubles:" << endl;
            cout << "---------------------" << endl;
        }
        TestSyrk<double>( shape, orientation, m, k, (double)3, (double)4,
                          testCorrectness, printMatrices, grid           );
        if( rank == 0 )
            cout << endl;

#ifndef WITHOUT_COMPLEX
        if( rank == 0 )
        {
            cout << "--------------------------------------" << endl;
            cout << "Testing with single-precision complex:" << endl;
            cout << "--------------------------------------" << endl;
        }
        TestSyrk<scomplex>( shape, orientation, m, k, (float)3, (float)4,
                            testCorrectness, printMatrices, grid         );
        if( rank == 0 )
            cout << endl;

        if( rank == 0 )
        {
            cout << "--------------------------------------" << endl;
            cout << "Testing with double-precision complex:" << endl;
            cout << "--------------------------------------" << endl;
        }
        TestSyrk<dcomplex>( shape, orientation, m, k, (double)3, (double)4,
                            testCorrectness, printMatrices, grid           );
        if( rank == 0 )
            cout << endl;
#endif
    }
    catch( exception e )
    {
        cerr << "Caught exception on process " << rank << endl;
    }
    Elemental::Finalize();
    return 0;
}

