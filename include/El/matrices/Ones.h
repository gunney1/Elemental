/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_ONES_CINT_H
#define EL_ONES_CINT_H

#ifdef __cplusplus
extern "C" {
#endif

void ElOnes_s( ElMatrix_s* A, Int m, Int n );
void ElOnes_d( ElMatrix_d* A, Int m, Int n );
void ElOnes_c( ElMatrix_c* A, Int m, Int n );
void ElOnes_z( ElMatrix_z* A, Int m, Int n );

void ElOnes_s( ElDistMatrix_s* A, Int m, Int n );
void ElOnes_d( ElDistMatrix_d* A, Int m, Int n );
void ElOnes_c( ElDistMatrix_c* A, Int m, Int n );
void ElOnes_z( ElDistMatrix_z* A, Int m, Int n );

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef EL_ONES_CINT_H
