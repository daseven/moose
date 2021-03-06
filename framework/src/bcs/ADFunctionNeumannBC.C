//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFunctionNeumannBC.h"

#include "Function.h"

registerADMooseObject("MooseApp", ADFunctionNeumannBC);

defineADLegacyParams(ADFunctionNeumannBC);

template <ComputeStage compute_stage>
InputParameters
ADFunctionNeumannBC<compute_stage>::validParams()
{
  InputParameters params = ADIntegratedBC<compute_stage>::validParams();
  params.addRequiredParam<FunctionName>("function", "The function.");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=h(t,\\vec{x})$, "
                             "where $h$ is a (possibly) time and space-dependent MOOSE Function.");
  return params;
}

template <ComputeStage compute_stage>
ADFunctionNeumannBC<compute_stage>::ADFunctionNeumannBC(const InputParameters & parameters)
  : ADIntegratedBC<compute_stage>(parameters), _func(getFunction("function"))
{
}

template <ComputeStage compute_stage>
ADReal
ADFunctionNeumannBC<compute_stage>::computeQpResidual()
{
  return -_test[_i][_qp] * _func.value(_t, _q_point[_qp]);
}
