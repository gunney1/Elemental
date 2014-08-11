/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/

namespace El {
namespace trsm {

// Right Lower (Conjugate)Transpose (Non)Unit Trsm
//   X := X tril(L)^-T,
//   X := X tril(L)^-H,
//   X := X trilu(L)^-T, or
//   X := X trilu(L)^-H
template<typename F>
inline void
RLT
( Orientation orientation, UnitOrNonUnit diag,
  const AbstractDistMatrix<F>& LPre, AbstractDistMatrix<F>& XPre, 
  bool checkIfSingular )
{
    DEBUG_ONLY(
        CallStackEntry cse("trsm::RLT");
        if( orientation == NORMAL )
            LogicError("Expected (Conjugate)Transpose option");
    )
    const Int m = XPre.Height();
    const Int n = XPre.Width();
    const Int bsize = Blocksize();
    const Grid& g = LPre.Grid();

    DistMatrix<F> L(g), X(g);
    Copy( LPre, L, READ_PROXY );
    Copy( XPre, X, READ_WRITE_PROXY );

    DistMatrix<F,STAR,STAR> L11_STAR_STAR(g);
    DistMatrix<F,VR,  STAR> L21_VR_STAR(g);
    DistMatrix<F,STAR,MR  > L21Trans_STAR_MR(g);
    DistMatrix<F,VC,  STAR> X1_VC_STAR(g);
    DistMatrix<F,STAR,MC  > X1Trans_STAR_MC(g);

    const IndexRange outerInd( 0, m );

    for( Int k=0; k<n; k+=bsize )
    {
        const Int nb = Min(bsize,n-k);

        const IndexRange ind1( k,    k+nb );
        const IndexRange ind2( k+nb, n    );

        auto L11 = LockedView( L, ind1, ind1 );
        auto L21 = LockedView( L, ind2, ind1 );

        auto X1 = View( X, outerInd, ind1 );
        auto X2 = View( X, outerInd, ind2 );

        L11_STAR_STAR = L11; 
        X1_VC_STAR.AlignWith( X2 );
        X1_VC_STAR = X1;  
        
        LocalTrsm
        ( RIGHT, LOWER, orientation, diag, 
          F(1), L11_STAR_STAR, X1_VC_STAR, checkIfSingular );

        X1Trans_STAR_MC.AlignWith( X2 );
        X1_VC_STAR.TransposePartialColAllGather( X1Trans_STAR_MC );
        X1.TransposeRowFilterFrom( X1Trans_STAR_MC );
        L21_VR_STAR.AlignWith( X2 );
        L21_VR_STAR = L21;
        L21Trans_STAR_MR.AlignWith( X2 );
        L21_VR_STAR.TransposePartialColAllGather
        ( L21Trans_STAR_MR, (orientation==ADJOINT) ); 

        // X2[MC,MR] -= X1[MC,*] (L21[MR,*])^(T/H)
        //            = X1^T[* ,MC] (L21^(T/H))[*,MR]
        LocalGemm
        ( TRANSPOSE, NORMAL, 
          F(-1), X1Trans_STAR_MC, L21Trans_STAR_MR, F(1), X2 );
    }
    Copy( X, XPre, RESTORE_READ_WRITE_PROXY );
}

} // namespace trsm
} // namespace El
