#ifndef POLYCRYSTALREDUCEDIC_H
#define POLYCRYSTALREDUCEDIC_H

#include "Kernel.h"
#include "InitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class PolycrystalReducedIC;

template<>
InputParameters validParams<PolycrystalReducedIC>();

/**
 * PolycrystalReducedIC creates a polycrystal initial condition.
 * With 2 Grains, _typ = 0 results in a circular inclusion grain and _type = 1 gives a bicrystal.
 * With more than 2 grains, _typ = 0 gives set positions for 6 grains, _type = 1 gives hexagonal grains for 4 grains.
 *                          _typ = 2 Gives a random voronoi structure
*/
class PolycrystalReducedIC : public InitialCondition
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  PolycrystalReducedIC(const std::string & name,
                InputParameters parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point & p);

  virtual void initialSetup();

  MooseMesh & _mesh;
  /// A reference to the nonlinear system
  NonlinearSystem & _nl;

  unsigned int _op_num;
  unsigned int _grain_num;
  unsigned int _op_index;

  unsigned int _rand_seed;

  bool _cody_test;

  Point _bottom_left;
  Point _top_right;
  Point _range;
  

  std::vector<Point> _centerpoints;
  std::vector<Real> _assigned_op;
  
};

#endif //POLYCRYSTALREDUCEDIC_H
