//shader.vert
#define M_PI 3.1415926535897932384626433832795

varying vec4 vColor;

varying vec4 eye;
varying vec3 normal;

uniform vec3 waveAmbient;
uniform vec3 waveDiffuse;
uniform vec3 waveSpecular;

uniform mat4 mvMat, pMat;
uniform mat3 nMat;
uniform float t, x, z;
uniform bool grid;
uniform bool perPixel;

const float A1 = 0.25, k1 = 2.0 * M_PI, w1 = 0.25;
const float A2 = 0.25, k2 = 2.0 * M_PI, w2 = 0.25;

vec3 calcLight(){
	vec3 nColor = vec3(0.0);

	vec3 La, Ma, ambient, lEC, nEC, Ld, Md, Ls, Ms, vEC, H, diffuse, specular;
	//os  - object space, es - eye space, cd - clip space

	La = vec3(0.2);
	Ma = vec3(0.2);
	ambient = (La * Ma);
	nColor += ambient * waveAmbient;

	lEC = vec3(0.0, 0.0, 1.0);
	nEC = nMat * normalize(gl_Normal);

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
	vec4 esVert = mvMat * gl_Vertex;
	vec4 csVert = pMat * esVert;
	gl_Position = csVert;
	
	//The normal value to be sent to the fragment shader for per-pixel shading
	//Is essentially nEC
	normal = normalize(nMat * gl_Normal);
	vColor = vec4(calcLight(), 1.0);

	//Equivalent to: gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex
}