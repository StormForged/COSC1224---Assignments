//shader.frag
varying vec4 vColor;
varying vec3 normal;

uniform vec3 waveAmbient;
uniform vec3 waveDiffuse;
uniform vec3 waveSpecular;

uniform bool perPixel;

vec3 calcLight(){
	vec3 La, Ma, ambient, lEC, nEC, Ld, Md, Ls, Ms, vEC, H, diffuse, specular, nColor;
		float depth = gl_FragCoord.z;

		nColor = vec3(0.0);

		La = vec3(0.2);
		Ma = vec3(0.2);
		ambient = (La * Ma);
		nColor += ambient * waveAmbient;

		lEC = vec3(0.0, 0.0, 1.0);
		nEC = normal;

		float dp = dot(nEC, lEC);
		if(dp > 0.0){
			Ld = vec3(1.0);
			Md = vec3(0.8);

			nEC = normalize(nEC);
			float NdotL = dot(nEC, lEC);
			diffuse = vec3(Ld * Md * NdotL);
			nColor += diffuse * waveDiffuse;

			//Specular
			Ls = vec3(1.0);
			Ms = vec3(1.0);
			vEC = vec3(0.0, 0.0, 1.0);
	
			H = vec3(lEC + vEC);
			H = normalize(H);

			float NdotH = dot(nEC, H);
			if(NdotH < 0.0)
				NdotH = 0.0;
			specular = vec3(Ls * Ms * pow(NdotH, 50.0));
			nColor += specular * waveSpecular;
		}
	return nColor;
}

void main(void){
	if(perPixel)
		gl_FragColor.rgb = calcLight();
	else
		gl_FragColor = vColor;

}
