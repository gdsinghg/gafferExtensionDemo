//
//  SineDeformer.h
//

#ifndef DEMOGAFFEREXTENSION_SineDeformer_h
#define DEMOGAFFEREXTENSION_SineDeformer_h

#include "DemoGafferExtension/TypeIds.h"
#include "GafferScene/Deformer.h"

namespace Gaffer
{

IE_CORE_FORWARDDECLARE( StringPlug )

} // namespace Gaffer

namespace DemoGafferExtension
{
class SineDeformer : public GafferScene::Deformer
{

public:

    SineDeformer( const std::string &name = defaultName<SineDeformer>() );
    ~SineDeformer() override;

    GAFFER_GRAPHCOMPONENT_DECLARE_TYPE( DemoGafferExtension::SineDeformer, TypeId::SineDeformerTypeId, GafferScene::Deformer );
    
    Gaffer::StringPlug *axisTypePlug();
    const Gaffer::StringPlug *axisTypePlug() const;

	Gaffer::BoolPlug *absoluteSinePlug();
	const Gaffer::BoolPlug *absoluteSinePlug() const;

	Gaffer::IntPlug *cyclesPlug();
	const Gaffer::IntPlug *cyclesPlug() const;

	Gaffer::IntPlug *samplePointsPlug();
	const Gaffer::IntPlug *samplePointsPlug() const;

	Gaffer::FloatPlug *amplitudePlug();
	const Gaffer::FloatPlug *amplitudePlug() const;
	
protected :

    bool affectsProcessedObject( const Gaffer::Plug *input ) const override;
    void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
    IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const override;

private :

    static size_t g_firstPlugIndex;
};
}


#endif /* SineDeformer_h */
