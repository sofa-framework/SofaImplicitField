from pyexpat import model

from sympy import root

import Sofa

def Robot():
    import numpy as np

    N = 100      # nombre de points
    x = 10         # valeur fixe de X
    y = np.random.uniform(-0.1, 0.1, N)
    z = np.random.uniform(-1.1, -1, N)

    points = np.column_stack((np.full(N, x), y, z))

    robot = Sofa.Core.Node("Robot")
    robot.addObject("MechanicalObject", name="state", template="Vec3", position=[(yi, x, zi) for yi, zi in zip(y, z)])
    robot.addObject("UniformVelocityDampingForceField", name="damping", damping=0.002)
    robot.addObject("UniformMass", name="mass", totalMass=1.0)
    return robot

def addResponseContact(robot, field, context):
    child = robot.addChild("CollisionResponse")        
    child.addObject("MechanicalObject", name="state", template="Vec1", position=[0]*len(robot.state.position.value))
    child.addObject("ScalarFieldMapping", name="mapping", 
                                 input=robot.state.linkpath, 
                                 output=child.state.linkpath, 
                                 field=field.linkpath)
    child.addObject("StopperLagrangianConstraint", name="constraint", min=0.00, max=100000.0)
    child.addObject("GenericConstraintCorrection", name="correction", linearSolver=context.linearSolver.linkpath, ODESolver=context.odesolver.linkpath)
    return child

def addResponse(robot, field):
    child = robot.addChild("CollisionResponse")        
    child.addObject("MechanicalObject", name="state", template="Vec1", position=[0]*len(robot.state.position.value))
    child.addObject("ScalarFieldMapping", name="mapping", 
                             input=robot.state.linkpath, 
                             output=child.state.linkpath, 
                             field=field.linkpath)
    child.addObject("AllowedIntervalSpringForceField", name="forcefield", stiffness=0.1, damping=10)
    return child

def Environment():
    root = Sofa.Core.Node("Environment")
    root.addObject("GridVDBLoader", name="loader", filename="volumes/test1/1297782461/blobs/00000_00000_file_1.vdb")
    #root.addObject("SparseGridToGrid", name="engine", source=root.loader.vdbgrid.linkpath, resolution=[256, 256, 256], min=[-10,-10,-10], max=[10,10,10])
    #root.addObject("DiscreteGridField", name="container", buffer=root.engine.buffer.linkpath)
    root.addObject("SparseGridField", name="container", vdbgrid=root.loader.vdbgrid.linkpath) 
    #root.addObject("SphericalField", name="container", radius=7) 
        
    #root.addObject("FieldToSurfaceMesh", name="engine", field=root.container.linkpath, min=[-10,-10,-10], max=[10,10,10], step=1.0)
    
    #root.addObject("GridSampler", name="sampler", field=root.container.linkpath, resolution=[100,100,100])
    #root.addObject("DiscreteGridField", name="dgrid", buffer=root.sampler.buffer.linkpath)
    #root.addObject("FieldToGaussianSplat", name="splats", field=root.container.linkpath, min=[-2,-2,-2], max=[2,2,2], resolution=[100,100,100])
    #root.addObject("SparseGridToSurfaceMesh", name="mesher", source=root.loader.vdbgrid.linkpath, printLog=True)  
    if True:
        root.addObject("SparseGridToSurfaceMesh", name="mesher", source=root.loader.vdbgrid.linkpath)  
        root.addObject("OglModel", name="renderer",
                               position=root.mesher.points.linkpath,
                               triangles=root.mesher.triangles.linkpath,
                               quads=root.mesher.quads.linkpath, alphaBlend=True, color=[0.5,0.5,0.5,0.5])
    return root

def createScene(root):
    #root.gravity.value = [-9.81, 0, 0]
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.AnimationLoop') # Needed to use components [FreeMotionAnimationLoop]  
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.StateContainer') # Needed to use components [MechanicalObject]  
    root.addObject("RequiredPlugin", pluginName="SofaImplicitField")
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.Mass') # Needed to use components [UniformMass]  
    root.addObject("RequiredPlugin", pluginName=["SofaImplicitField"])
    root.addObject("VisualStyle", displayFlags="showVisual")
    root.addObject("InteractiveCamera", computeZClip=False)

    root.addObject("FreeMotionAnimationLoop")
    root.addObject("BlockGaussSeidelConstraintSolver", name="solver", tolerance=1e-6, maxIterations=1000)

    root.addObject("EulerImplicitSolver", name="odesolver")
    root.addObject("SparseLDLSolver", name="linearSolver", template="CompressedRowSparseMatrixd")

    environment = root.addChild(Environment())
    #environment.showObject = True

    model = root.addChild(Robot())
    model.state.showObject = True
    model.state.showObjectScale = 5.0

    #addResponse(model, environment.container)    
    response = addResponseContact(model, environment.container, context=root)
    response.addObject("TrailRenderer", position=model.state.position.linkpath, color=[1,0,0,1])

    return root