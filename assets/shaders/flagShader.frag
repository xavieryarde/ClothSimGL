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
    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, Tex).rgb;
  	
    // diffuse 
    vec3 norm = normalize(-Normal);
    if (!gl_FrontFacing) {
        norm = -norm;
    }
   
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, Tex).rgb;

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
} 
