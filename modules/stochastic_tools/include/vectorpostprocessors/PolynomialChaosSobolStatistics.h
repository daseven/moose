//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "PolynomialChaos.h"

class PolynomialChaosSobolStatistics : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  PolynomialChaosSobolStatistics(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Reference to PolynomialChaos
  const PolynomialChaos & _pc_uo;
  /// The selected Sobol sensitivity orders to compute
  const MultiMooseEnum & _order;
  /// Whether or not we have initialized the vectors
  bool _initialized;
  /// Vector containing the Sobol statistics
  VectorPostprocessorValue & _stats;
  /// Vector containing the Sobol total sensitivity indices
  VectorPostprocessorValue * _total_ind;
  /// Vector containing the Sobol sensitivity indices
  std::vector<VectorPostprocessorValue *> _ind_vector;

private:
  /// Helper function to gather the sensitivity indices
  static std::set<std::set<unsigned int>>
  buildSobolIndices(const unsigned int nind, const unsigned int ndim, const unsigned int sdim);
  /// Function to set ordering of sobol indices
  static bool orderSobolIndices(const std::set<unsigned int> & a, const std::set<unsigned int> & b);
  struct SobolOrder
  {
    bool operator()(const std::set<unsigned int> & a, const std::set<unsigned int> & b) const
    {
      if (a.size() != b.size())
        return a.size() < b.size();
      else
        return a < b;
    }
  };

  /// Number of indices based on maximum order
  unsigned int _nind;
  /// Number of values to be computed
  unsigned int _nval;
  /// Set containing all sensitivities computed
  std::set<std::set<unsigned int>, SobolOrder> _ind;
};
