// ###########################################################################################
// ## A simple pixel shader taking a position and a colour and outputting a pixel colour.
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