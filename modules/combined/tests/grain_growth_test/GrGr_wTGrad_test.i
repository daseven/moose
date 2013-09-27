[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 1000
  ymin = 0
  ymax = 1000
  zmin = 0
  zmax = 0
  elem_type = QUAD4

   uniform_refine = 2
[]

[GlobalParams]
  crys_num = 2
  var_name_base = gr
  v = 'gr0 gr1'
[]

[Functions]
  [./TGradient]
    type = ParsedFunction
    value = '450 + 0.1*y'
  [../]
[]

[Variables]

  [./PolycrystalVariables]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./BicrystalCircleGrainIC]
      radius = 333.333
      x = 500
      y = 500
      int_width = 80
    [../]
  [../]
[]

[AuxVariables]

  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]

  [./T]
    order = FIRST
    family = LAGRANGE
  [../]
[]	     

[Kernels]

  [./PolycrystalKernel]
  [../]
[]

[AuxKernels]
  [./BndsCalc]
    type = BndsCalcAux
    variable = bnds
  [../]

  [./Tgrad]
    type = FunctionAux
    variable = T
    function = TGradient
  [../]
[]

[BCs]
  active = 'Periodic'

  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]

[]

[Materials]
  [./Copper]
    type = GBEvolution
    block = 0
    temp = 500 # K
    wGB = 60 # nm
    GBmob0 = 2.5e-6 #m^4/(Js) from Schoenfelder 1997
    Q = 0.23 #Migration energy in eV
    GBenergy = 0.708 #GB energy in J/m^2    
  [../]
[]

[Postprocessors]
  [./gr_area]
    type = ElementIntegralVariablePostprocessor
    variable = gr0
  [../]
[]

[Preconditioning]

  [./SMP]
   type = SMP
   full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'NEWTON'


  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 30
  l_tol = 1.0e-4

  nl_max_its = 20
  nl_rel_tol = 1.0e-9

  start_time = 0.0
  num_steps = 6
  dt = 80.0

  [./Adaptivity]
   initial_adaptivity = 2
    refine_fraction = 0.8
    coarsen_fraction = 0.05
    max_h_level = 2
  [../]
[]

[Output]
  file_base = wT
  interval = 1
  exodus = true
  perf_log = true
[]
   
    

