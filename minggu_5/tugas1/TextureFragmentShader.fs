#version 330 core
// out vec3 FragColor;

// Interpolated values from the vertex shaders
in vec3 FragPos;
in vec2 UV;

// used for lighting
in vec3 Normal;

// Ouput data
out vec4 outputColor;

// uniform vec3 lightPos;
// uniform vec3 viewPos;
// uniform vec3 lightColor;
// uniform vec3 objectColor;
uniform mat4 MVP;
uniform vec3 ViewPosition;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main(){

	// Output color = color of the texture at the specified UV
	vec3 color = texture( myTextureSampler, UV ).rgb;

	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 lightPos = vec3(1.0f, 1.0f, 1.0f);

	// ambient lighting
	float ambientStrength = 0.4f;
	vec3 ambient = ambientStrength * vec3(0.0f, 1.0f, 0.0f);

	// get normalized normal vector
	vec3 norm = normalize(Normal);

	// diffuse lighting
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1.0f, 1.0f, 1.0f);

	// specular lighting
	float specularStrength = 0.5f;
	vec3 viewDir = normalize(ViewPosition - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * vec3(1.0f, 1.0f, 1.0f);
	

	// calculate color result
	vec3 result = (ambient + diffuse + specular) * color;
	outputColor = vec4(result, 1.0f);
}