#  Demo SineDeformer Documentation
The sine wave deform takes an input mesh and creates a sine wave along a certain co-ordinate plane. For instance in zy. It would create a wave propagating along the z axis for y values in a certain range eg. z = sin(y)

### Begin
Create a mesh node (plane or cube) and connect it to the demo sinedeformer node. Then you select the sine deformer node. Also select the plane or cube in the hierarchy view and add it to the filter section of the node (check testImages/filterSetup).

### Node Parameters
Some of the user facing values can be explained by this equation.
(check testImages/cubeExample)

Amplitude
The amplitude is the amplitude of the sine wave.

Cycles
frequency = cycles/t   A cycle is the number of waves .
Check out testImages/cyclesExample and compare it to cyclesCubeExample.
Basically there are more waves.

Sample Points
It is the t value in the equation. A higher value creates a much more smoother curve.

Absolute sine
A sine curve can have negative value. If we want to flip the negative part to
the positive part then we can turn this on. In most of the examples referenced
here it has been turned on.
check out testImages/cyclesExample and testImages/cubeNonAbsolute.

They are some other examples added here like
testImages/cubeExampleNumber which creates a number 
or 
testImages/cubeMeshToPoints adding a mesh to points.
The points form a sine curve.


