// Distributed under the MIT License.
// See LICENSE.txt for details.

#pragma once

#include <cstddef>

#include "DataStructures/Tensor/TypeAliases.hpp"

namespace TestHelpers {
namespace Schwarzschild {
/*!
 * \ingroup TestingFrameworkGroup
 * \brief Schwarzschild (Kerr-Schild) spatial ricci tensor
 *
 * \details
 * Computes \f$R_{ij} = M \frac{r^2(4M+r)\delta_{ij}-(8M+3r)x_i x_j}
 * {r^4(2M+r^2)},\f$
 * where \f$r = x_i x_j \delta^{ij}\f$, \f$x_i\f$ is the
 * position vector in Cartesian coordinates, and M is the mass.
 */
template <size_t SpatialDim, typename Frame, typename DataType>
tnsr::ii<DataType, SpatialDim, Frame> spatial_ricci(
    const tnsr::I<DataType, SpatialDim, Frame>& x, const double& mass) noexcept;
} //namespace Schwarzschild

namespace Minkowski {
/*!
 * \ingroup TestingFrameworkGroup
 * \brief Extrinsic curvature of 2D sphere in 3D flat space
 *
 * \details
 * Computes \f$K_{ij} = \frac{1}{r}\left(\delta_{ij} -
 * \frac{x_i x_j}{r}\right),\f$
 * where \f$r = x_i x_j \delta^{ij}\f$ and \f$x_i\f$ is the
 * position vector in Cartesian coordinates.
 */
template <size_t SpatialDim, typename Frame, typename DataType>
tnsr::ii<DataType, SpatialDim, Frame> extrinsic_curvature_sphere(
    const tnsr::I<DataType, SpatialDim, Frame>& x) noexcept;
} //namespace Minkowski
} //namespace TestHelpers
