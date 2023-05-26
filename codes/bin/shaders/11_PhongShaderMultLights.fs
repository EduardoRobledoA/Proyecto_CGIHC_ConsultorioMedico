#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 ex_N; 

in vec3 vertexPosition_cameraspace;
in vec3 Normal_cameraspace;

uniform mat4 view;

uniform sampler2D texture_diffuse1;

uniform vec4 MaterialAmbientColor;
uniform vec4 MaterialDiffuseColor;
uniform vec4 MaterialSpecularColor;
uniform float transparency;

#define MAX_LIGHTS 10 // Númeor máximo de fuentes de iluminación.No hay memoria dinámica en shaders, por eso se determina aquí.
uniform int numLights;
// All ligths, dado a que se va a compartir con el MAIN, es UNIFORM.
uniform struct Light {
   vec3  Position;
   vec3  Direction;
   vec4  Color;
   vec4  Power;
   int   alphaIndex;
   float distance; // Sirve para atenuar la intensidad dada la distancia.
} allLights[MAX_LIGHTS];

// Creando función que recibe fuente de iluminación
vec4 ApplyLight(Light light, vec3 N, vec3 L, vec3 E) {
    
    float cosTheta = clamp( dot( N,L ), 0,1 );

    // Cálculo de componente difusa
    vec4 MaterialAmbColor = MaterialAmbientColor * light.Color;

    // Cálculo de componente especular
    // R (reflejo, se calcula aquí) dados N y L
    vec3 R = reflect(-L,N);

    float cosAlpha = clamp( dot( E,R ), 0,1 ); // E*R determina el ancho del espectacular.
    // Modelo de Phong. Elevando componente especular a una potencia.
    vec4 l_contribution = MaterialAmbColor  * light.Power / (light.distance * light.distance ) +
                    MaterialDiffuseColor * light.Color * light.Power * cosTheta / (light.distance * light.distance ) +
                    MaterialSpecularColor * light.Color * light.Power * pow(cosAlpha,light.alphaIndex) / (light.distance * light.distance );

    return l_contribution;
}

void main()
{    
    // Cálculo de componente difusa
    vec3 n = normalize( Normal_cameraspace );
    
    vec4 ex_color = vec4(0.0f);

    for(int i = 0; i < numLights; ++i){
        
        vec3 EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;
        vec3 LightPosition_cameraspace = ( view * vec4(allLights[i].Position,1)).xyz;
        vec3 LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
        vec3 e = normalize(EyeDirection_cameraspace);
        vec3 l = normalize( LightDirection_cameraspace );
        ex_color += ApplyLight(allLights[i], n, l, e); // Acumulación de colores que se van calculando.
    }
           
    ex_color.a = transparency;

    vec4 texel = texture(texture_diffuse1, TexCoords);

    FragColor = texel * ex_color;
}