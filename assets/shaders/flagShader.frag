#version 460 core
in vec2 Tex;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    vec3 specular;    
    float shininess;
}; 

struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
    vec3 albedo = texture(material.diffuse, Tex).rgb;

    vec3 norm = normalize(Normal);
    if (!gl_FrontFacing) {
        norm = -norm;
    }
    
    vec3 lightDir = normalize(-light.direction);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    float backlight = max(dot(-norm, lightDir), 0.0);
    vec3 subsurface = backlight * light.diffuse * albedo * 0.4;
    
    float NdotL = dot(norm, lightDir);
    float wrappedDiff = (NdotL * 0.6 + 0.4);
    wrappedDiff = smoothstep(0.0, 1.0, wrappedDiff);
    
    float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 1.5);
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(norm, halfwayDir), 0.0);
    float spec = pow(NdotH, material.shininess * 0.3);
    
    vec2 weave = fract(Tex * 200.0);
    float weavePattern = (sin(weave.x * 6.28) + sin(weave.y * 6.28)) * 0.015 + 1.0;
    
    vec3 ambient = light.ambient * albedo;
    vec3 diffuse = light.diffuse * wrappedDiff * albedo;
    vec3 specular = light.specular * spec * material.specular * 0.2; 
    vec3 rimLight = fresnel * light.diffuse * 0.15;
    
    vec3 result = (ambient + diffuse * weavePattern + subsurface + rimLight) + specular;
    
    float luminance = dot(result, vec3(0.299, 0.587, 0.114));
    result = mix(vec3(luminance), result, 0.95);
    
    FragColor = vec4(result, 1.0);
}