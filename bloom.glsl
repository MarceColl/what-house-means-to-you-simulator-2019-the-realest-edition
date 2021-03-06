uniform sampler2D 	source;
// uniform vec2 		offsetFactor;

float random( vec2 p )
{
    vec2 K1 = vec2(
        23.14069263277926, // e^pi (Gelfond's constant)
         2.665144142690225 // 2^sqrt(2) (Gelfondâ€“Schneider constant)
    );
    return fract( cos( dot(p,K1) ) * 12345.6789 );
}

void main()
{
	vec2 offsetFactor = vec2(0.05, 0.05);
	vec2 textureCoordinates = gl_TexCoord[0].xy;
	vec4 curr_color = texture2D(source, textureCoordinates);

	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	if (curr_color.r == 0.0) {
		color += texture2D(source, vec2(textureCoordinates.x - 4.0 * offsetFactor.x, textureCoordinates.y)) * 0.0162162162;
		color += texture2D(source, vec2(textureCoordinates.x - 3.0 * offsetFactor.x, textureCoordinates.y)) * 0.0540540541;
		color += texture2D(source, vec2(textureCoordinates.x - 2.0 * offsetFactor.x, textureCoordinates.y)) * 0.1216216216;
		color += texture2D(source, textureCoordinates - offsetFactor) * 0.1945945946;
		color += texture2D(source, textureCoordinates) * 0.2270270270;
		color += texture2D(source, textureCoordinates + offsetFactor) * 0.1945945946;
		color += texture2D(source, vec2(textureCoordinates.x + 2.0 * offsetFactor.x, textureCoordinates.y)) * 0.0540540541;
		color += texture2D(source, vec2(textureCoordinates.x + 3.0 * offsetFactor.x, textureCoordinates.y)) * 0.1216216216;
		color += texture2D(source, vec2(textureCoordinates.x + 4.0 * offsetFactor.x, textureCoordinates.y)) * 0.0162162162;
	}
	else {
		color = curr_color;
	}

	gl_FragColor = color;
}
