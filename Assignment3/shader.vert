//shader.vert
#define M_PI 3.1415926535897932384626433832795

varying vec4 vColor;
uniform float mass;

void main(void){
	vec4 osVert = gl_Vertex;
	vec4 esVert = gl_ModelViewMatrix * osVert;
	vec4 csVert = gl_ProjectionMatrix * esVert;
	gl_Position = csVert;
	
	//I like them going from blue to red
	vColor = vec4(mass, 0.3, 0.3, 1.0);
}