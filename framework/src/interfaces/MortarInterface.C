//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE Includes
#include "MortarInterface.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MortarData.h"

#include <algorithm>

InputParameters
MortarInterface::validParams()
{
  // Create InputParameters object that will be appended to the parameters for the inheriting object
  InputParameters params = emptyInputParameters();

  params.addRequiredParam<BoundaryName>("master_boundary",
                                        "The name of the master boundary sideset.");
  params.addRequiredParam<BoundaryName>("slave_boundary",
                                        "The name of the slave boundary sideset.");
  params.addRequiredParam<SubdomainName>("master_subdomain", "The name of the master subdomain.");
  params.addRequiredParam<SubdomainName>("slave_subdomain", "The name of the slave subdomain.");
  params.addParam<bool>(
      "periodic",
      false,
      "Whether this constraint is going to be used to enforce a periodic condition. This has the "
      "effect of changing the normals vector for projection from outward to inward facing");

  return params;
}

// Standard constructor
MortarInterface::MortarInterface(const MooseObject * moose_object)
  : _moi_problem(*moose_object->getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _moi_mesh(_moi_problem.mesh()),
    _mortar_data(_moi_problem.mortarData()),
    _slave_id(_moi_mesh.getBoundaryID(moose_object->getParamTempl<BoundaryName>("slave_boundary"))),
    _master_id(
        _moi_mesh.getBoundaryID(moose_object->getParamTempl<BoundaryName>("master_boundary"))),
    _slave_subdomain_id(
        _moi_mesh.getSubdomainID(moose_object->getParamTempl<SubdomainName>("slave_subdomain"))),
    _master_subdomain_id(
        _moi_mesh.getSubdomainID(moose_object->getParamTempl<SubdomainName>("master_subdomain")))
{
  if (_moi_mesh.dimension() == 3)
    mooseError("Mortar cannot currently be run in three dimensions. It's on the to-do list!");

  // Create the mortar interface if it hasn't already been created
  _moi_problem.createMortarInterface(std::make_pair(_master_id, _slave_id),
                                     std::make_pair(_master_subdomain_id, _slave_subdomain_id),
                                     moose_object->isParamValid("use_displaced_mesh")
                                         ? moose_object->getParamTempl<bool>("use_displaced_mesh")
                                         : false,
                                     moose_object->getParamTempl<bool>("periodic"));

  const auto & slave_set = _mortar_data.getHigherDimSubdomainIDs(_slave_subdomain_id);
  const auto & master_set = _mortar_data.getHigherDimSubdomainIDs(_master_subdomain_id);

  std::set_union(slave_set.begin(),
                 slave_set.end(),
                 master_set.begin(),
                 master_set.end(),
                 std::inserter(_higher_dim_subdomain_ids, _higher_dim_subdomain_ids.begin()));
  _boundary_ids = {_slave_id, _master_id};
}
