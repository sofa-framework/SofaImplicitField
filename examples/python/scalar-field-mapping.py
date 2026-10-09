from sympy import root

import Sofa
import Sofa.Core
from SofaImplicitField import ScalarField
import numpy
from xshape.primitives import *
from xshape.transforms import *
from xshape.operators import * 

def Model():
    particules = Sofa.Core.Node("Particles")
    particules.addObject("MechanicalObject", name="state", template="Vec3", position=[[-1,1.5,0], [-0.5,1.5,0], 
                                                                                      [0,1.5,0],
                                                                                      [0.5,1.5,0], [1,1.5,0]])
    particules.addObject("UniformMass", name="mass", totalMass=1.0)
    
    child = particules.addChild("Child")
    child.addObject("MechanicalObject", name="state", template="Vec1", position=[0,0,0,0,0])

    child.addObject("SphericalField", name="fieldsphere", center=[0,0,0], radius=1.0)

    child.addObject(
         Difference(name="fieldsrc",
               childA=Sphere(name="sphere", center=[0,0,0],radius=0.7),
               childB=RoundedBox(center=[0.0,0.0,0.0],dimensions=[0.95,0.5,0.5], rounding_radius=0.1))
      ) 
    m1 = child.addObject("GridSampler", name="sampler", min=[-2,-2,-2], max=[2,2,2], resolution=[255,255,255])
    m1.field.setLinkedBase(child.fieldsrc)
    child.addObject("DiscreteGridField", name="field", buffer=m1.buffer.linkpath)

    child.addObject("ScalarFieldMapping", name="mapping", 
                         input=particules.state.linkpath, 
                         output=child.state.linkpath, field=child.field.linkpath)
    child.addObject("FieldToGaussianSplat", name="gs", field=child.field.linkpath, resolution=[30,0,0])

    return particules

def createScene(root):
    """In this scene we create a ScalarField of spherical shape and a mechanical object of 3D particles. 
       The field and the particules are used as input of a ScalarFieldMapping, mapping the particules to a 1D child space.
       This child space is then used in a constraints. 
    """
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.AnimationLoop') # Needed to use components [FreeMotionAnimationLoop]  
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.Constraint.Lagrangian.Model') # Needed to use components [StopperLagrangianConstraint]  
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.Constraint.Lagrangian.Solver') # Needed to use components [BlockGaussSeidelConstraintSolver]  
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.StateContainer') # Needed to use components [MechanicalObject]  
    root.addObject("RequiredPlugin", pluginName="SofaImplicitField")
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.Mass') # Needed to use components [UniformMass]  

    root.addObject("FreeMotionAnimationLoop")
    root.addObject("BlockGaussSeidelConstraintSolver", name="solver", tolerance=1e-6, maxIterations=1000)

    root.addObject("EulerImplicitSolver", name="odesolver", rayleighStiffness=0.1, rayleighMass=0.1)
    root.addObject("SparseLDLSolver", name="linearSolver", template="CompressedRowSparseMatrixd")

    model = root.addChild(Model())
    model.state.showObject = True
    model.state.showObjectScale = 5.0

    model.Child.addObject("StopperLagrangianConstraint", name="constraint", min=0.00, max=100000.0)
    model.Child.addObject("GenericConstraintCorrection", name="correction", linearSolver=root.linearSolver.linkpath, ODESolver=root.odesolver.linkpath)
