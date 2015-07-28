// ###########################################################################################
// ## A simple vertex shader taking a position and a colour and outputting them to the next
// ## stage (and changing the position to homogeneous coordinates).
// ##
// ## Copyright (c) <2015> <Tim Henriksson and Kim Restad>
// ## 
// ## This software is provided 'as-is', without any express or implied warranty. In no event
// ## will the authors be held liable for any damages arising from the use of this software.
// ## 
// ## Permission is granted to anyone to use this software for any purpose, including
// ## commercial applications, and to alter it and redistribute it freely, subject to the
// ## following restrictions:
// ## 
// ## 1. The origin of this software must not be misrepresented; you must not claim that you
// ## wrote the original software. If you use this software in a product, an acknowledgement
// ## in the product documentation would be appreciated but is not required.
// ## 2. Altered source versions must be plainly marked as such, and must not be
// ## misrepresented as being the original software.
// ## 3. This notice may not be removed or altered from any source distribution.
// ##
// ###########################################################################################

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