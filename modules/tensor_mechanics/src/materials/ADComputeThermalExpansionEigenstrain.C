//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeThermalExpansionEigenstrain.h"

registerADMooseObject("TensorMechanicsApp", ADComputeThermalExpansionEigenstrain);

defineADLegacyParams(ADComputeThermalExpansionEigenstrain);

template <ComputeStage compute_stage>
InputParameters
ADComputeThermalExpansionEigenstrain<compute_stage>::validParams()
{
  InputParameters params = ADComputeThermalExpansionEigenstrainBase<compute_stage>::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion "
                             "with a constant coefficient");
  params.addRequiredParam<Real>("thermal_expansion_coeff", "Thermal expansion coefficient");
  return params;
}

template <ComputeStage compute_stage>
ADComputeThermalExpansionEigenstrain<compute_stage>::ADComputeThermalExpansionEigenstrain(
    const InputParameters & parameters)
  : ADComputeThermalExpansionEigenstrainBase<compute_stage>(parameters),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff"))
{
}

template <ComputeStage compute_stage>
void
ADComputeThermalExpansionEigenstrain<compute_stage>::computeThermalStrain(ADReal & thermal_strain)
{
  thermal_strain = _thermal_expansion_coeff * (_temperature[_qp] - _stress_free_temperature[_qp]);
}
