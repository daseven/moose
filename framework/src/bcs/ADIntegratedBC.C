//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADIntegratedBC.h"

// MOOSE includes
#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"

#include "libmesh/quadrature.h"

defineADLegacyParams(ADIntegratedBC);
defineADLegacyParams(ADVectorIntegratedBC);

template <typename T, ComputeStage compute_stage>
InputParameters
ADIntegratedBCTempl<T, compute_stage>::validParams()
{
  InputParameters params = IntegratedBCBase::validParams();
  return params;
}

template <typename T, ComputeStage compute_stage>
ADIntegratedBCTempl<T, compute_stage>::ADIntegratedBCTempl(const InputParameters & parameters)
  : IntegratedBCBase(parameters),
    MooseVariableInterface<T>(this,
                              false,
                              "variable",
                              Moose::VarKindType::VAR_NONLINEAR,
                              std::is_same<T, Real>::value ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                                           : Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(*this->mooseVariable()),
    _normals(_assembly.adNormals<compute_stage>()),
    _ad_q_points(_assembly.adQPointsFace<compute_stage>()),
    _test(_var.phiFace()),
    _grad_test(_var.template adGradPhiFace<compute_stage>()),
    _u(_var.template adSln<compute_stage>()),
    _grad_u(_var.template adGradSln<compute_stage>()),
    _ad_JxW(_assembly.adJxWFace<compute_stage>()),
    _ad_coord(_assembly.template adCoordTransformation<compute_stage>()),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh"))
{
  addMooseVariableDependency(this->mooseVariable());

  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getStandardVariable(_tid, _save_in_strings[i]);

    if (var->feType() != _var.feType())
      paramError(
          "save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));
    _save_in[i] = var;
    var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_save_in = _save_in.size() > 0;

  for (unsigned int i = 0; i < _diag_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getStandardVariable(_tid, _diag_save_in_strings[i]);

    if (var->feType() != _var.feType())
      paramError(
          "diag_save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));

    _diag_save_in[i] = var;
    var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_diag_save_in = _diag_save_in.size() > 0;
}

template <typename T, ComputeStage compute_stage>
void
ADIntegratedBCTempl<T, compute_stage>::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test.size(); _i++)
        _local_re(_i) += _ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual();
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test.size(); _i++)
        _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

template <>
void
ADIntegratedBCTempl<Real, JACOBIAN>::computeResidual()
{
}

template <>
void
ADIntegratedBCTempl<RealVectorValue, JACOBIAN>::computeResidual()
{
}

template <typename T, ComputeStage compute_stage>
void
ADIntegratedBCTempl<T, compute_stage>::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  size_t ad_offset = _var.number() * _sys.getMaxVarNDofsPerElem();

  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test.size(); _i++)
      {
        DualReal residual = _ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual();
        for (_j = 0; _j < _var.phiSize(); ++_j)
          _local_ke(_i, _j) += residual.derivatives()[ad_offset + _j];
      }
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test.size(); _i++)
      {
        DualReal residual = _JxW[_qp] * _coord[_qp] * computeQpResidual();
        for (_j = 0; _j < _var.phiSize(); ++_j)
          _local_ke(_i, _j) += residual.derivatives()[ad_offset + _j];
      }

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); i++)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

template <>
void
ADIntegratedBCTempl<Real, RESIDUAL>::computeJacobian()
{
}
template <>
void
ADIntegratedBCTempl<RealVectorValue, RESIDUAL>::computeJacobian()
{
}

template <typename T, ComputeStage compute_stage>
void
ADIntegratedBCTempl<T, compute_stage>::computeJacobianBlock(MooseVariableFEBase & jvar)
{
  auto jvar_num = jvar.number();
  auto phi_size = _sys.getVariable(_tid, jvar.number()).dofIndices().size();

  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar_num);
    mooseAssert(
        phi_size == ke.n(),
        "The size of the phi container does not match the number of local Jacobian columns");

    size_t ad_offset = jvar_num * _sys.getMaxVarNDofsPerElem();

    if (_use_displaced_mesh)
      for (_i = 0; _i < _test.size(); _i++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        {
          DualReal residual = _ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual();

          for (_j = 0; _j < phi_size; _j++)
            ke(_i, _j) += residual.derivatives()[ad_offset + _j];
        }
    else
      for (_i = 0; _i < _test.size(); _i++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        {
          DualReal residual = _JxW[_qp] * _coord[_qp] * computeQpResidual();

          for (_j = 0; _j < phi_size; _j++)
            ke(_i, _j) += residual.derivatives()[ad_offset + _j];
        }
  }
}

template <>
void
ADIntegratedBCTempl<Real, RESIDUAL>::computeJacobianBlock(MooseVariableFEBase &)
{
}
template <>
void
ADIntegratedBCTempl<RealVectorValue, RESIDUAL>::computeJacobianBlock(MooseVariableFEBase &)
{
}

template <typename T, ComputeStage compute_stage>
void
ADIntegratedBCTempl<T, compute_stage>::computeJacobianBlockScalar(unsigned int /*jvar*/)
{
}

template class ADIntegratedBCTempl<Real, RESIDUAL>;
template class ADIntegratedBCTempl<Real, JACOBIAN>;
template class ADIntegratedBCTempl<RealVectorValue, RESIDUAL>;
template class ADIntegratedBCTempl<RealVectorValue, JACOBIAN>;
