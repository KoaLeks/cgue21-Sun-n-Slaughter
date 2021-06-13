#version 430 core

const float levels = 10.0;

uniform float scaleXZ;
uniform float scaleY;
uniform vec3 lightPosition;
uniform sampler2D waterTexture;
uniform sampler2D sandTexture;
uniform sampler2D grassTexture;
uniform sampler2D stoneTexture;
uniform sampler2D snowTexture;

uniform sampler2D shadowMap;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform mat4 lightSpaceMatrix;
uniform float brightness;

const float regionMinWater = -scaleY * 0.125;
const float regionMaxWater = scaleY * 0.005;
const float regionMinSand = scaleY * 0.005;
const float regionMaxSand = scaleY * 0.3;
const float regionMinGrass = scaleY * 0.3;
const float regionMaxGrass = scaleY * 0.5;
const float regionMinStone = scaleY * 0.5;
const float regionMaxStone = scaleY * 0.8;
const float regionMinSnow = scaleY * 0.8;
const float regionMaxSnow = scaleY;

in vec4 teNormal;
in vec4 tePosition;
in vec2 teTextureCoordinate;
in vec4 teFragPosLightSpace;

out vec4 color;

//https://gamedev.stackexchange.com/questions/59797/glsl-shader-change-hue-saturation-brightness
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

float calculateRegionWeight(float regionMax, float regionMin){
    float height = teNormal.w;
    float regionRange = 0.0;
    float regionWeight = 0.0;
    regionRange = regionMax - regionMin;
    regionWeight = (regionRange - abs(height - regionMax)) / regionRange;
    return max(0.0, regionWeight);
}

vec4 generateTerrainColor(vec2 texCoord) {
    float regionWeight = 0.0;
	vec4 terrainColor = vec4(0);
	
    // Terrain water
    regionWeight = calculateRegionWeight(regionMaxWater, regionMinWater);
    terrainColor += regionWeight * texture(waterTexture, texCoord);

    // Terrain sand
    regionWeight = calculateRegionWeight(regionMaxSand, regionMinSand);
    terrainColor += regionWeight * texture(sandTexture, texCoord);

    // Terrain grass
    regionWeight = calculateRegionWeight(regionMaxGrass, regionMinGrass);
    terrainColor += regionWeight * texture(grassTexture, texCoord);

    // Terrain stone
    regionWeight = calculateRegionWeight(regionMaxStone, regionMinStone);
    terrainColor += regionWeight * texture(stoneTexture, texCoord);

    // Terrain snow
    regionWeight = calculateRegionWeight(regionMaxSnow, regionMinSnow);
    terrainColor += regionWeight * texture(snowTexture, texCoord);

	return terrainColor;
}
// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float shadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 shadowCoord = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // transform to [0,1] range
    shadowCoord = shadowCoord * 0.5 + 0.5;


    vec4 lightDir = vec4(lightPos - vec3(0), 1);
    float bias = max(0.005 * (1.0 - dot(teNormal, lightDir)), 0.001); 
    float shadow = 0.0;     
    vec2 texelSize = 1 / vec2(textureSize(shadowMap, 0));
	
    // dont shadow if its outside the shadowmap
    if (shadowCoord.z > 1.0){
        return 0.0;
    }
    
    int pcfRange = 3;
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

void main(){
	vec2 texCoord = tePosition.xz / (scaleXZ / 20);
    
    vec3 lightColor = vec3(1.0);
	vec3 lightDir = normalize(lightPosition - tePosition.xyz);
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
	float diff = max(dot(teNormal.xyz, lightDir), 0.0);	
	vec3 diffuse = diff * lightColor;
    
    // specular
    vec3 viewDir = normalize(viewPos - tePosition.xyz);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(teNormal.xyz, halfwayDir), 0.0), 64.0);
    vec3 specular = vec3(0); // spec * lightColor;    
	
    vec4 terrainColor = generateTerrainColor(texCoord);
	
    // cel shading
    vec3 terrainColorHSV = rgb2hsv(terrainColor.rgb);
    float terrainLevel = floor(terrainColorHSV.z * levels);
	terrainColorHSV.z = (terrainLevel / levels);
    terrainColor = vec4(hsv2rgb(terrainColorHSV), 1.0);
    
    // calculate shadow
    float shadow = shadowCalculation(teFragPosLightSpace); 
    //float shadow = shadowCalculation(lightSpaceMatrix * tePosition); 
    
    vec3 light = ambient + (diffuse + specular) * (1 - shadow);
	//light = floor(light * levels);
	//light = light / levels;
	
    vec3 result = brightness * light * terrainColor.rgb;
	color = vec4(result, 1.0);
	//color = vec4(teNormal, 1);
}