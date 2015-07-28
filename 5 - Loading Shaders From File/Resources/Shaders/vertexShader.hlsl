// Struct defining the format of the input to the vertex shader. This needs to correspond to
// an input layout bound to the pipeline, which in turn should correspond to the vertex
// structure.
struct VSInput
{
	float3 position : POSITION;
	float4 colour : COLOR;
};

// Struct defining the format of the output from the vertex shader. It may (as in this case),
// but doesn't have to be the same as the input struct. However it must correspond to the
// input struct of the next stage in the pipeline which in our case is the pixel shader.
struct VSOutput
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
};

// The main function simply passes on the input information for each vertex to the output, changing
// the position from a three dimensional to a four dimensional vector where the last element, w, is
// 1.0f.
VSOutput main(VSInput input)
{
	VSOutput output;

	output.position = float4(input.position, 1.0f);
	output.colour = input.colour;

	return output;
}