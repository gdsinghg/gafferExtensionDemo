import unittest

import IECore
import IECoreScene

import Gaffer
import GafferScene
import GafferTest
import GafferSceneTest

import DemoGafferExtension

class SineDeformerTest( GafferSceneTest.SceneTestCase ) :

	def testAffects( self ) :
		node = DemoGafferExtension.SineDeformer()
		s = GafferTest.CapturingSlot( node.plugDirtiedSignal() )
		plane = GafferScene.Plane()
		node["in"].setInput( plane["out"] )
		affected = [ c[0] for c in s ]
		self.assertTrue( node["in"] in affected )
		self.assertTrue( node["out"]["bound"] in affected )
		self.assertTrue( node["out"]["object"] in affected )
		self.assertTrue( node["out"] in affected )
		del s[:]
		node["absoluteSine"].setValue(True)
		self.assertEqual( len( s ), 4 )
		self.failUnless( s[0][0].isSame( node["absoluteSine"] ) )
		self.failUnless( s[1][0].isSame( node["out"]["object"] ) )
		self.failUnless( s[2][0].isSame( node["out"]["bound"] ) )
		self.failUnless( s[3][0].isSame( node["out"] ) )

	def testCompute( self ) :

		   plane = GafferScene.Plane()

		   group = GafferScene.Group()
		   group["in"][0].setInput( plane["out"] )

		   filter = GafferScene.PathFilter()
		   filter["paths"].setValue( IECore.StringVectorData( [ "/group/plane" ] ) )

		   sineDeformer = DemoGafferExtension.SineDeformer()
		   sineDeformer["in"].setInput( group["out"] )
		   sineDeformer["filter"].setInput( filter["out"] )

		   self.assertSceneValid( sineDeformer["out"] )
		   mesh = group["out"].object( "/group/plane", _copy = False )
		   newMesh = sineDeformer["out"].object( "/group/plane", _copy = False )

		   # check that the new mesh has valid primitive variables.
		   self.assertTrue( newMesh.arePrimitiveVariablesValid() )
		   # check that the number of points are equal to points of the plane
		   # + the extra sample points.
		   self.assertEqual( newMesh["P"].data.size(), mesh["P"].data.size() + sineDeformer["samplePoints"].getValue()*2 )

if __name__ == "__main__":
    unittest.main()
