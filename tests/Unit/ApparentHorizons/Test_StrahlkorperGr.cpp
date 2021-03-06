// Distributed under the MIT License.
// See LICENSE.txt for details.

#include <catch.hpp>

#include "ApparentHorizons/Strahlkorper.hpp"
#include "ApparentHorizons/StrahlkorperDataBox.hpp"
#include "ApparentHorizons/StrahlkorperGr.hpp"
#include "DataStructures/DataBox/DataBox.hpp"
#include "DataStructures/Tensor/EagerMath/DeterminantAndInverse.hpp"
#include "DataStructures/Tensor/EagerMath/Magnitude.hpp"
#include "PointwiseFunctions/AnalyticSolutions/EinsteinSolutions/KerrSchild.hpp"
#include "PointwiseFunctions/AnalyticSolutions/EinsteinSolutions/Minkowski.hpp"
#include "PointwiseFunctions/GeneralRelativity/Christoffel.hpp"
#include "PointwiseFunctions/GeneralRelativity/ComputeSpacetimeQuantities.hpp"
#include "PointwiseFunctions/GeneralRelativity/GrTags.hpp"
#include "PointwiseFunctions/GeneralRelativity/IndexManipulation.hpp"
#include "Utilities/MakeWithValue.hpp"
#include "tests/Unit/ApparentHorizons/StrahlkorperGrTestHelpers.hpp"
#include "tests/Unit/TestHelpers.hpp"
#include "tests/Unit/TestingFramework.hpp"

namespace {
namespace TestExpansion {
void test_schwarzschild() {
  // Make surface of radius 2.
  const auto box =
      db::create<db::AddTags<StrahlkorperTags::items_tags<Frame::Inertial>>,
                 db::AddComputeItemsTags<
                     StrahlkorperTags::compute_items_tags<Frame::Inertial>>>(
          Strahlkorper<Frame::Inertial>(8, 8, 2.0, {{0.0, 0.0, 0.0}}));

  const double t = 0.0;
  const auto& cart_coords =
      db::get<StrahlkorperTags::CartesianCoords<Frame::Inertial>>(box);

  // Schwarzschild solution
  const double mass = 1.0;
  const std::array<double, 3> spin{{0.0, 0.0, 0.0}};
  const std::array<double, 3> center{{0.0, 0.0, 0.0}};
  EinsteinSolutions::KerrSchild solution(mass, spin, center);
  const auto vars = solution.solution(cart_coords, t);

  const auto& spatial_metric =
      get<gr::Tags::SpatialMetric<3, Frame::Inertial, DataVector>>(vars);
  const auto& deriv_spatial_metric =
      get<EinsteinSolutions::KerrSchild::deriv_spatial_metric<DataVector>>(
          vars);
  const auto inverse_spatial_metric =
      determinant_and_inverse(spatial_metric).second;

  const DataVector one_over_one_form_magnitude =
      1.0 / get(magnitude(
                db::get<StrahlkorperTags::NormalOneForm<Frame::Inertial>>(box),
                inverse_spatial_metric));
  const auto unit_normal_one_form = StrahlkorperGr::unit_normal_one_form(
      db::get<StrahlkorperTags::NormalOneForm<Frame::Inertial>>(box),
      one_over_one_form_magnitude);
  const auto grad_unit_normal_one_form =
      StrahlkorperGr::grad_unit_normal_one_form(
          db::get<StrahlkorperTags::Rhat<Frame::Inertial>>(box),
          db::get<StrahlkorperTags::Radius<Frame::Inertial>>(box),
          unit_normal_one_form,
          db::get<StrahlkorperTags::D2xRadius<Frame::Inertial>>(box),
          one_over_one_form_magnitude,
          raise_or_lower_first_index(
              gr::christoffel_first_kind(deriv_spatial_metric),
              inverse_spatial_metric));
  const auto inverse_surface_metric = StrahlkorperGr::inverse_surface_metric(
      raise_or_lower_index(unit_normal_one_form, inverse_spatial_metric),
      inverse_spatial_metric);

  const auto residual = StrahlkorperGr::expansion(
      grad_unit_normal_one_form, inverse_surface_metric,
      gr::extrinsic_curvature(
          get<gr::Tags::Lapse<3, Frame::Inertial, DataVector>>(vars),
          get<gr::Tags::Shift<3, Frame::Inertial, DataVector>>(vars),
          get<EinsteinSolutions::KerrSchild::deriv_shift<DataVector>>(vars),
          spatial_metric,
          get<gr::Tags::DtSpatialMetric<3, Frame::Inertial, DataVector>>(vars),
          deriv_spatial_metric));

  Approx custom_approx = Approx::custom().epsilon(1.e-12).scale(1.0);
  CHECK_ITERABLE_CUSTOM_APPROX(
      get(residual), DataVector(get(residual).size(), 0.0), custom_approx);
}

void test_minkowski() {
  // Make surface of radius 2.
  const auto box =
      db::create<db::AddTags<StrahlkorperTags::items_tags<Frame::Inertial>>,
                 db::AddComputeItemsTags<
                     StrahlkorperTags::compute_items_tags<Frame::Inertial>>>(
          Strahlkorper<Frame::Inertial>(8, 8, 2.0, {{0.0, 0.0, 0.0}}));

  const double t = 0.0;
  const auto& cart_coords =
      db::get<StrahlkorperTags::CartesianCoords<Frame::Inertial>>(box);

  EinsteinSolutions::Minkowski<3> solution{};

  const auto deriv_spatial_metric =
      solution.deriv_spatial_metric(cart_coords, t);
  const auto inverse_spatial_metric =
      solution.inverse_spatial_metric(cart_coords, t);

  const DataVector one_over_one_form_magnitude =
      1.0 / get(magnitude(
                db::get<StrahlkorperTags::NormalOneForm<Frame::Inertial>>(box),
                inverse_spatial_metric));
  const auto unit_normal_one_form = StrahlkorperGr::unit_normal_one_form(
      db::get<StrahlkorperTags::NormalOneForm<Frame::Inertial>>(box),
      one_over_one_form_magnitude);
  const auto grad_unit_normal_one_form =
      StrahlkorperGr::grad_unit_normal_one_form(
          db::get<StrahlkorperTags::Rhat<Frame::Inertial>>(box),
          db::get<StrahlkorperTags::Radius<Frame::Inertial>>(box),
          unit_normal_one_form,
          db::get<StrahlkorperTags::D2xRadius<Frame::Inertial>>(box),
          one_over_one_form_magnitude,
          raise_or_lower_first_index(
              gr::christoffel_first_kind(deriv_spatial_metric),
              inverse_spatial_metric));
  const auto inverse_surface_metric = StrahlkorperGr::inverse_surface_metric(
      raise_or_lower_index(unit_normal_one_form, inverse_spatial_metric),
      inverse_spatial_metric);

  const auto residual = StrahlkorperGr::expansion(
      grad_unit_normal_one_form, inverse_surface_metric,
      solution.extrinsic_curvature(cart_coords, t));

  Approx custom_approx = Approx::custom().epsilon(1.e-12);
  CHECK_ITERABLE_CUSTOM_APPROX(
      get(residual), DataVector(get(residual).size(), 1.0), custom_approx);
}
}  // namespace TestExpansion

namespace TestExtrinsicCurvature {
void test_minkowski() {
  // Make surface of radius 2.
  const auto box =
      db::create<db::AddTags<StrahlkorperTags::items_tags<Frame::Inertial>>,
                 db::AddComputeItemsTags<
                     StrahlkorperTags::compute_items_tags<Frame::Inertial>>>(
          Strahlkorper<Frame::Inertial>(8, 8, 2.0, {{0.0, 0.0, 0.0}}));

  const double t = 0.0;
  const auto& cart_coords =
      db::get<StrahlkorperTags::CartesianCoords<Frame::Inertial>>(box);

  EinsteinSolutions::Minkowski<3> solution{};

  const auto deriv_spatial_metric =
      solution.deriv_spatial_metric(cart_coords, t);
  const auto inverse_spatial_metric =
      solution.inverse_spatial_metric(cart_coords, t);

  const DataVector one_over_one_form_magnitude =
      1.0 / get(magnitude(
                db::get<StrahlkorperTags::NormalOneForm<Frame::Inertial>>(box),
                inverse_spatial_metric));
  const auto unit_normal_one_form = StrahlkorperGr::unit_normal_one_form(
      db::get<StrahlkorperTags::NormalOneForm<Frame::Inertial>>(box),
      one_over_one_form_magnitude);
  const auto grad_unit_normal_one_form =
      StrahlkorperGr::grad_unit_normal_one_form(
          db::get<StrahlkorperTags::Rhat<Frame::Inertial>>(box),
          db::get<StrahlkorperTags::Radius<Frame::Inertial>>(box),
          unit_normal_one_form,
          db::get<StrahlkorperTags::D2xRadius<Frame::Inertial>>(box),
          one_over_one_form_magnitude,
          raise_or_lower_first_index(
              gr::christoffel_first_kind(deriv_spatial_metric),
              inverse_spatial_metric));

  const auto unit_normal_vector =
      raise_or_lower_index(unit_normal_one_form, inverse_spatial_metric);
  const auto extrinsic_curvature = StrahlkorperGr::extrinsic_curvature(
      grad_unit_normal_one_form, unit_normal_one_form, unit_normal_vector);
  const auto extrinsic_curvature_minkowski =
      TestHelpers::Minkowski::extrinsic_curvature_sphere(cart_coords);

  CHECK_ITERABLE_APPROX(extrinsic_curvature, extrinsic_curvature_minkowski);
}
}  // namespace TestExtrinsicCurvature

namespace TestRicciScalar {
void test_schwarzschild() {
  // Make surface of radius 2.
  const auto box =
      db::create<db::AddTags<StrahlkorperTags::items_tags<Frame::Inertial>>,
                 db::AddComputeItemsTags<
                     StrahlkorperTags::compute_items_tags<Frame::Inertial>>>(
          Strahlkorper<Frame::Inertial>(8, 8, 2.0, {{0.0, 0.0, 0.0}}));

  const double t = 0.0;
  const auto& cart_coords =
      db::get<StrahlkorperTags::CartesianCoords<Frame::Inertial>>(box);

  // Schwarzschild solution
  const double mass = 1.0;
  const std::array<double, 3> spin{{0.0, 0.0, 0.0}};
  const std::array<double, 3> center{{0.0, 0.0, 0.0}};
  EinsteinSolutions::KerrSchild solution(mass, spin, center);
  const auto vars = solution.solution(cart_coords, t);

  const auto& spatial_metric =
      get<gr::Tags::SpatialMetric<3, Frame::Inertial, DataVector>>(vars);
  const auto& deriv_spatial_metric =
      get<EinsteinSolutions::KerrSchild::deriv_spatial_metric<DataVector>>(
          vars);
  const auto inverse_spatial_metric =
      determinant_and_inverse(spatial_metric).second;

  const DataVector one_over_one_form_magnitude =
      1.0 / get(magnitude(
                db::get<StrahlkorperTags::NormalOneForm<Frame::Inertial>>(box),
                inverse_spatial_metric));
  const auto unit_normal_one_form = StrahlkorperGr::unit_normal_one_form(
      db::get<StrahlkorperTags::NormalOneForm<Frame::Inertial>>(box),
      one_over_one_form_magnitude);
  const auto grad_unit_normal_one_form =
      StrahlkorperGr::grad_unit_normal_one_form(
          db::get<StrahlkorperTags::Rhat<Frame::Inertial>>(box),
          db::get<StrahlkorperTags::Radius<Frame::Inertial>>(box),
          unit_normal_one_form,
          db::get<StrahlkorperTags::D2xRadius<Frame::Inertial>>(box),
          one_over_one_form_magnitude,
          raise_or_lower_first_index(
              gr::christoffel_first_kind(deriv_spatial_metric),
              inverse_spatial_metric));

  const auto unit_normal_vector =
      raise_or_lower_index(unit_normal_one_form, inverse_spatial_metric);
  const auto ricci_scalar = StrahlkorperGr::ricci_scalar(
      TestHelpers::Schwarzschild::spatial_ricci(cart_coords, mass),
      unit_normal_vector,
      StrahlkorperGr::extrinsic_curvature(
          grad_unit_normal_one_form, unit_normal_one_form, unit_normal_vector),
      inverse_spatial_metric);

  CHECK_ITERABLE_APPROX(get(ricci_scalar), DataVector(get(ricci_scalar).size(),
                                                      0.5 / square(mass)));
}

void test_minkowski() {
  // Make surface of radius 2.
  const auto box =
      db::create<db::AddTags<StrahlkorperTags::items_tags<Frame::Inertial>>,
                 db::AddComputeItemsTags<
                     StrahlkorperTags::compute_items_tags<Frame::Inertial>>>(
          Strahlkorper<Frame::Inertial>(8, 8, 2.0, {{0.0, 0.0, 0.0}}));

  const double t = 0.0;
  const auto& cart_coords =
      db::get<StrahlkorperTags::CartesianCoords<Frame::Inertial>>(box);

  EinsteinSolutions::Minkowski<3> solution{};

  const auto deriv_spatial_metric =
      solution.deriv_spatial_metric(cart_coords, t);
  const auto inverse_spatial_metric =
      solution.inverse_spatial_metric(cart_coords, t);

  const DataVector one_over_one_form_magnitude =
      1.0 / get(magnitude(
                db::get<StrahlkorperTags::NormalOneForm<Frame::Inertial>>(box),
                inverse_spatial_metric));
  const auto unit_normal_one_form = StrahlkorperGr::unit_normal_one_form(
      db::get<StrahlkorperTags::NormalOneForm<Frame::Inertial>>(box),
      one_over_one_form_magnitude);
  const auto grad_unit_normal_one_form =
      StrahlkorperGr::grad_unit_normal_one_form(
          db::get<StrahlkorperTags::Rhat<Frame::Inertial>>(box),
          db::get<StrahlkorperTags::Radius<Frame::Inertial>>(box),
          unit_normal_one_form,
          db::get<StrahlkorperTags::D2xRadius<Frame::Inertial>>(box),
          one_over_one_form_magnitude,
          raise_or_lower_first_index(
              gr::christoffel_first_kind(deriv_spatial_metric),
              inverse_spatial_metric));

  const auto unit_normal_vector =
      raise_or_lower_index(unit_normal_one_form, inverse_spatial_metric);
  const auto extrinsic_curvature = StrahlkorperGr::extrinsic_curvature(
      grad_unit_normal_one_form, unit_normal_one_form, unit_normal_vector);

  const auto ricci_scalar = StrahlkorperGr::ricci_scalar(
      make_with_value<tnsr::ii<DataVector, 3, Frame::Inertial>>(
          inverse_spatial_metric, 0.0),
      unit_normal_vector, extrinsic_curvature, inverse_spatial_metric);

  CHECK_ITERABLE_APPROX(get(ricci_scalar),
                        DataVector(get(ricci_scalar).size(), 0.5));
}
}  // namespace TestRicciScalar
}  // namespace

SPECTRE_TEST_CASE("Unit.ApparentHorizons.StrahlkorperGr.Expansion",
                  "[ApparentHorizons][Unit]") {
  TestExpansion::test_schwarzschild();
  TestExpansion::test_minkowski();
}

SPECTRE_TEST_CASE("Unit.ApparentHorizons.StrahlkorperGr.ExtrinsicCurvature",
                  "[ApparentHorizons][Unit]") {
  // N.B.: test_minkowski() fully tests the extrinsic curvature function.
  // All components of extrinsic curvature of a sphere in flat space
  // are nontrivial; cf. extrinsic_curvature_sphere()
  // in StrahlkorperGrTestHelpers.cpp).
  TestExtrinsicCurvature::test_minkowski();
}

SPECTRE_TEST_CASE("Unit.ApparentHorizons.StrahlkorperGr.RicciScalar",
                  "[ApparentHorizons][Unit]") {
  TestRicciScalar::test_schwarzschild();
  TestRicciScalar::test_minkowski();
}
