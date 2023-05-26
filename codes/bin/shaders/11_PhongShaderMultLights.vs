#version 330 core
layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoords;
layout (location = 3) in vec3  tangent;
layout (location = 4) in vec3  bitangent;

out vec2 TexCoords;
out vec3 ex_N;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 eye;

out vec3 vertexPosition_cameraspace;
out vec3 Normal_cameraspace;

void main()
{

    vec4 PosL = vec4(aPos, 1.0f);
     //PosL.z += 0.1f * sin((PosL.x + time)*3); //amplitud, en z dado el perfil de blender
    //PosL.z += 0.5f * sin(PosL.y + time);

    gl_Position = projection * view * model * PosL;

    TexCoords = aTexCoords;  
    
    vertexPosition_cameraspace = ( view * model * vec4(aPos,1)).xyz;

    Normal_cameraspace = ( view * model * vec4(aNormal,0)).xyz;

    ex_N = aNormal;
}