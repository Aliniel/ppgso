#version 150
// A texture is expected as program attribute
uniform sampler2D Texture;
uniform vec3 lightPosition;
uniform vec3 viewPosition;

// The vertex shader fill feed this input
in vec2 FragTexCoord;
in vec4 normal;
in vec3 FragPos;

// The final color
out vec4 FragmentColor;

void main() {
  // Ambient light
  vec4 lightColor = vec4(1,1,1,1);
  float ambientStrength = 0.25f;
  vec4 ambientColor = ambientStrength * lightColor;

  // Diffuse light
  vec3 lightDirection = normalize(lightPosition - FragPos);
  float distance = distance(lightPosition, FragPos);
  float diff = max(dot(normal, vec4(lightDirection, 0.0f)), 0.0f);
  vec4 diffuseColor = 30.0f * diff * lightColor / pow(distance, 1.75f);

  // Speculiar light
  float specularStrength = 0.5f;
  vec3 viewDir = normalize(viewPosition - FragPos);
  vec3 reflectDir = reflect(-lightDirection, vec3(normal));
  float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 16);
  vec4 specularColor = specularStrength * spec * lightColor;

  // Lookup the color in Texture on coordinates given by fragTexCoord
  FragmentColor = texture(Texture, FragTexCoord) * (diffuseColor + ambientColor + specularColor);
}