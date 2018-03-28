/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#include <El-lite.hpp>
#include <El/blas_like.hpp>

#define COLDIST STAR
#define ROWDIST VR

#include "./setup.hpp"

namespace El {

// Public section
// ##############

// Assignment and reconfiguration
// ==============================

// Make a copy
// -----------
template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,MC,MR,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    copy::RowAllToAllDemote(A, *this);
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,MC,STAR,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    DistMatrix<T,MC,MR,ELEMENT,D> A_MC_MR(A);
    *this = A_MC_MR;
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,STAR,MR,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    copy::PartialRowFilter(A, *this);
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,MD,STAR,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    // TODO: More efficient implementation
    copy::GeneralPurpose(A, *this);
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,STAR,MD,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    // TODO: More efficient implementation
    copy::GeneralPurpose(A, *this);
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,MR,MC,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    DistMatrix<T,STAR,VC,ELEMENT,D> A_STAR_VC(A);
    *this = A_STAR_VC;
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,MR,STAR,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    DistMatrix<T,MR,MC,ELEMENT,D> A_MR_MC(A);
    DistMatrix<T,STAR,VC,ELEMENT,D> A_STAR_VC(A_MR_MC);
    A_MR_MC.Empty();
    *this = A_STAR_VC;
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,STAR,MC,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    DistMatrix<T,STAR,VC,ELEMENT,D> A_STAR_VC(A);
    *this = A_STAR_VC;
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,VC,STAR,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    DistMatrix<T,MC,MR,ELEMENT,D> A_MC_MR(A);
    *this = A_MC_MR;
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,STAR,VC,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    copy::RowwiseVectorExchange<T,MC,MR>(A, *this);
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,VR,STAR,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    DistMatrix<T,MR,MC,ELEMENT,D> A_MR_MC(A);
    DistMatrix<T,STAR,VC,ELEMENT,D> A_STAR_VC(A_MR_MC);
    A_MR_MC.Empty();
    *this = A_STAR_VC;
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,STAR,STAR,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    copy::RowFilter(A, *this);
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const DistMatrix<T,CIRC,CIRC,ELEMENT,D>& A)
{
    EL_DEBUG_CSE
    copy::Scatter(A, *this);
    return *this;
}

template <typename T, Device D>
DM& DM::operator=(const ElementalMatrix<T>& A)
{
    EL_DEBUG_CSE
    #define GUARD(CDIST,RDIST,WRAP,DEVICE) \
      A.DistData().colDist == CDIST && A.DistData().rowDist == RDIST && \
      ELEMENT == WRAP && A.GetLocalDevice() == DEVICE
    #define PAYLOAD(CDIST,RDIST,WRAP,DEVICE) \
      auto& ACast = static_cast<const DistMatrix<T,CDIST,RDIST,ELEMENT,DEVICE>&>(A); \
      *this = ACast;
    #include "El/macros/DeviceGuardAndPayload.h"
    return *this;
}

// Basic queries
// =============
template <typename T, Device D>
mpi::Comm DM::DistComm() const EL_NO_EXCEPT
{ return this->Grid().VRComm(); }
template <typename T, Device D>
mpi::Comm DM::CrossComm() const EL_NO_EXCEPT
{ return (this->Grid().InGrid() ? mpi::COMM_SELF : mpi::COMM_NULL); }
template <typename T, Device D>
mpi::Comm DM::RedundantComm() const EL_NO_EXCEPT
{ return (this->Grid().InGrid() ? mpi::COMM_SELF : mpi::COMM_NULL); }

template <typename T, Device D>
mpi::Comm DM::ColComm() const EL_NO_EXCEPT
{ return (this->Grid().InGrid() ? mpi::COMM_SELF : mpi::COMM_NULL); }
template <typename T, Device D>
mpi::Comm DM::RowComm() const EL_NO_EXCEPT
{ return this->Grid().VRComm(); }

template <typename T, Device D>
mpi::Comm DM::PartialRowComm() const EL_NO_EXCEPT
{ return this->Grid().MRComm(); }
template <typename T, Device D>
mpi::Comm DM::PartialUnionRowComm() const EL_NO_EXCEPT
{ return this->Grid().MCComm(); }

template <typename T, Device D>
mpi::Comm DM::PartialColComm() const EL_NO_EXCEPT
{ return this->ColComm(); }
template <typename T, Device D>
mpi::Comm DM::PartialUnionColComm() const EL_NO_EXCEPT
{ return (this->Grid().InGrid() ? mpi::COMM_SELF : mpi::COMM_NULL); }

template <typename T, Device D>
int DM::ColStride() const EL_NO_EXCEPT { return 1; }
template <typename T, Device D>
int DM::RowStride() const EL_NO_EXCEPT { return this->Grid().VRSize(); }

template <typename T, Device D>
int DM::DistSize() const EL_NO_EXCEPT { return this->Grid().VRSize(); }
template <typename T, Device D>
int DM::CrossSize() const EL_NO_EXCEPT { return 1; }
template <typename T, Device D>
int DM::RedundantSize() const EL_NO_EXCEPT { return 1; }

template <typename T, Device D>
int DM::PartialRowStride() const EL_NO_EXCEPT
{ return this->Grid().MRSize(); }
template <typename T, Device D>
int DM::PartialUnionRowStride() const EL_NO_EXCEPT
{ return this->Grid().MCSize(); }

template <typename T, Device D>
int DM::PartialColStride() const EL_NO_EXCEPT { return this->ColStride(); }
template <typename T, Device D>
int DM::PartialUnionColStride() const EL_NO_EXCEPT { return 1; }

template <typename T, Device D>
int DM::ColRank() const EL_NO_EXCEPT
{ return (this->Grid().InGrid() ? 0 : mpi::UNDEFINED); }
template <typename T, Device D>
int DM::RowRank() const EL_NO_EXCEPT { return this->Grid().VRRank(); }

template <typename T, Device D>
int DM::DistRank() const EL_NO_EXCEPT { return this->Grid().VRRank(); }
template <typename T, Device D>
int DM::CrossRank() const EL_NO_EXCEPT
{ return (this->Grid().InGrid() ? 0 : mpi::UNDEFINED); }
template <typename T, Device D>
int DM::RedundantRank() const EL_NO_EXCEPT
{ return (this->Grid().InGrid() ? 0 : mpi::UNDEFINED); }

template <typename T, Device D>
int DM::PartialRowRank() const EL_NO_EXCEPT
{ return this->Grid().MRRank(); }
template <typename T, Device D>
int DM::PartialUnionRowRank() const EL_NO_EXCEPT
{ return this->Grid().MCRank(); }

template <typename T, Device D>
int DM::PartialColRank() const EL_NO_EXCEPT
{ return this->ColRank(); }
template <typename T, Device D>
int DM::PartialUnionColRank() const EL_NO_EXCEPT
{ return (this->Grid().InGrid() ? 0 : mpi::UNDEFINED); }

// Instantiate {Int,Real,Complex<Real>} for each Real in {float,double}
// ####################################################################

#define SELF(T,U,V,D) \
  template DistMatrix<T,COLDIST,ROWDIST,ELEMENT,D>::DistMatrix \
  (const DistMatrix<T,U,V,ELEMENT,D>& A)
#define OTHER(T,U,V,D) \
  template DistMatrix<T,COLDIST,ROWDIST,ELEMENT,D>::DistMatrix \
  (const DistMatrix<T,U,V,BLOCK,D>& A); \
  template DistMatrix<T,COLDIST,ROWDIST,ELEMENT,D>& \
           DistMatrix<T,COLDIST,ROWDIST,ELEMENT,D>::operator= \
           (const DistMatrix<T,U,V,BLOCK,D>& A)
#define BOTH(T,U,V,D) \
    SELF(T,U,V,D);    \
  OTHER(T,U,V,D)
#define PROTO(T) \
    template class DistMatrix<T,COLDIST,ROWDIST,ELEMENT,Device::CPU>;      \
  BOTH(T,CIRC,CIRC,Device::CPU); \
  BOTH(T,MC,  MR ,Device::CPU); \
  BOTH(T,MC,  STAR,Device::CPU); \
  BOTH(T,MD,  STAR,Device::CPU); \
  BOTH(T,MR,  MC ,Device::CPU); \
  BOTH(T,MR,  STAR,Device::CPU); \
  BOTH(T,STAR,MC ,Device::CPU); \
  BOTH(T,STAR,MD ,Device::CPU); \
  BOTH(T,STAR,MR ,Device::CPU); \
  BOTH(T,STAR,STAR,Device::CPU); \
  BOTH(T,STAR,VC ,Device::CPU); \
  OTHER(T,STAR,VR ,Device::CPU); \
  BOTH(T,VC,  STAR,Device::CPU); \
  BOTH(T,VR,  STAR,Device::CPU);

template class DistMatrix<float,COLDIST,ROWDIST,ELEMENT,Device::GPU>;
SELF(float,CIRC,CIRC,Device::GPU);
SELF(float,MC,  MR ,Device::GPU);
SELF(float,MC,  STAR,Device::GPU);
SELF(float,MD,  STAR,Device::GPU);
SELF(float,MR,  MC ,Device::GPU);
SELF(float,MR,  STAR,Device::GPU);
SELF(float,STAR,MC ,Device::GPU);
SELF(float,STAR,MD ,Device::GPU);
SELF(float,STAR,MR ,Device::GPU);
SELF(float,STAR,STAR,Device::GPU);
SELF(float,STAR,VC ,Device::GPU);
SELF(float,VC,  STAR,Device::GPU);
SELF(float,VR,  STAR,Device::GPU);

template class DistMatrix<double,COLDIST,ROWDIST,ELEMENT,Device::GPU>;
SELF(double,CIRC,CIRC,Device::GPU);
SELF(double,MC,  MR ,Device::GPU);
SELF(double,MC,  STAR,Device::GPU);
SELF(double,MD,  STAR,Device::GPU);
SELF(double,MR,  MC ,Device::GPU);
SELF(double,MR,  STAR,Device::GPU);
SELF(double,STAR,MC ,Device::GPU);
SELF(double,STAR,MD ,Device::GPU);
SELF(double,STAR,MR ,Device::GPU);
SELF(double,STAR,STAR,Device::GPU);
SELF(double,STAR,VC ,Device::GPU);
SELF(double,VC,  STAR,Device::GPU);
SELF(double,VR,  STAR,Device::GPU);

#define EL_ENABLE_DOUBLEDOUBLE
#define EL_ENABLE_QUADDOUBLE
#define EL_ENABLE_QUAD
#define EL_ENABLE_BIGINT
#define EL_ENABLE_BIGFLOAT
#include <El/macros/Instantiate.h>

} // namespace El
