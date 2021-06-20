#version 430 core
/*
* Copyright 2019 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/

in VertexData {
	vec3 position_world;
	vec3 normal_world;
	vec2 uv;
} vert;

out vec4 color;

uniform vec3 camera_world;

uniform vec3 materialCoefficients; // x = ambient, y = diffuse, z = specular 
uniform float specularAlpha;
uniform layout(binding = 0) sampler2D diffuseTexture;
uniform float brightness;

// shadow map
uniform mat4 lightSpaceMatrix;
uniform vec3 lightPos;
uniform sampler2D shadowMap;
uniform bool showShadows;

uniform struct PointLight {
	vec3 color;
	vec3 position;
	vec3 attenuation;
} pointL;

float shadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 shadowCoord = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // transform to [0,1] range
    shadowCoord = 0.5 * (shadowCoord + 1.0);

    vec4 lightDir = vec4(lightPos - vec3(0), 1);
    float bias = max(0.005 * (1.0 - dot(vec4(vert.normal_world, 1), lightDir)), 0.0005); 
    float shadow = 0.0;     
    vec2 texelSize = 1 / vec2(textureSize(shadowMap, 0));
	
    // dont shadow if its outside the shadowmap
    if (shadowCoord.z > 1.0){
        return 0.0;
    }
    
    int pcfRange = 2;
    for (int y = -pcfRange; y <= pcfRange; y++) {
        for(int x = -pcfRange; x <= pcfRange; x++){
            vec2 offset = vec2(x,y) * texelSize;
            float pcfDepth = texture(shadowMap, shadowCoord.xy + offset).r;
            shadow += ((shadowCoord.z - bias) > pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= pow(2 * pcfRange + 1, 2);
      
    //for (int i = 0; i < 4; i++){
    //    int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
	//	float pcfDepth = (texture( shadowMap, vec2(shadowCoord.xy + poissonDisk[index]/700)).r); 
    //    shadow += 0.2 * ((shadowCoord.z - bias) > pcfDepth ? 1.0 : 0.0);
    //}

    return shadow;
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 phong(vec3 n, vec3 l, vec3 v, vec3 diffuseC, float diffuseF, vec3 specularC, float specularF, float alpha, bool attenuate, vec3 attenuation) {
	float d = length(l);
	l = normalize(l);
	float att = 1.0;	
	if(attenuate) att = 1.0f / (attenuation.x + d * attenuation.y + d * d * attenuation.z);
	vec3 r = reflect(-l, n);
	return (diffuseF * diffuseC * max(0, dot(n, l)) + specularF * specularC * pow(max(0, dot(r, v)), alpha)) * att; 
}

void main() {	
	//vec3 n = normalize(vert.normal_world);
	//vec3 v = normalize(camera_world - vert.position_world);
	
	//vec3 texColor = vec3(1, 1, 1);//texture(diffuseTexture, vert.uv).rgb;

	//color = vec4(texColor * materialCoefficients.x, 1); // ambient
	
	//color = vec4(0.8, 0.3, 0.6, 1);
	

	
	vec3 n = normalize(vert.normal_world);
	vec3 v = normalize(camera_world - vert.position_world);
	
	
	// calculate shadow
    float shadow;
	vec4 fragPosLightSpace = lightSpaceMatrix * vec4(vert.position_world, 1);
    if(showShadows){
        shadow = shadowCalculation(fragPosLightSpace); 
    } else {
        shadow = 0;
    }

	vec3 texColor = texture(diffuseTexture, vert.uv).rgb;
	// vec4(texColor * materialCoefficients.x, 1) //ambient
    
	// add directional light contribution
	//color.rgb += phong(n, -dirL.direction, v, dirL.color * texColor, materialCoefficients.y, dirL.color, materialCoefficients.z, specularAlpha, false, vec3(0));

	//color.rgb +=  phong(n, pointL.position - vert.position_world, v, pointL.color * texColor, materialCoefficients.y, pointL.color, materialCoefficients.z, specularAlpha, false,  vec3(0));
	//vec3 light = (materialCoefficients.x + (1-shadow) * phong(n, (pointL.position - vert.position_world), v, pointL.color * texColor, materialCoefficients.y, pointL.color, materialCoefficients.z, specularAlpha, false,  vec3(0)));
	

    
	vec3 lightDir = normalize(pointL.position - vert.position_world);
	// ambient
    vec3 ambient = materialCoefficients.x * pointL.color;
    
    // diffuse
	float diff = max(dot(n, lightDir), 0.0);	
	vec3 diffuse = diff * materialCoefficients.y * pointL.color;

    // specular
    vec3 viewDir = normalize(camera_world - vert.position_world);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(n, halfwayDir), 0.0), specularAlpha);
    vec3 specular =  spec * materialCoefficients.z * pointL.color;   
    
    
    // cel shading
    float levels = 8.0;
    vec3 texColorHSV = rgb2hsv(texColor);
    float texLevel = floor(texColorHSV.z * levels);
	texColorHSV.z = (texLevel / levels);
    texColor = hsv2rgb(texColorHSV);
    
	
    vec3 light = ambient + ((diffuse + specular) * (1-shadow));
	vec3 result = brightness * texColor * light; 
	color = vec4(result, 1);
}