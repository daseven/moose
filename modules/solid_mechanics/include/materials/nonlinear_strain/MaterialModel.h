#ifndef MATERIALMODEL_H
#define MATERIALMODEL_H

#include "Material.h"

#include "SymmTensor.h"

// Forward declarations
class ElasticityTensor;
class MaterialModel;
class VolumetricModel;

template<>
InputParameters validParams<MaterialModel>();

/**
 * MaterialModel is the base class for all solid mechanics material models in Elk.
 */
class MaterialModel : public Material
{
public:
  MaterialModel( const std::string & name,
                 InputParameters parameters );
  virtual ~MaterialModel();

  void testMe();

  static Real detMatrix( const ColumnMajorMatrix & A );

  static void invertMatrix( const ColumnMajorMatrix & A,
                            ColumnMajorMatrix & Ainv );

  static void rotateSymmetricTensor( const ColumnMajorMatrix & R, const RealTensorValue & T,
                                     RealTensorValue & result );
  static void rotateSymmetricTensor( const ColumnMajorMatrix & R, const SymmTensor & T,
                                     SymmTensor & result );

protected:

  enum DecompMethod
  {
    RashidApprox = 0,
    Eigen        = 1
  };

  bool _bulk_modulus_set;
  bool _lambda_set;
  bool _poissons_ratio_set;
  bool _shear_modulus_set;
  bool _youngs_modulus_set;

  Real _bulk_modulus;
  Real _lambda;
  Real _poissons_ratio;
  Real _shear_modulus;
  Real _youngs_modulus;

  Real _cracking_strain;

  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;
  VariableGradient & _grad_disp_x_old;
  VariableGradient & _grad_disp_y_old;
  VariableGradient & _grad_disp_z_old;

  const bool _has_temp;
  VariableValue & _temperature;
  VariableValue & _temperature_old;

  std::vector<VolumetricModel*> _volumetric_models;

  DecompMethod _decomp_method;
  const Real _alpha;

  MaterialProperty<SymmTensor> & _stress;
  MaterialProperty<SymmTensor> & _stress_old;

  MaterialProperty<SymmTensor> & _total_strain;
  MaterialProperty<SymmTensor> & _total_strain_old;

  MaterialProperty<RealVectorValue> * _crack_flags;
  MaterialProperty<RealVectorValue> * _crack_flags_old;

  MaterialProperty<ColumnMajorMatrix> & _Jacobian_mult;

  // Accumulate derivatives of strain tensors with respect to Temperature into this
  ColumnMajorMatrix _d_strain_dT;

  // The derivative of the stress with respect to Temperature
  MaterialProperty<RealTensorValue> & _d_stress_dT;

  SymmTensor _total_strain_increment;
  SymmTensor _strain_increment;
  ColumnMajorMatrix _incremental_rotation;
  ColumnMajorMatrix _Uhat;


  std::vector<ColumnMajorMatrix> _Fhat;
  std::vector<ColumnMajorMatrix> _Fbar;


  virtual void initialSetup();


  virtual void computeProperties();



  /// Modify increment for things like thermal strain
  virtual void modifyStrain();

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress() = 0;

  /// Rotate stress to current configuration
  virtual void finalizeStress();


  void computeIncrementalDeformationGradient( std::vector<ColumnMajorMatrix> & Fhat);
  void computeStrainIncrement( const ColumnMajorMatrix & Fhat);
  void computePolarDecomposition( const ColumnMajorMatrix & Fhat);
  virtual void computePreconditioning();
  int delta(int i, int j);

  void computeStrainAndRotationIncrement( const ColumnMajorMatrix & Fhat);


  void fillMatrix( const VariableGradient & grad_x,
                   const VariableGradient & grad_y,
                   const VariableGradient & grad_z,
                   ColumnMajorMatrix & A );

  void
  elasticityTensor( ElasticityTensor * e );

  ElasticityTensor *
  elasticityTensor() const
  {
    return _elasticity_tensor;
  }


private:
  ElasticityTensor * _elasticity_tensor;

};



#endif
