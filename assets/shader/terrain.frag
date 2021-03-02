#version 430 core

const float levels = 10.0;

const float regionMinWater = -100;
const float regionMaxWater = 10;
const float regionMinSand = 10;
const float regionMaxSand = 250;
const float regionMinGrass = 200;
const float regionMaxGrass = 400;
const float regionMinStone = 400;
const float regionMaxStone = 700;
const float regionMinSnow = 600;
const float regionMaxSnow = 800;

// big terrain
//const float regionMinWater = -100;
//const float regionMaxWater = 100;
//const float regionMinSand = 100;
//const float regionMaxSand = 2500;
//const float regionMinGrass = 2000;
//const float regionMaxGrass = 4000;
//const float regionMinStone = 4000;
//const float regionMaxStone = 7000;
//const float regionMinSnow = 6000;
//const float regionMaxSnow = 8000;

uniform float scaleXZ;
uniform vec3 lightPosition;
uniform sampler2D waterTexture;
uniform sampler2D sandTexture;
uniform sampler2D grassTexture;
uniform sampler2D stoneTexture;
uniform sampler2D snowTexture;

in vec4 teNormal;
in vec4 tePosition;
in vec2 teTextureCoordinate;

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

void main(){
	vec2 texCoord = tePosition.xz / (scaleXZ / 20);
    
    vec3 lightColor = vec3(0.75);
	vec3 lightDir = normalize(lightPosition - tePosition.xyz);
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
	float diff = max(dot(teNormal.xyz, lightDir), 0.0);	
	vec3 diffuse = diff * lightColor;
	
    vec4 terrainColor = generateTerrainColor(texCoord);
	
    // cel shading
    vec3 terrainColorHSV = rgb2hsv(terrainColor.rgb);
    float terrainLevel = floor(terrainColorHSV.z * levels);
	terrainColorHSV.z = (terrainLevel / levels);
    terrainColor = vec4(hsv2rgb(terrainColorHSV), 1.0);

    vec3 light = ambient + diffuse;
	light = floor(light * levels);
	light = light / levels;
	
    vec3 result = light * terrainColor.rgb; //objectColor;
	color = vec4(result, 1.0);
	//color = vec4(teNormal, 1);
}