//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoostLognormalDistribution.h"

#include "libmesh/auto_ptr.h"

registerMooseObjectAliased("StochasticToolsApp", BoostLognormalDistribution, "BoostLognormal");
registerMooseObjectReplaced("StochasticToolsApp",
                            BoostLognormalDistribution,
                            "07/01/2020 00:00",
                            BoostLognormal);

defineLegacyParams(BoostLognormalDistribution);

InputParameters
BoostLognormalDistribution::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Boost Lognormal distribution.");
  params.addRequiredParam<Real>("location", "The Lognormal location parameter.");
  params.addRequiredParam<Real>("scale", "The Lognormal scale parameter.");
  return params;
}

BoostLognormalDistribution::BoostLognormalDistribution(const InputParameters & parameters)
  : BoostDistribution<boost::math::lognormal_distribution<Real>>(parameters)
{
  _distribution_unique_ptr = libmesh_make_unique<boost::math::lognormal_distribution<Real>>(
      getParam<Real>("location"), getParam<Real>("scale"));
}
