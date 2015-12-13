#version 150
// The inputs will be fed by the vertex buffer objects
in vec3 Position;
in vec2 TexCoord;
in vec3 Normal;

// This will be passed to the fragment shader
out vec2 FragTexCoord;
out vec4 normal;
out vec3 FragPos;
out vec3 lightPosition;

// Matrices as program attributes
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

void main() {
  // Copy the input to the fragment shader
  FragTexCoord = TexCoord;

  // Calculate the final position on screen
  gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position, 1.0);

  // Normal in world coordinates.
  vec3 tempNormal = mat3(transpose(inverse(ModelMatrix))) * Normal;
  normal = vec4(tempNormal, 0.0f);

  FragPos = vec3(ModelMatrix * vec4(Position, 1.0f));
}