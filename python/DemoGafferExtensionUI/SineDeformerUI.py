import IECore
import Gaffer

import DemoGafferExtension

Gaffer.Metadata.registerNode(

    DemoGafferExtension.SineDeformer,

    "description",
    """
    This node deforms input mesh along a sine curve.
    """,

    plugs = {

        "adjustBounds" : [

            "userDefault", False,

        ],

        "axisType" : [

            "description",
            """
            The co-ordinate axis along which the wave is propagated.
            The first axis is the wave axis.
            The second axis is the time or another co-ordinate axis.
            eg. y = sinx for every value x there is a y
            """,

            "preset:ZY", "zy",
            "preset:YZ", "YZ",
            "preset:XZ", "xz",
            "preset:ZX", "zx",
            "preset:XY", "xy",
            "preset:YX", "YX",

            "plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",

        ],

		"absoluteSine" : [

            "description",
            """
            Whether to take the absolute value of sine.
            """
        ],

		"cycles" : [

            "description",
            """
            Cycles of the wave.
            """
        ],

		"samplePoints" : [

            "description",
            """
            Number of points between the end points of the wave.
            """
        ],

		"amplitude" : [

            "description",
            """
            Amplitude of the wave.
            """
        ],

    }
)
