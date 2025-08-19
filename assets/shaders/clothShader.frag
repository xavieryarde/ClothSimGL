#version 460 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D clothTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    // Base color (white cloth)
    vec3 baseColor = texture(clothTexture, TexCoord).rgb;
    
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-L, N);
    vec3 H = normalize(L + V);
    
    //  Silky material properties
    
    float fresnel = pow(1.0 - max(dot(N, V), 0.0), 2.0);
    
    float NdotL = dot(N, L);
    float softDiffuse = max(NdotL * 0.5 + 0.5, 0.0);
    softDiffuse = smoothstep(0.0, 1.0, softDiffuse);
    
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);
    
    float spec1 = pow(NdotH, 128.0);
    
    float spec2 = pow(NdotH, 32.0);
    
    vec3 tangent = normalize(vec3(1.0, 0.0, 0.0)); 
    vec3 bitangent = cross(N, tangent);
    
    float tDotH = dot(tangent, H);
    float bDotH = dot(bitangent, H);
    float anisoSpec = pow(max(abs(tDotH), 0.0), 60.0) * 0.3;
    
    vec3 silkTint = vec3(0.98, 0.99, 1.0);
    baseColor *= silkTint;
    
    vec3 ambient = 0.15 * baseColor;
    vec3 diffuse = softDiffuse * baseColor * 0.8;
    vec3 specular = (spec1 * 0.8 + spec2 * 0.3 + anisoSpec * 0.4) * vec3(1.0, 0.98, 0.96);
    
    vec3 rimLight = fresnel * vec3(0.9, 0.95, 1.0) * 0.3;
    
    vec3 result = ambient + diffuse + specular + rimLight;
    
    result = pow(result, vec3(0.9));
    
    FragColor = vec4(result, 1.0);
}