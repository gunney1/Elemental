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
#ifndef ELEMENTAL_ENVIRONMENT_H 
#define ELEMENTAL_ENVIRONMENT_H 1

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>
#include "mpi.h"

namespace Elemental
{
#ifndef RELEASE
    void PushCallStack( std::string );
    void PopCallStack();
    void DumpCallStack();
#endif
}

#include "ElementalMemory.h"
#include "ElementalGrid.h"
#include "ElementalRandom.h"
#include "ElementalTypes.h"
#include "ElementalUtilities.h"

namespace Elemental
{
    void Init( int* argc, char** argv[] );
    void Finalize();

    int Blocksize();
    void SetBlocksize( int blocksize );

    void PushBlocksizeStack( int blocksize );
    void PopBlocksizeStack();
}

#endif /* ELEMENTAL_ENVIRONMENT_H */

