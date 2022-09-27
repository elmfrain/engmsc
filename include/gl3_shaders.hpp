static const char* POS_SHADER_vcode =
"#version 150 core\n"
"in vec3 i_pos;\n"
"uniform mat4 u_projectionMatrix;\n"
"uniform mat4 u_modelViewMatrix;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(i_pos, 1.0);\n"
"}\n"
;

static const char* POS_SHADER_fcode =
"#version 150 core\n"
"out vec4 o_fragColor;\n"
"uniform vec4 u_color;\n"
"uniform sampler2D u_textures[32];\n"
"uniform int u_maxTextures;\n"
"\n"
"void main()\n"
"{\n"
"   o_fragColor = u_color;\n"
"}\n"
;

static const char* POS_UV_SHADER_vcode =
"#version 150 core\n"
"in vec3 i_pos;\n"
"in vec2 i_uv;\n"
"out vec2 b_uv;\n"
"uniform mat4 u_projectionMatrix;\n"
"uniform mat4 u_modelViewMatrix;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(i_pos, 1.0);\n"
"   b_uv = i_uv;\n"
"}\n"
;

static const char* POS_UV_SHADER_fcode =
"#version 150 core\n"
"in vec2 b_uv;\n"
"out vec4 o_fragColor;\n"
"uniform vec4 u_color;\n"
"uniform sampler2D u_textures[32];\n"
"uniform int u_maxTextures;\n"
"\n"
"void main()\n"
"{\n"
"   o_fragColor = u_color * texture(u_textures[0], b_uv);\n"
"}\n"
;

static const char* POS_COLOR_SHADER_vcode =
"#version 150 core\n"
"in vec3 i_pos;\n"
"in vec4 i_color;\n"
"out vec4 b_color;\n"
"uniform mat4 u_projectionMatrix;\n"
"uniform mat4 u_modelViewMatrix;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(i_pos, 1.0);\n"
"   b_color = i_color;\n"
"}\n"
;

static const char* POS_COLOR_SHADER_fcode =
"#version 150 core\n"
"in vec4 b_color;\n"
"out vec4 o_fragColor;\n"
"uniform vec4 u_color;\n"
"uniform sampler2D u_textures[32];\n"
"uniform int u_maxTextures;\n"
"\n"
"void main()\n"
"{\n"
"   o_fragColor = u_color * b_color;\n"
"}\n"
;

static const char* POS_UV_COLOR_TEXID_SHADER_vcode =
"#version 150 core\n"
"in vec3 i_pos;\n"
"in vec2 i_uv;\n"
"in vec4 i_color;\n"
"in float i_texID;\n"
"out vec2 b_uv;\n"
"out vec4 b_color;\n"
"out float b_texID;\n"
"uniform mat4 u_projectionMatrix;\n"
"uniform mat4 u_modelViewMatrix;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(i_pos, 1.0);\n"
"   b_uv = i_uv;\n"
"   b_color = i_color;\n"
"   b_texID = i_texID;\n"
"}\n"
;

static const char* POS_UV_COLOR_TEXID_SHADER_fcode =
"#version 150 core\n"
"in vec2 b_uv;\n"
"in vec4 b_color;\n"
"in float b_texID;\n"
"out vec4 o_fragColor;\n"
"uniform vec4 u_color;\n"
"uniform sampler2D u_textures[32];\n"
"uniform int u_maxTextures;\n"
"\n"
"void main()\n"
"{\n"
"   vec4 textureColor = vec4(1.0);\n"
"   if(b_texID != 0) textureColor = texture(u_textures[int(b_texID) - 1], b_uv);\n"
"   o_fragColor = u_color * b_color * textureColor;\n"
"}\n"
;

static const char* UI_SHADER_vcode =
"#version 150 core\n"
"in vec3 i_pos;\n"
"in vec2 i_uv;\n"
"in vec4 i_color;\n"
"in float i_texID;\n"
"out vec2 b_uv;\n"
"out vec4 b_color;\n"
"out float b_texID;\n"
"uniform mat4 u_projectionMatrix;\n"
"uniform mat4 u_modelViewMatrix;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(i_pos, 1.0);\n"
"   b_uv = i_uv;\n"
"   b_color = i_color;\n"
"   b_texID = i_texID;\n"
"}\n"
;

static const char* UI_SHADER_fcode =
"#version 150 core\n"
"in vec2 b_uv;\n"
"in vec4 b_color;\n"
"in float b_texID;\n"
"out vec4 o_fragColor;\n"
"uniform vec4 u_color;\n"
"uniform sampler2D u_textures[32];\n"
"uniform int u_maxTextures;\n"
"\n"
"#define FILL_TOL 0.57\n"
"#define FILL_TOL_BOLD 0.47\n"
"#define AA_TOL 0.44\n"
"#define AA_TOL_BOLD 0.37\n"
"\n"
"void main()\n"
"{\n"
"   int texID = int(b_texID);\n"
"   vec4 textureColor = vec4(1.0);\n"
"   if(texID != 0) textureColor = texture(u_textures[min(texID, u_maxTextures) - 1], b_uv);\n"
"\n"
"   if(texID >= u_maxTextures)\n"
"   {\n"
"       bool isBold = texID - u_maxTextures == 1;\n"
"       float fillTol = isBold ? FILL_TOL_BOLD : FILL_TOL;\n"
"       float aaTol = isBold ? AA_TOL_BOLD : AA_TOL;\n"
"       if(textureColor.a > fillTol) o_fragColor = u_color * b_color;\n"
"       else if(textureColor.a > aaTol)\n"
"       {\n"
"           vec4 edge = vec4(1.0, 1.0, 1.0, smoothstep(aaTol, fillTol, textureColor.a));\n"
"           o_fragColor = u_color * b_color * edge;\n"
"       }\n"
"       else discard;\n"
"   }\n"
"   else o_fragColor = u_color * textureColor * b_color;\n"
"}\n"
;

static const char* GAUGE_NEEDLE_SHADER_vcode =
"#version 150 core\n"
"in vec3 i_pos;\n"
"in vec4 i_color;\n"
"out vec4 b_color;\n"
"uniform mat4 u_projectionMatrix;\n"
"uniform mat4 u_modelViewMatrix;\n"
"uniform float u_angleDelta;\n"
"uniform float u_numInstances;\n"
"\n"
"mat4 rotationMatrix(vec3 axis, float angle)\n"
"{\n"
"   axis = normalize(axis);\n"
"   float s = sin(angle);\n"
"   float c = cos(angle);\n"
"   float oc = 1.0 - c;\n"
"   \n"
"   return mat4(\n"
"   oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,\n"
"   oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,\n"
"   oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,\n"
"   0.0,                                0.0,                                0.0,                                1.0);\n"
"}\n"
"void main()\n"
"{\n"
"   float lerp = float(gl_InstanceID) / u_numInstances;\n"
"   mat4 modelview = u_modelViewMatrix * rotationMatrix(vec3(0.0, 0.0, 1.0), u_angleDelta * lerp);\n"
"   gl_Position = u_projectionMatrix * modelview * vec4(i_pos, 1.0);\n"
"   b_color = i_color;\n"
"}\n"
;

static const char* GAUGE_NEEDLE_SHADER_fcode =
"#version 150 core\n"
"in vec4 b_color;\n"
"out vec4 o_fragColor;\n"
"uniform vec4 u_color;\n"
"uniform sampler2D u_textures[32];\n"
"uniform int u_maxTextures;\n"
"\n"
"void main()\n"
"{\n"
"   o_fragColor = u_color * b_color;\n"
"}\n"
;

static const char* ENGINE2D_SHADER_vcode =
"#version 150 core\n"
"in vec3 i_pos;\n"
"in vec4 i_color;\n"
"out vec4 b_color;\n"
"uniform mat4 u_projectionMatrix;\n"
"uniform mat4 u_modelViewMatrix;\n"
"uniform float u_crankAngle;\n"
"uniform float u_crankAngleDelta;\n"
"uniform float u_numInstances;\n"
"uniform int u_partID;\n"
"\n"
"#define HALF_PI       1.570796326\n"
"#define PI            3.141592653\n"
"#define TWO_PI        6.283185307\n"
"#define THREE_HALF_PI 4.712388980\n"
"\n"
"mat4 rotationMatrix(vec3 axis, float angle)\n"
"{\n"
"   axis = normalize(axis);\n"
"   float s = sin(angle);\n"
"   float c = cos(angle);\n"
"   float oc = 1.0 - c;\n"
"   \n"
"   return mat4(\n"
"   oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,\n"
"   oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,\n"
"   oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,\n"
"   0.0,                                0.0,                                0.0,                                1.0);\n"
"}\n"
"mat4 translationMatrix(vec3 axis)\n"
"{\n"
"   return mat4(\n"
"   1.0,    0.0,    0.0,    0.0,\n"
"   0.0,    1.0,    0.0,    0.0,\n"
"   0.0,    0.0,    1.0,    0.0,\n"
"   axis.x, axis.y, axis.z, 1.0);\n"
"}\n"
"void piston(float crankAngle)\n"
"{\n"
"   crankAngle = -crankAngle + HALF_PI;\n"
"   float pistonY = \n"
"   sin(acos(cos(crankAngle) / 2.4)) * 1.2 + sin(crankAngle) / 2.0;\n"
"   \n"
"   mat4 modelview = u_modelViewMatrix * translationMatrix(vec3(0.0, pistonY, 0.0));\n"
"   gl_Position = u_projectionMatrix * modelview * vec4(i_pos, 1.0);\n"
"   b_color = i_color;\n"
"}\n"
"void conrod(float crankAngle)\n"
"{\n"
"   crankAngle = -crankAngle + HALF_PI;\n"
"   float pistonY = \n"
"   sin(acos(cos(crankAngle) / 2.4)) * 1.2 + sin(crankAngle) / 2.0;\n"
"   \n"
"   vec4 conrodPos = rotationMatrix(vec3(0.0, 0.0, -1.0), crankAngle - HALF_PI) * vec4(0.0, 0.5, 0.0, 1.0);\n"
"   \n"
"   float a = asin((pistonY - conrodPos.y) / 1.2);\n"
"   float conrodAngle = mod(crankAngle + HALF_PI, TWO_PI);\n"
"   conrodAngle = conrodAngle > PI ? a : PI - a;\n"
"   conrodAngle -= HALF_PI;\n"
"   \n"
"   mat4 modelview = u_modelViewMatrix * translationMatrix(vec3(conrodPos.xy, 0.0));\n"
"   modelview *= rotationMatrix(vec3(0.0, 0.0, -1.0), conrodAngle);\n"
"   gl_Position = u_projectionMatrix * modelview * vec4(i_pos, 1.0);\n"
"   b_color = i_color;\n"
"}\n"
"void intakeValve(float crankAngle)\n"
"{\n"
"   float duration = 2.217;\n"
"   float camAngle = mod((crankAngle - 1.004) / 2.0 - PI, TWO_PI);\n"
"   mat4 modelview = u_modelViewMatrix * translationMatrix(vec3(-0.2069, 2.0873, 0.0));\n"
"   modelview *= rotationMatrix(vec3(0.0, 0.0, 1.0), -0.1745);\n"
"   float x = 0.0;\n"
"   if(PI - 0.5 * duration < camAngle && camAngle < PI + 0.5 * duration)\n"
"   { x = cos(TWO_PI * (camAngle - PI) / duration) * -0.034 - 0.034; }\n"
"   modelview *= translationMatrix(vec3(0.0, x, 0.0));\n"
"   gl_Position = u_projectionMatrix * modelview * vec4(i_pos, 1.0);\n"
"   b_color = i_color;\n"
"}\n"
"void exhaustValve(float crankAngle)\n"
"{\n"
"   float duration = 2.217;\n"
"   float camAngle = mod((crankAngle + 1.004) / 2.0 - PI, TWO_PI);\n"
"   mat4 modelview = u_modelViewMatrix * translationMatrix(vec3(0.2158, 2.0853, 0.0));\n"
"   modelview *= rotationMatrix(vec3(0.0, 0.0, 1.0), 0.1745);\n"
"   float x = 0.0;\n"
"   if(PI - 0.5 * duration < camAngle && camAngle < PI + 0.5 * duration)\n"
"   { x = cos(TWO_PI * (camAngle - PI) / duration) * -0.034 - 0.034; }\n"
"   modelview *= translationMatrix(vec3(0.0, x, 0.0));\n"
"   gl_Position = u_projectionMatrix * modelview * vec4(i_pos, 1.0);\n"
"   b_color = i_color;\n"
"}\n"
"void main()\n"
"{\n"
"   float lerp = float(gl_InstanceID) / u_numInstances;\n"
"   float crankAngle = u_crankAngle + u_crankAngleDelta * lerp;\n"
"   \n"
"   if(u_partID == 1) { piston(crankAngle); return; }\n"
"   else if(u_partID == 2) { conrod(crankAngle); return; }\n"
"   else if(u_partID == 3) { intakeValve(crankAngle); return; }\n"
"   else if(u_partID == 4) { exhaustValve(crankAngle); return; }\n"
"\n"
"   mat4 modelview = u_modelViewMatrix * rotationMatrix(vec3(0.0, 0.0, 1.0), crankAngle - HALF_PI);\n"
"   gl_Position = u_projectionMatrix * modelview * vec4(i_pos, 1.0);\n"
"   b_color = i_color;\n"
"}\n"
;

static const char* ENGINE2D_SHADER_fcode =
"#version 150 core\n"
"in vec4 b_color;\n"
"out vec4 o_fragColor;\n"
"uniform vec4 u_color;\n"
"uniform sampler2D u_textures[32];\n"
"uniform int u_maxTextures;\n"
"\n"
"void main()\n"
"{\n"
"   o_fragColor = u_color * b_color;\n"
"}\n"
;