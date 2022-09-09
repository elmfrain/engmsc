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
"uniform sampler2D u_textures[16];\n"
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
"uniform sampler2D u_textures[16];\n"
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
"uniform sampler2D u_textures[16];\n"
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
"uniform sampler2D u_textures[16];\n"
"\n"
"void main()\n"
"{\n"
"   vec4 textureColor = vec4(1.0);\n"
"   if(b_texID != 0) textureColor = texture(u_textures[int(b_texID) - 1], b_uv);\n"
"   o_fragColor = u_color * b_color * textureColor;\n"
"}\n"
;