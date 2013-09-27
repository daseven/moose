#ifndef HEXPOLYCRYSTALIC_H
#define HEXPOLYCRYSTALIC_H

#include "Kernel.h"
#include "PolycrystalReducedIC.h"

// System includes
#include <string>

// Forward Declarations
class HexPolycrystalIC;

template<>
InputParameters validParams<HexPolycrystalIC>();

/**
 * HexPolycrystalIC creates a hexagonal polycrystal initial condition.
 * Only works for squared number of grains and with periodic BCs
*/
class HexPolycrystalIC : public PolycrystalReducedIC
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  HexPolycrystalIC(const std::string & name,
                InputParameters parameters);
  
  virtual void initialSetup();

private:
  Real _x_offset;
  
};

#endif //HEXPOLYCRYSTALIC_H
