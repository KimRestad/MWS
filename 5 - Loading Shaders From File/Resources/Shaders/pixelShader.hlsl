// Struct defining the format of the input to the pixel shader. This must correspond to the
// output format of the previous stage in the pipeline - in our case the vertex shader.
struct PSInput
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
};

// The main function is executed for each pixel covered by a primitive (triangle, in this
// sample) and returns the colour the pixel should be painted in - a four dimensional float
// using the semantic SV_TARGET. The input's colour is interpolated between the vertices
// colour giving a nice gradient. This is returned.
float4 main(PSInput input) : SV_TARGET
{
	return input.colour;
}