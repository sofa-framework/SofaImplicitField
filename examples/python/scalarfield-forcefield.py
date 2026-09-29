import Sofa
import Sofa.Core
from SofaImplicitField import ScalarField
import numpy

def Model():
    particules = Sofa.Core.Node("Particles")
    particules.addObject("MechanicalObject", name="state", template="Vec3", position=[[-2,2,0],[-1,2,0],[-0.5,2,0], 
                                                                                      [0,2,0],[0.5,2,0],[1,2,0],[2,2,0]])
    particules.addObject("UniformMass", name="mass", totalMass=1.0)
    
    child = particules.addChild("Child")
    child.addObject("MechanicalObject", name="state", template="Vec1", position=[0,0,0,0,0,0,0])
    child.addObject("SphericalField", name="field", center=[0,0,0], radius=1.0)
    child.addObject("ScalarFieldMapping", name="mapping", 
                         input=particules.state.linkpath, 
                         output=child.state.linkpath, field=child.field.linkpath)

    return particules

def createScene(root):
    """In this scene we create a ScalarField of spherical shape and a mechanical object of 3D particles. 
       The field and the particules are used as input of a ScalarFieldMapping, mapping the particules to a 1D child space.
       This child space is then used in a constraints. 
    """
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.AnimationLoop') # Needed to use components [FreeMotionAnimationLoop]  
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.StateContainer') # Needed to use components [MechanicalObject]  
    root.addObject("RequiredPlugin", pluginName="SofaImplicitField")
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.Mass') # Needed to use components [UniformMass]  

    root.addObject("DefaultAnimationLoop")

    root.addObject("EulerImplicitSolver", name="odesolver", rayleighStiffness=0.1, rayleighMass=0.1)
    root.addObject("SparseLDLSolver", name="linearSolver", template="CompressedRowSparseMatrixd")
    #root.addObject("CGLinearSolver", name="linearSolver", template="CompressedRowSparseMatrixd")

    model = root.addChild(Model())
    model.state.showObject = True
    model.state.showObjectScale = 5.0

    #model.Child.addObject("AllowedIntervalSpringForceField", name="forcefield", stiffness=200.)
    #model.Child.addObject("MechanicalObject", name="target", stiffness=200., state1=particules.state.linkpath, state2=model.Child.state.linkpath)
    
    model.Child.addObject("AllowedIntervalSpringForceField", name="forcefield", stiffness=3, damping=3)

    #r = root.addChild("Rendu")
    #r.addObject("MechanicalObject", name="state", template="Vec3", position=[[0,0,0]])
    #r.addObject("SphereCollisionModel", name="sphericalModel", radius=1.0)
