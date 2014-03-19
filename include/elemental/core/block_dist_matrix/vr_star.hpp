/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_BLOCKDISTMATRIX_VR_STAR_DECL_HPP
#define ELEM_BLOCKDISTMATRIX_VR_STAR_DECL_HPP

namespace elem {

// Partial specialization to A[VR,* ].
//
// The columns of these distributed matrices are spread throughout the 
// process grid in a row-major fashion, while the rows are not 
// distributed.
template<typename T>
class BlockDistMatrix<T,VR,STAR> : public GeneralBlockDistMatrix<T,VR,STAR>
{
    // TODO
};

} // namespace elem

#endif // ifndef ELEM_BLOCKDISTMATRIX_VR_STAR_DECL_HPP