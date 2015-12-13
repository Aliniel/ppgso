#version 150
// A texture is expected as program attribute
uniform sampler2D Texture;
uniform vec3 lightPosition;
uniform vec3 viewPosition;

// Mines
uniform vec3 mine1;
uniform vec3 mine2;
uniform vec3 mine3;


// The vertex shader fill feed this input
in vec2 FragTexCoord;
in vec4 normal;
in vec3 FragPos;

// The final color
out vec4 FragmentColor;

void main() {
  vec4 lightColor = vec4(1,1,1,1);
  vec4 mineColor = vec4(1,0,0,1);

  // Ambient light
  float ambientStrength = 0.25f;
  vec4 ambientColor = ambientStrength * lightColor;

  // Diffuse light
  vec4 diffuseColor = vec4(0,0,0,0);

  vec3 lightDirection = normalize(mine1 - FragPos);
  float dist = distance(mine1, FragPos);
  float diff = max(dot(normal, vec4(lightDirection, 0.0f)), 0.0f);
  diffuseColor += 30.0f * diff * mineColor / pow(dist, 2.25f);

  lightDirection = normalize(mine2 - FragPos);
  dist = distance(mine2, FragPos);
  diff = max(dot(normal, vec4(lightDirection, 0.0f)), 0.0f);
  diffuseColor += 30.0f * diff * mineColor / pow(dist, 2.25f);

  lightDirection = normalize(mine3 - FragPos);
  dist = distance(mine3, FragPos);
  diff = max(dot(normal, vec4(lightDirection, 0.0f)), 0.0f);
  diffuseColor += 30.0f * diff * mineColor / pow(dist, 2.25f);

  lightDirection = normalize(lightPosition - FragPos);
  dist = distance(lightPosition, FragPos);
  diff = max(dot(normal, vec4(lightDirection, 0.0f)), 0.0f);
  diffuseColor += 30.0f * diff * lightColor / pow(dist, 1.75f);

  // Speculiar light
  float specularStrength = 0.5f;
  vec3 viewDir = normalize(viewPosition - FragPos);
  vec3 reflectDir = reflect(-lightDirection, vec3(normal));
  float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 16);
  vec4 specularColor = specularStrength * spec * lightColor;

  // Lookup the color in Texture on coordinates given by fragTexCoord
  FragmentColor = texture(Texture, FragTexCoord) * (diffuseColor + ambientColor + specularColor);
}