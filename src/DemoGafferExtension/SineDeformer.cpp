//
//  SineDeformer.cpp
//

#include <math.h>
#include <algorithm>
#include "DemoGafferExtension/SineDeformer.h"

#include "Gaffer/StringPlug.h"

#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Gaffer;
using namespace GafferScene;
using namespace DemoGafferExtension;

IE_CORE_DEFINERUNTIMETYPED(SineDeformer );

size_t SineDeformer::g_firstPlugIndex = 0;

SineDeformer::SineDeformer( const std::string &name )
    :    Deformer( name )
{
    storeIndexOfNextChild( g_firstPlugIndex );
	// the co-ordinate axis along which the wave is propagated.
    addChild( new StringPlug( "axisType", Plug::In, "zy" ) );
	addChild( new BoolPlug( "absoluteSine", Plug::In, false) );
	addChild( new IntPlug( "cycles", Plug::In, 1, 1 ) );
	addChild( new IntPlug( "samplePoints", Plug::In, 50, 1 ) );
	addChild( new FloatPlug( "amplitude", Plug::In, 2.0f, 0.01 ) );
}

SineDeformer::~SineDeformer()
{
}

Gaffer::StringPlug *SineDeformer::axisTypePlug()
{
    return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *SineDeformer::axisTypePlug() const
{
    return getChild<StringPlug>( g_firstPlugIndex );
}

Gaffer::BoolPlug *SineDeformer::absoluteSinePlug()
{
	return getChild<BoolPlug>( g_firstPlugIndex + 1);
}

const Gaffer::BoolPlug *SineDeformer::absoluteSinePlug() const
{
	return getChild<BoolPlug>( g_firstPlugIndex + 1);
}

Gaffer::IntPlug *SineDeformer::cyclesPlug()
{
	return getChild<IntPlug>( g_firstPlugIndex + 2);
}

const Gaffer::IntPlug *SineDeformer::cyclesPlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex + 2);
}

Gaffer::IntPlug *SineDeformer::samplePointsPlug()
{
	return getChild<IntPlug>( g_firstPlugIndex + 3);
}

const Gaffer::IntPlug *SineDeformer::samplePointsPlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex + 3);
}

FloatPlug *SineDeformer::amplitudePlug()
{
	return getChild<FloatPlug>( g_firstPlugIndex + 4 );
}

const FloatPlug *SineDeformer::amplitudePlug() const
{
	return getChild<FloatPlug>( g_firstPlugIndex + 4 );
}

bool SineDeformer::affectsProcessedObject( const Gaffer::Plug *input ) const
{
	return Deformer::affectsProcessedObject( input ) ||
	input == axisTypePlug() || input == absoluteSinePlug() || input == cyclesPlug()
	|| input == samplePointsPlug() || input == amplitudePlug();
}

void SineDeformer::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    Deformer::hashProcessedObject( path, context, h );
    axisTypePlug()->hash( h );
	absoluteSinePlug()->hash( h );
	cyclesPlug()->hash( h );
	samplePointsPlug()->hash( h );
	amplitudePlug()->hash( h );
}

IECore::ConstObjectPtr SineDeformer::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
{
    const MeshPrimitive *mesh = runTimeCast<const MeshPrimitive>( inputObject );
    if( !mesh )
    {
        return inputObject;
    }


	// read the values from the various plugs.
	const int cycles = cyclesPlug()->getValue();
	const int samplePoints = samplePointsPlug()->getValue();
	const std::string axis = axisTypePlug()->getValue();
	bool absolute = absoluteSinePlug()->getValue();
	float amplitude = amplitudePlug()->getValue();

	// There are two axis. One along which the wave is propagating.
	// and the other the time axis.
	// In case of a zy, the sine wave is propagating along z
	// for a value of y going from one number value to another.
	// z = sin(t) and y = t where t E [y1, y2]
	Imath::V3f waveAxis(0, 0, 1);
	Imath::V3f timeAxis(0, 1, 0);
	if(axis == "xz" || axis == "XZ")
	{
		waveAxis = Imath::V3f(1, 0, 0);
		timeAxis = Imath::V3f(0, 0, 1);
	}
	else if(axis == "zx" || axis == "ZX")
	{
		waveAxis = Imath::V3f(0, 0, 1);
		timeAxis = Imath::V3f(1, 0, 0);
	}
	else if(axis == "xy" || axis == "XY")
	{
		waveAxis = Imath::V3f(1, 0, 0);
		timeAxis = Imath::V3f(0, 1, 0);
	}
	else if(axis == "yx" || axis == "YX")
	{
		waveAxis = Imath::V3f(0, 1, 0);
		timeAxis = Imath::V3f(1, 0, 0);
	}
	else if(axis == "yz" || axis == "YZ")
	{
		waveAxis = Imath::V3f(0, 1, 0);
		timeAxis = Imath::V3f(0, 0, 1);
	}
	else if(axis == "zy" || axis == "ZY")
	{
		waveAxis = Imath::V3f(0, 0, 1);
		timeAxis = Imath::V3f(0, 1, 0);
	}
	else
	{
		throw InvalidArgumentException( "SineDeformer: the input axis is wrong" );
	}

	// find the points.
	PrimitiveVariableMap::const_iterator it = mesh->variables.find( "P" );
	if ( it == mesh->variables.end() )
	{
		return inputObject;
	}
	ConstV3fVectorDataPtr pData = runTimeCast< const V3fVectorData >( it->second.data );
	if ( !pData )
	{
		throw InvalidArgumentException( "SineDeformer: Input mesh has no 'P' primvar of type V3fVectorData" );
	}
	const V3fVectorData::ValueType &p = pData->readable();
	// get the vertices per face and vertex ids.
	const std::vector<int> &verticesPerFace = mesh->verticesPerFace()->readable();
	const std::vector<int> &vertexIds = mesh->vertexIds()->readable();

	// find all the faces along which there would be a wave.
	// If the normal of the face is in same direction
	// as the time axis then that face is not waved otherwise it is.
	// for example in a cube the top two faces have normals in the y
	// direction and if we choose zy and y is the timeAxis,
	// we are not going to wave the top two faces
	// as they are the end points of the wave.
	std::vector<int> facesToWave;
	for(int i = 0; i < verticesPerFace.size();++i)
	{
		int vertices = verticesPerFace[i];
		assert(vertices*(i + 1) <= vertexIds.size());
		if (vertices == 4)
		{
			int begin = vertices*(i);
			Imath::V3f p1 = p[vertexIds[begin]];
			Imath::V3f p2 = p[vertexIds[begin+1]];
			Imath::V3f p3 = p[vertexIds[begin+2]];
			Imath::V3f p4 = p[vertexIds[begin+3]];
			// calculate normal and do the dot product with timeAxis.
			// if the normal and the timeaxis is not in the same direction
			// then do the wave.
			Imath::V3f normal = (p3 - p1).cross(p3 - p2);
			if(normal.dot(timeAxis) == 0)
			{
				facesToWave.push_back(i);
			}
		}
		else
		{
			// not handling cases with non 4 verts per face.
			return inputObject;
		}
	}

	// create containers for points.
	V3fVectorDataPtr newP = new V3fVectorData();
	newP->setInterpretation( GeometricData::Point );
	IntVectorDataPtr newVerticesPerFace = new IntVectorData();
	IntVectorDataPtr newVertexIds = new IntVectorData();
	auto &points = newP->writable();
	// set the size to points + the new points we are going to create
	// to create extra faces. We create extra faces by taking a face and it's
	// two vertex on one side and connecting these vertex with two extra points
	// that we generate using the sine equation. So we are connecting two points
	// per sample to create a new face. Taking into all the samples and the faces
	// to wave gives us faces*samples*2 extra points. This plus the points we
	// already have.
	points.resize(p.size() + facesToWave.size()*samplePoints*2, Imath::V3f(0));
	int nextVertexId = p.size();
	for(int i = 0; i < verticesPerFace.size();++i)
	{
		// check if the face is in the facesToWave vector.
		bool doWave = std::find(facesToWave.begin(), facesToWave.end(), i) != facesToWave.end();
		int vertices = verticesPerFace[i];
		assert(vertices*(i + 1) <= vertexIds.size());
		int begin = vertices*(i);
		int vertexId1 = vertexIds[begin];
		int vertexId2 = vertexIds[begin+1];
		int vertexId3 = vertexIds[begin+2];
		int vertexId4 = vertexIds[begin+3];
		// get the points on the face.
		Imath::V3f p1 = p[vertexId1];
		Imath::V3f p2 = p[vertexId2];
		Imath::V3f p3 = p[vertexId3];
		Imath::V3f p4 = p[vertexId4];
		// Now we choose two points that form an edge on one side.
		// They are connected to two points that form an edge
		// on the other side. We create a sine wave between them.
		// for example
		/*          p1------- p2
					|         |
					|		  |
					P4--------p3
			We are creating a sine wave between p1 and p4 and p2 and p3.
			Now we have to determine the correct p1, p2 ....
			If the timeaxis lies alongside p1 and p4 then we have the correct
			ones otherwise we need to flip p2 and p4. The wave is perpendicular
			to the time axis.
		 */
		if(timeAxis.dot(p1 - p4) == 0)
		{
			p2 = p[vertexId4];
			p4 = p[vertexId2];
			int tmp = vertexId4;
			vertexId4 = vertexId2;
			vertexId2 = tmp;
		}
		// add the points that form one side of a face edge.
		points[vertexId1] = p1;
		points[vertexId2] = p2;
		newVertexIds->writable().push_back ( vertexId1 );
		newVertexIds->writable().push_back ( vertexId2 );
		Imath::V3f oneExtraPoint = p2;
		int j = 1;
		// if we are doing the wave then we create new points,
		// and faces in the loop.
		while(doWave)
		{
			// sine = ASin(2*pi*f*t) t E [1, samplePoints]
			// t = samplePoints. sine is zero and equal to the point
			// on the other end. So we begin from one point, along the sine
			// and then to the point on the other end.
			float frequency = cycles/float(samplePoints);
			float sineFactor = amplitude*sin(2*(22/7.0)*frequency*j);
			// if we want absolute.
			if (absolute && sineFactor < 0)
				sineFactor *= -1;
			// The code below is best explained by an example.
			// zy axis.
			// waveAxis = (0, 0, 1)
			// timeAxis = (0, 1, 0)
			// sineFactor*waveAxis is the amount by which the newPoint
			// would be above the z component of p1 and p4.
			// Now we add this to a sort of linear
			// interpolation between p1 and p4 along the y axis. The x component
			// is the same between them and will not change after the addition.
			/* best explained by an example j = 1 and sample points 100.
			 ((samplePoints - j)/float(samplePoints))*p1 is 0.99*p1
			 (j/float(samplePoints))*p4 is 0.01*p3.
			 This addition would result in a y co-ordinate
			 between p1 and p4 and the z co-ordinate is the percentage
			 of contribution between them + contribution from the sine.
			*/
			Imath::V3f newPoint1 = sineFactor*waveAxis + ((samplePoints - j)/float(samplePoints))*p1 + (j/float(samplePoints))*p4;
			Imath::V3f newPoint2 = sineFactor*waveAxis + ((samplePoints - j)/float(samplePoints))*p2 + (j/float(samplePoints))*p3;
			// add the new vertex ids and points to complete the face.
			newVertexIds->writable().push_back ( nextVertexId+1 );
			newVertexIds->writable().push_back ( nextVertexId );
			points[nextVertexId] = newPoint1;
			points[nextVertexId+1] = newPoint2;
			newVerticesPerFace->writable().push_back(4);
			++j;
			// add the vertex ids for the next face which would be the
			// vertexIds from the previous one (shared edge.)
			newVertexIds->writable().push_back ( nextVertexId );
			newVertexIds->writable().push_back ( nextVertexId+1 );
			// increment the vertex id by two to move onto the next
			// two points to be added.
			nextVertexId += 2;
			// break when we reach the sample points.
			if(j > samplePoints)
				break;
		}
		// add the end points for the face. We may have added extra points
		// and faces or not. It does not matter here. We just add a face
		// and the two points. If there were no extra points then these just
		// connect to p1 and p2, otherwise they connect to the last extra point.
		newVerticesPerFace->writable().push_back(4);
		newVertexIds->writable().push_back ( vertexId3 );
		newVertexIds->writable().push_back ( vertexId4 );
		points[vertexId3] = p3;
		points[vertexId4] = p4;
	}
	MeshPrimitivePtr newMesh = new MeshPrimitive( newVerticesPerFace, newVertexIds, "linear", newP );
    return newMesh;
}
