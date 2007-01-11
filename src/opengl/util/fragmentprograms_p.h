#ifndef FRAGMENTPROGRAMS_H
#define FRAGMENTPROGRAMS_H

enum FragmentVariable {
    VAR_BRUSH_TEXTURE,
    VAR_LINEAR,
    VAR_INV_MATRIX_M1,
    VAR_INV_MASK_SIZE,
    VAR_INV_MATRIX_M2,
    VAR_PORTERDUFF_AB,
    VAR_MASK_CHANNEL,
    VAR_ELLIPSE_OFFSET,
    VAR_PORTERDUFF_XYZ,
    VAR_INV_DST_SIZE,
    VAR_MASK_TEXTURE,
    VAR_DST_TEXTURE,
    VAR_PALETTE,
    VAR_MASK_OFFSET,
    VAR_INV_BRUSH_TEXTURE_SIZE,
    VAR_FMP2_M_RADIUS2,
    VAR_FMP,
    VAR_INV_MATRIX_M0,
    VAR_ANGLE,
};

enum FragmentBrushType {
    FRAGMENT_PROGRAM_BRUSH_SOLID,
    FRAGMENT_PROGRAM_BRUSH_RADIAL,
    FRAGMENT_PROGRAM_BRUSH_CONICAL,
    FRAGMENT_PROGRAM_BRUSH_LINEAR,
    FRAGMENT_PROGRAM_BRUSH_TEXTURE,
    FRAGMENT_PROGRAM_BRUSH_PATTERN,
};

enum FragmentCompositionModeType {
    COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
    COMPOSITION_MODE_BLEND_MODE_MASK,
    COMPOSITION_MODE_BLEND_MODE_NOMASK,
};

enum FragmentMaskType {
    FRAGMENT_PROGRAM_MASK_TRAPEZOID_AA,
    FRAGMENT_PROGRAM_MASK_ELLIPSE_AA,
};

static const unsigned int num_fragment_variables = 19;

static const unsigned int num_fragment_brushes = 6;
static const unsigned int num_fragment_composition_modes = 3;
static const unsigned int num_fragment_masks = 2;

static const char *FragmentProgram_FRAGMENT_PROGRAM_MASK_TRAPEZOID_AA =
    "!!ARBfp1.0\n"
    "PARAM c[1] = { { 0.5, 2 } };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "TEMP R4;\n"
    "ADD R4.x, fragment.position, c[0];\n"
    "ADD R0.y, fragment.position, -c[0].x;\n"
    "MAX R2.x, R0.y, fragment.texcoord[0].y;\n"
    "ADD R0.x, fragment.position.y, c[0];\n"
    "MIN R2.y, R0.x, fragment.texcoord[0].x;\n"
    "ADD R3.x, fragment.position, -c[0];\n"
    "ADD R1.zw, -fragment.texcoord[0], -fragment.texcoord[0];\n"
    "MOV R3.y, R4.x;\n"
    "MOV R0.yw, R2.x;\n"
    "MOV R0.xz, R2.y;\n"
    "MAD R0, fragment.texcoord[1].xxzz, R0, fragment.texcoord[1].yyww;\n"
    "MAD R1.xy, fragment.position.x, c[0].y, -R0.zwzw;\n"
    "MOV R0.w, R1.x;\n"
    "MOV R1.x, R0.y;\n"
    "MOV R0.z, R0.x;\n"
    "SGE R2.zw, R1.xyxy, R0;\n"
    "MAX R0.xy, R0.zwzw, R1;\n"
    "MIN R0.zw, R0, R1.xyxy;\n"
    "MAD R2.zw, R2, R1, fragment.texcoord[0];\n"
    "ADD R1, R3.xyxy, -R0.zzww;\n"
    "MAD R1, R1, R2.zzww, R2.x;\n"
    "ADD R3.zw, R0.xyxy, R0;\n"
    "ADD R3.y, R2, -R2.x;\n"
    "ADD R2.zw, R1.xyyw, -R2.x;\n"
    "ADD R4.zw, R4.x, -R0;\n"
    "MUL R2.zw, R4, R2;\n"
    "ADD R4.zw, R1.xyyw, R1.xyxz;\n"
    "ADD R1.xz, R2.y, -R1;\n"
    "MAD R2.zw, -R2, c[0].x, R3.y;\n"
    "MAD R3.zw, R3, c[0].x, -R3.x;\n"
    "MAD R3.zw, R3, R3.y, -R2;\n"
    "ADD R1.y, R4.x, -R3.x;\n"
    "MAD R4.zw, -R4, c[0].x, R2.y;\n"
    "MUL R4.zw, R4, R1.y;\n"
    "ADD R1.yw, R0.xxzy, -R3.x;\n"
    "MUL R1.xy, R1.xzzw, R1.ywzw;\n"
    "MAD R1.zw, R1.xyxy, c[0].x, -R4;\n"
    "SGE R1.xy, R4.x, R0;\n"
    "MUL R1.zw, R1.xyxy, R1;\n"
    "MAD R1.xy, R1, R3.zwzw, R2.zwzw;\n"
    "SGE R2.zw, R3.x, R0;\n"
    "ADD R1.zw, R4, R1;\n"
    "ADD R1.zw, R1, -R1.xyxy;\n"
    "MAD R1.xy, R2.zwzw, R1.zwzw, R1;\n"
    "ADD R1.xy, R1, -R3.y;\n"
    "SGE R0.zw, R4.x, R0;\n"
    "MAD R0.zw, R0, R1.xyxy, R3.y;\n"
    "SGE R0.xy, R0, R3.x;\n"
    "MUL R0.xy, R0.zwzw, R0;\n"
    "ADD R0.x, R3.y, -R0;\n"
    "SGE R0.z, R2.y, R2.x;\n"
    "ADD R0.x, R0, -R0.y;\n"
    "MUL result.color, R0.x, R0.z;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_MASK_ELLIPSE_AA =
    "!!ARBfp1.0\n"
    "PARAM c[6] = { program.local[0..3],\n"
    "		{ -2, 1, -0.5, 2 },\n"
    "		{ 3 } };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "ADD R0.xy, fragment.position, c[0];\n"
    "MUL R1.xyz, R0.y, c[2];\n"
    "MAD R0.xyz, R0.x, c[1], R1;\n"
    "ADD R0.xyz, R0, c[3];\n"
    "RCP R2.z, R0.z;\n"
    "MUL R0.zw, R0.xyxy, R2.z;\n"
    "MUL R2.xy, R0.zwzw, fragment.texcoord[0];\n"
    "MOV R1.xy, c[1];\n"
    "MOV R1.zw, c[2].xyxy;\n"
    "MOV R0.x, c[1].z;\n"
    "MOV R0.y, c[2].z;\n"
    "MAD R0, -R0.xyxy, R0.zzww, R1.xzyw;\n"
    "MUL R1.xy, R2, fragment.texcoord[0];\n"
    "MUL R0, R0, R2.z;\n"
    "MUL R1.xy, R1, c[4].x;\n"
    "MUL R1.zw, R1.xyxy, R0.xyxz;\n"
    "MUL R0.xy, R1, R0.ywzw;\n"
    "ADD R0.w, R0.x, R0.y;\n"
    "MUL R0.xy, R2, R2;\n"
    "ADD R0.x, R0, R0.y;\n"
    "ADD R0.z, R1, R1.w;\n"
    "MUL R0.zw, R0, R0;\n"
    "ADD R0.y, R0.z, R0.w;\n"
    "RSQ R0.y, R0.y;\n"
    "ADD R0.x, -R0, c[4].y;\n"
    "MAD_SAT R0.x, R0.y, R0, -c[4].z;\n"
    "MUL R0.y, -R0.x, c[4].w;\n"
    "ADD R0.y, R0, c[5].x;\n"
    "MUL R0.x, R0, R0;\n"
    "MUL result.color, R0.x, R0.y;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_SOLID_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[7] = { program.local[0..3],\n"
    "		{ 1 },\n"
    "		program.local[5..6] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "MUL R0.xy, fragment.position, c[3];\n"
    "TEX R1, R0, texture[0], 2D;\n"
    "MUL R0.xyz, R1, c[6].y;\n"
    "MUL R2.xyz, R0, fragment.color.primary.w;\n"
    "MUL R0.xyz, fragment.color.primary, c[6].x;\n"
    "MAD R2.xyz, R0, R1.w, R2;\n"
    "ADD R3.xy, fragment.position, c[0];\n"
    "ADD R0.w, -R1, c[4].x;\n"
    "MUL R0.xyz, fragment.color.primary, c[5].y;\n"
    "MAD R2.xyz, R0, R0.w, R2;\n"
    "MUL R0.xyz, R1, c[5].z;\n"
    "ADD R0.w, -fragment.color.primary, c[4].x;\n"
    "MAD R2.xyz, R0, R0.w, R2;\n"
    "ADD R0.y, -R1.w, c[4].x;\n"
    "MUL R0.x, fragment.color.primary.w, R1.w;\n"
    "MUL R0.y, fragment.color.primary.w, R0;\n"
    "MUL R0.z, R1.w, R0.w;\n"
    "DP3 R2.w, R0, c[5];\n"
    "MUL R3.xy, R3, c[1];\n"
    "TEX R0, R3, texture[1], 2D;\n"
    "ADD R2, R2, -R1;\n"
    "DP4 R0.x, R0, c[2];\n"
    "MAD result.color, R0.x, R2, R1;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_SOLID_COMPOSITION_MODE_BLEND_MODE_MASK =
    "!!ARBfp1.0\n"
    "PARAM c[3] = { program.local[0..2] };\n"
    "TEMP R0;\n"
    "ADD R0.xy, fragment.position, c[0];\n"
    "MUL R0.xy, R0, c[1];\n"
    "TEX R0, R0, texture[0], 2D;\n"
    "DP4 R0.x, R0, c[2];\n"
    "MUL result.color, fragment.color.primary, R0.x;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_SOLID_COMPOSITION_MODE_BLEND_MODE_NOMASK =
    "!!ARBfp1.0\n"
    "PARAM c[1] = { program.local[0] };\n"
    "MOV result.color, fragment.color.primary;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_RADIAL_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[12] = { program.local[0..6],\n"
    "		{ 2, 4, 1 },\n"
    "		program.local[8..11] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "MUL R0.xyz, fragment.position.y, c[4];\n"
    "MAD R0.xyz, fragment.position.x, c[3], R0;\n"
    "ADD R0.xyz, R0, c[5];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "MUL R0.zw, R0.xyxy, R0.xyxy;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, R0, c[6];\n"
    "ADD R0.x, R0, R0.y;\n"
    "MUL R0.z, c[8].x, -R0;\n"
    "MUL R0.y, R0.z, c[7];\n"
    "MUL R0.x, R0, c[7];\n"
    "MAD R0.y, R0.x, R0.x, -R0;\n"
    "RSQ R0.z, R0.y;\n"
    "RCP R0.z, R0.z;\n"
    "ADD R0.x, -R0, R0.z;\n"
    "MUL R0.zw, fragment.position.xyxy, c[9].xyxy;\n"
    "TEX R1, R0.zwzw, texture[0], 2D;\n"
    "MOV R0.y, c[7].x;\n"
    "MUL R0.y, c[8].x, R0;\n"
    "RCP R0.y, R0.y;\n"
    "MUL R0.x, R0, R0.y;\n"
    "TEX R0, R0, texture[2], 1D;\n"
    "MUL R2.xyz, R1, c[11].y;\n"
    "MUL R3.xyz, R2, R0.w;\n"
    "MUL R2.xyz, R0, c[11].x;\n"
    "MAD R2.xyz, R2, R1.w, R3;\n"
    "ADD R3.xy, fragment.position, c[0];\n"
    "ADD R2.w, -R1, c[7].z;\n"
    "MUL R0.xyz, R0, c[10].y;\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "MUL R0.xyz, R1, c[10].z;\n"
    "ADD R3.z, -R0.w, c[7];\n"
    "MAD R2.xyz, R0, R3.z, R2;\n"
    "MUL R0.y, R0.w, R2.w;\n"
    "MUL R0.x, R0.w, R1.w;\n"
    "MUL R0.z, R1.w, R3;\n"
    "DP3 R2.w, R0, c[10];\n"
    "MUL R3.xy, R3, c[1];\n"
    "TEX R0, R3, texture[1], 2D;\n"
    "ADD R2, R2, -R1;\n"
    "DP4 R0.x, R0, c[2];\n"
    "MAD result.color, R0.x, R2, R1;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_RADIAL_COMPOSITION_MODE_BLEND_MODE_MASK =
    "!!ARBfp1.0\n"
    "PARAM c[9] = { program.local[0..3],\n"
    "		{ 2, 4 },\n"
    "		program.local[5..8] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "MUL R0.xyz, fragment.position.y, c[1];\n"
    "MAD R0.xyz, fragment.position.x, c[0], R0;\n"
    "ADD R0.xyz, R0, c[2];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "MUL R0.zw, R0.xyxy, R0.xyxy;\n"
    "MUL R0.xy, R0, c[3];\n"
    "ADD R0.z, R0, R0.w;\n"
    "ADD R0.x, R0, R0.y;\n"
    "MUL R0.z, c[5].x, -R0;\n"
    "MUL R0.y, R0.z, c[4];\n"
    "MUL R0.x, R0, c[4];\n"
    "MAD R0.y, R0.x, R0.x, -R0;\n"
    "RSQ R0.y, R0.y;\n"
    "RCP R0.y, R0.y;\n"
    "ADD R1.x, -R0, R0.y;\n"
    "MOV R0.z, c[4].x;\n"
    "MUL R0.z, c[5].x, R0;\n"
    "RCP R1.y, R0.z;\n"
    "ADD R0.xy, fragment.position, c[6];\n"
    "MUL R0.xy, R0, c[7];\n"
    "TEX R0, R0, texture[0], 2D;\n"
    "MUL R1.x, R1, R1.y;\n"
    "DP4 R1.y, R0, c[8];\n"
    "TEX R0, R1, texture[1], 1D;\n"
    "MUL result.color, R0, R1.y;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_RADIAL_COMPOSITION_MODE_BLEND_MODE_NOMASK =
    "!!ARBfp1.0\n"
    "PARAM c[6] = { program.local[0..3],\n"
    "		{ 2, 4 },\n"
    "		program.local[5] };\n"
    "TEMP R0;\n"
    "MUL R0.xyz, fragment.position.y, c[1];\n"
    "MAD R0.xyz, fragment.position.x, c[0], R0;\n"
    "ADD R0.xyz, R0, c[2];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "MUL R0.zw, R0.xyxy, R0.xyxy;\n"
    "MUL R0.xy, R0, c[3];\n"
    "ADD R0.x, R0, R0.y;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.z, c[5].x, -R0;\n"
    "MUL R0.y, R0.z, c[4];\n"
    "MUL R0.x, R0, c[4];\n"
    "MAD R0.y, R0.x, R0.x, -R0;\n"
    "MOV R0.z, c[4].x;\n"
    "RSQ R0.y, R0.y;\n"
    "MUL R0.z, c[5].x, R0;\n"
    "RCP R0.y, R0.y;\n"
    "RCP R0.z, R0.z;\n"
    "ADD R0.x, -R0, R0.y;\n"
    "MUL R0.x, R0, R0.z;\n"
    "TEX result.color, R0, texture[0], 1D;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_CONICAL_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[13] = { program.local[0..5],\n"
    "		{ 0.0020000001, 9.9999997e-10, 0.1963, 0.9817 },\n"
    "		{ 2.3561945, 0.78539819, -1, 1 },\n"
    "		program.local[8],\n"
    "		{ 0.15915494 },\n"
    "		program.local[10..12] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "MUL R0.xyz, fragment.position.y, c[4];\n"
    "MAD R0.xyz, fragment.position.x, c[3], R0;\n"
    "ADD R0.xyz, R0, c[5];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "ABS R0.w, R0.x;\n"
    "ABS R0.z, R0.y;\n"
    "ADD R0.z, R0, -R0.w;\n"
    "ADD R0.w, R0.y, c[6].x;\n"
    "ABS R0.z, R0;\n"
    "CMP R0.y, -R0.z, R0, R0.w;\n"
    "ABS R0.z, -R0.y;\n"
    "ADD R0.z, R0, c[6].y;\n"
    "ADD R0.w, R0.x, R0.z;\n"
    "ADD R1.x, R0.z, -R0;\n"
    "RCP R1.y, R0.w;\n"
    "RCP R1.x, R1.x;\n"
    "MUL R0.w, R0, R1.x;\n"
    "ADD R0.z, R0.x, -R0;\n"
    "MUL R0.z, R0, R1.y;\n"
    "CMP R0.z, R0.x, R0.w, R0;\n"
    "MUL R0.w, R0.z, R0.z;\n"
    "MOV R1.x, c[7].y;\n"
    "CMP R0.x, R0, c[7], R1;\n"
    "MAD R0.w, R0, c[6].z, -c[6];\n"
    "MAD R0.x, R0.w, R0.z, R0;\n"
    "CMP R0.y, -R0, c[7].z, c[7].w;\n"
    "MAD R0.x, R0, R0.y, c[8];\n"
    "MUL R0.x, R0, c[9];\n"
    "FLR R0.y, R0.x;\n"
    "MUL R0.zw, fragment.position.xyxy, c[10].xyxy;\n"
    "TEX R1, R0.zwzw, texture[0], 2D;\n"
    "ADD R0.x, R0, -R0.y;\n"
    "TEX R0, R0, texture[2], 1D;\n"
    "MUL R2.xyz, R1, c[12].y;\n"
    "MUL R3.xyz, R2, R0.w;\n"
    "MUL R2.xyz, R0, c[12].x;\n"
    "MAD R2.xyz, R2, R1.w, R3;\n"
    "ADD R3.xy, fragment.position, c[0];\n"
    "ADD R2.w, -R1, c[7];\n"
    "MUL R0.xyz, R0, c[11].y;\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "MUL R0.xyz, R1, c[11].z;\n"
    "ADD R3.z, -R0.w, c[7].w;\n"
    "MAD R2.xyz, R0, R3.z, R2;\n"
    "MUL R0.y, R0.w, R2.w;\n"
    "MUL R0.x, R0.w, R1.w;\n"
    "MUL R0.z, R1.w, R3;\n"
    "DP3 R2.w, R0, c[11];\n"
    "MUL R3.xy, R3, c[1];\n"
    "TEX R0, R3, texture[1], 2D;\n"
    "ADD R2, R2, -R1;\n"
    "DP4 R0.x, R0, c[2];\n"
    "MAD result.color, R0.x, R2, R1;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_CONICAL_COMPOSITION_MODE_BLEND_MODE_MASK =
    "!!ARBfp1.0\n"
    "PARAM c[10] = { program.local[0..2],\n"
    "		{ 0.0020000001, 9.9999997e-10, 0.1963, 0.9817 },\n"
    "		{ 2.3561945, 0.78539819, -1, 1 },\n"
    "		program.local[5],\n"
    "		{ 0.15915494 },\n"
    "		program.local[7..9] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "MUL R0.xyz, fragment.position.y, c[1];\n"
    "MAD R0.xyz, fragment.position.x, c[0], R0;\n"
    "ADD R0.xyz, R0, c[2];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "ABS R0.w, R0.x;\n"
    "ABS R0.z, R0.y;\n"
    "ADD R0.z, R0, -R0.w;\n"
    "ADD R0.w, R0.y, c[3].x;\n"
    "ABS R0.z, R0;\n"
    "CMP R0.y, -R0.z, R0, R0.w;\n"
    "ABS R0.z, -R0.y;\n"
    "ADD R0.z, R0, c[3].y;\n"
    "ADD R0.w, R0.x, R0.z;\n"
    "ADD R1.x, R0.z, -R0;\n"
    "RCP R1.y, R0.w;\n"
    "RCP R1.x, R1.x;\n"
    "MUL R0.w, R0, R1.x;\n"
    "ADD R0.z, R0.x, -R0;\n"
    "MUL R0.z, R0, R1.y;\n"
    "CMP R0.z, R0.x, R0.w, R0;\n"
    "MUL R0.w, R0.z, R0.z;\n"
    "MOV R1.x, c[4].y;\n"
    "CMP R0.x, R0, c[4], R1;\n"
    "MAD R0.w, R0, c[3].z, -c[3];\n"
    "MAD R0.x, R0.w, R0.z, R0;\n"
    "CMP R0.y, -R0, c[4].z, c[4].w;\n"
    "MAD R0.z, R0.x, R0.y, c[5].x;\n"
    "MUL R1.x, R0.z, c[6];\n"
    "FLR R1.y, R1.x;\n"
    "ADD R0.xy, fragment.position, c[7];\n"
    "MUL R0.xy, R0, c[8];\n"
    "TEX R0, R0, texture[0], 2D;\n"
    "ADD R1.x, R1, -R1.y;\n"
    "DP4 R1.y, R0, c[9];\n"
    "TEX R0, R1, texture[1], 1D;\n"
    "MUL result.color, R0, R1.y;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_CONICAL_COMPOSITION_MODE_BLEND_MODE_NOMASK =
    "!!ARBfp1.0\n"
    "PARAM c[7] = { program.local[0..2],\n"
    "		{ 0.0020000001, 9.9999997e-10, 0.1963, 0.9817 },\n"
    "		{ 2.3561945, 0.78539819, -1, 1 },\n"
    "		program.local[5],\n"
    "		{ 0.15915494 } };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "MUL R0.xyz, fragment.position.y, c[1];\n"
    "MAD R0.xyz, fragment.position.x, c[0], R0;\n"
    "ADD R0.xyz, R0, c[2];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "ABS R0.w, R0.x;\n"
    "ABS R0.z, R0.y;\n"
    "ADD R0.z, R0, -R0.w;\n"
    "ADD R0.w, R0.y, c[3].x;\n"
    "ABS R0.z, R0;\n"
    "CMP R0.y, -R0.z, R0, R0.w;\n"
    "ABS R0.z, -R0.y;\n"
    "ADD R0.z, R0, c[3].y;\n"
    "ADD R0.w, R0.x, R0.z;\n"
    "ADD R1.x, R0.z, -R0;\n"
    "RCP R1.x, R1.x;\n"
    "RCP R1.y, R0.w;\n"
    "MUL R0.w, R0, R1.x;\n"
    "ADD R0.z, R0.x, -R0;\n"
    "MUL R0.z, R0, R1.y;\n"
    "CMP R0.z, R0.x, R0.w, R0;\n"
    "MUL R0.w, R0.z, R0.z;\n"
    "MOV R1.x, c[4].y;\n"
    "CMP R0.y, -R0, c[4].z, c[4].w;\n"
    "MAD R0.w, R0, c[3].z, -c[3];\n"
    "CMP R0.x, R0, c[4], R1;\n"
    "MAD R0.x, R0.w, R0.z, R0;\n"
    "MAD R0.x, R0, R0.y, c[5];\n"
    "MUL R0.x, R0, c[6];\n"
    "FLR R0.y, R0.x;\n"
    "ADD R0.x, R0, -R0.y;\n"
    "TEX result.color, R0, texture[0], 1D;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_LINEAR_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[11] = { program.local[0..7],\n"
    "		{ 1 },\n"
    "		program.local[9..10] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "MUL R0.xyz, fragment.position.y, c[4];\n"
    "MAD R0.xyz, fragment.position.x, c[3], R0;\n"
    "ADD R0.xyz, R0, c[5];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "MUL R0.xy, R0, c[6];\n"
    "ADD R0.x, R0, R0.y;\n"
    "MUL R0.zw, fragment.position.xyxy, c[7].xyxy;\n"
    "TEX R1, R0.zwzw, texture[0], 2D;\n"
    "MUL R0.x, R0, c[6].z;\n"
    "TEX R0, R0, texture[2], 1D;\n"
    "MUL R2.xyz, R1, c[10].y;\n"
    "MUL R3.xyz, R2, R0.w;\n"
    "MUL R2.xyz, R0, c[10].x;\n"
    "MAD R2.xyz, R2, R1.w, R3;\n"
    "ADD R3.xy, fragment.position, c[0];\n"
    "MUL R0.xyz, R0, c[9].y;\n"
    "ADD R2.w, -R1, c[8].x;\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "MUL R0.xyz, R1, c[9].z;\n"
    "ADD R2.w, -R0, c[8].x;\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "ADD R0.y, -R1.w, c[8].x;\n"
    "MUL R0.z, R1.w, R2.w;\n"
    "MUL R0.x, R0.w, R1.w;\n"
    "MUL R0.y, R0.w, R0;\n"
    "DP3 R2.w, R0, c[9];\n"
    "MUL R3.xy, R3, c[1];\n"
    "TEX R0, R3, texture[1], 2D;\n"
    "ADD R2, R2, -R1;\n"
    "DP4 R0.x, R0, c[2];\n"
    "MAD result.color, R0.x, R2, R1;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_LINEAR_COMPOSITION_MODE_BLEND_MODE_MASK =
    "!!ARBfp1.0\n"
    "PARAM c[7] = { program.local[0..6] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "MUL R0.xyz, fragment.position.y, c[1];\n"
    "MAD R0.xyz, fragment.position.x, c[0], R0;\n"
    "ADD R0.xyz, R0, c[2];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.zw, R0.xyxy, R0.z;\n"
    "MUL R0.zw, R0, c[3].xyxy;\n"
    "ADD R1.x, R0.z, R0.w;\n"
    "ADD R0.xy, fragment.position, c[4];\n"
    "MUL R0.xy, R0, c[5];\n"
    "TEX R0, R0, texture[0], 2D;\n"
    "DP4 R1.y, R0, c[6];\n"
    "MUL R1.x, R1, c[3].z;\n"
    "TEX R0, R1, texture[1], 1D;\n"
    "MUL result.color, R0, R1.y;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_LINEAR_COMPOSITION_MODE_BLEND_MODE_NOMASK =
    "!!ARBfp1.0\n"
    "PARAM c[4] = { program.local[0..3] };\n"
    "TEMP R0;\n"
    "MUL R0.xyz, fragment.position.y, c[1];\n"
    "MAD R0.xyz, fragment.position.x, c[0], R0;\n"
    "ADD R0.xyz, R0, c[2];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "MUL R0.xy, R0, c[3];\n"
    "ADD R0.x, R0, R0.y;\n"
    "MUL R0.x, R0, c[3].z;\n"
    "TEX result.color, R0, texture[0], 1D;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_TEXTURE_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[11] = { program.local[0..7],\n"
    "		{ 1 },\n"
    "		program.local[9..10] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "MUL R0.xyz, fragment.position.y, c[4];\n"
    "MAD R0.xyz, fragment.position.x, c[3], R0;\n"
    "ADD R0.xyz, R0, c[5];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "MUL R0.xy, R0, c[6];\n"
    "MUL R0.zw, fragment.position.xyxy, c[7].xyxy;\n"
    "TEX R1, R0.zwzw, texture[0], 2D;\n"
    "MOV R0.y, -R0;\n"
    "TEX R0, R0, texture[2], 2D;\n"
    "MUL R2.xyz, R1, c[10].y;\n"
    "MUL R3.xyz, R2, R0.w;\n"
    "MUL R2.xyz, R0, c[10].x;\n"
    "MAD R2.xyz, R2, R1.w, R3;\n"
    "ADD R3.xy, fragment.position, c[0];\n"
    "MUL R0.xyz, R0, c[9].y;\n"
    "ADD R2.w, -R1, c[8].x;\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "ADD R2.w, -R0, c[8].x;\n"
    "MUL R0.xyz, R1, c[9].z;\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "ADD R0.y, -R1.w, c[8].x;\n"
    "MUL R0.z, R1.w, R2.w;\n"
    "MUL R0.x, R0.w, R1.w;\n"
    "MUL R0.y, R0.w, R0;\n"
    "DP3 R2.w, R0, c[9];\n"
    "MUL R3.xy, R3, c[1];\n"
    "TEX R0, R3, texture[1], 2D;\n"
    "ADD R2, R2, -R1;\n"
    "DP4 R0.x, R0, c[2];\n"
    "MAD result.color, R0.x, R2, R1;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_TEXTURE_COMPOSITION_MODE_BLEND_MODE_MASK =
    "!!ARBfp1.0\n"
    "PARAM c[7] = { program.local[0..6] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "MUL R0.xyz, fragment.position.y, c[1];\n"
    "MAD R0.xyz, fragment.position.x, c[0], R0;\n"
    "ADD R1.xyz, R0, c[2];\n"
    "RCP R0.z, R1.z;\n"
    "MUL R1.xy, R1, R0.z;\n"
    "ADD R0.xy, fragment.position, c[4];\n"
    "MUL R0.xy, R0, c[5];\n"
    "TEX R0, R0, texture[0], 2D;\n"
    "DP4 R1.z, R0, c[6];\n"
    "MUL R1.xy, R1, c[3];\n"
    "MOV R0.x, R1;\n"
    "MOV R0.y, -R1;\n"
    "TEX R0, R0, texture[1], 2D;\n"
    "MUL result.color, R0, R1.z;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_TEXTURE_COMPOSITION_MODE_BLEND_MODE_NOMASK =
    "!!ARBfp1.0\n"
    "PARAM c[4] = { program.local[0..3] };\n"
    "TEMP R0;\n"
    "MUL R0.xyz, fragment.position.y, c[1];\n"
    "MAD R0.xyz, fragment.position.x, c[0], R0;\n"
    "ADD R0.xyz, R0, c[2];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "MUL R0.xy, R0, c[3];\n"
    "MOV R0.y, -R0;\n"
    "TEX result.color, R0, texture[0], 2D;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_PATTERN_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[11] = { program.local[0..7],\n"
    "		{ 1 },\n"
    "		program.local[9..10] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "MUL R0.xyz, fragment.position.y, c[4];\n"
    "MAD R0.xyz, fragment.position.x, c[3], R0;\n"
    "ADD R0.xyz, R0, c[5];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "MUL R0.xy, R0, c[6];\n"
    "MOV R0.y, -R0;\n"
    "MUL R0.zw, fragment.position.xyxy, c[7].xyxy;\n"
    "TEX R1, R0.zwzw, texture[0], 2D;\n"
    "TEX R0.x, R0, texture[2], 2D;\n"
    "MUL R0, fragment.color.primary, R0.x;\n"
    "MUL R2.xyz, R1, c[10].y;\n"
    "MUL R3.xyz, R2, R0.w;\n"
    "MUL R2.xyz, R0, c[10].x;\n"
    "MAD R2.xyz, R2, R1.w, R3;\n"
    "ADD R3.xy, fragment.position, c[0];\n"
    "MUL R0.xyz, R0, c[9].y;\n"
    "ADD R2.w, -R1, c[8].x;\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "ADD R2.w, -R0, c[8].x;\n"
    "MUL R0.xyz, R1, c[9].z;\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "ADD R0.y, -R1.w, c[8].x;\n"
    "MUL R0.z, R1.w, R2.w;\n"
    "MUL R0.x, R0.w, R1.w;\n"
    "MUL R0.y, R0.w, R0;\n"
    "DP3 R2.w, R0, c[9];\n"
    "MUL R3.xy, R3, c[1];\n"
    "TEX R0, R3, texture[1], 2D;\n"
    "ADD R2, R2, -R1;\n"
    "DP4 R0.x, R0, c[2];\n"
    "MAD result.color, R0.x, R2, R1;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_PATTERN_COMPOSITION_MODE_BLEND_MODE_MASK =
    "!!ARBfp1.0\n"
    "PARAM c[7] = { program.local[0..6] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "MUL R0.xyz, fragment.position.y, c[1];\n"
    "MAD R0.xyz, fragment.position.x, c[0], R0;\n"
    "ADD R1.xyz, R0, c[2];\n"
    "RCP R0.z, R1.z;\n"
    "MUL R0.zw, R1.xyxy, R0.z;\n"
    "MUL R1.xy, R0.zwzw, c[3];\n"
    "MOV R1.y, -R1;\n"
    "ADD R0.xy, fragment.position, c[4];\n"
    "MUL R0.xy, R0, c[5];\n"
    "TEX R0, R0, texture[0], 2D;\n"
    "TEX R1.x, R1, texture[1], 2D;\n"
    "DP4 R0.x, R0, c[6];\n"
    "MUL R1, fragment.color.primary, R1.x;\n"
    "MUL result.color, R1, R0.x;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_PATTERN_COMPOSITION_MODE_BLEND_MODE_NOMASK =
    "!!ARBfp1.0\n"
    "PARAM c[4] = { program.local[0..3] };\n"
    "TEMP R0;\n"
    "MUL R0.xyz, fragment.position.y, c[1];\n"
    "MAD R0.xyz, fragment.position.x, c[0], R0;\n"
    "ADD R0.xyz, R0, c[2];\n"
    "RCP R0.z, R0.z;\n"
    "MUL R0.xy, R0, R0.z;\n"
    "MUL R0.xy, R0, c[3];\n"
    "MOV R0.y, -R0;\n"
    "TEX R0.x, R0, texture[0], 2D;\n"
    "MUL result.color, fragment.color.primary, R0.x;\n"
    "END\n"
    ;

static const char *mask_fragment_program_sources[num_fragment_masks] = {
    FragmentProgram_FRAGMENT_PROGRAM_MASK_TRAPEZOID_AA,
    FragmentProgram_FRAGMENT_PROGRAM_MASK_ELLIPSE_AA,
};

static const char *painter_fragment_program_sources[num_fragment_brushes][num_fragment_composition_modes] = {
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_SOLID_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_SOLID_COMPOSITION_MODE_BLEND_MODE_MASK,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_SOLID_COMPOSITION_MODE_BLEND_MODE_NOMASK,
    },
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_RADIAL_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_RADIAL_COMPOSITION_MODE_BLEND_MODE_MASK,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_RADIAL_COMPOSITION_MODE_BLEND_MODE_NOMASK,
    },
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_CONICAL_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_CONICAL_COMPOSITION_MODE_BLEND_MODE_MASK,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_CONICAL_COMPOSITION_MODE_BLEND_MODE_NOMASK,
    },
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_LINEAR_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_LINEAR_COMPOSITION_MODE_BLEND_MODE_MASK,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_LINEAR_COMPOSITION_MODE_BLEND_MODE_NOMASK,
    },
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_TEXTURE_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_TEXTURE_COMPOSITION_MODE_BLEND_MODE_MASK,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_TEXTURE_COMPOSITION_MODE_BLEND_MODE_NOMASK,
    },
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_PATTERN_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_PATTERN_COMPOSITION_MODE_BLEND_MODE_MASK,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_PATTERN_COMPOSITION_MODE_BLEND_MODE_NOMASK,
    },
};

static int painter_variable_locations[num_fragment_brushes][num_fragment_composition_modes][num_fragment_variables] = {
    {
        { -1, -1, -1, 1, -1, 6, 2, -1, 5, 3, 1, 0, -1, 0, -1, -1, -1, -1, -1, },
        { -1, -1, -1, 1, -1, -1, 2, -1, -1, -1, 0, -1, -1, 0, -1, -1, -1, -1, -1, },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    },
    {
        { -1, -1, 4, 1, 5, 11, 2, -1, 10, 9, 1, 0, 2, 0, -1, 8, 6, 3, -1, },
        { -1, -1, 1, 7, 2, -1, 8, -1, -1, -1, 0, -1, 1, 6, -1, 5, 3, 0, -1, },
        { -1, -1, 1, -1, 2, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, 5, 3, 0, -1, },
    },
    {
        { -1, -1, 4, 1, 5, 12, 2, -1, 11, 10, 1, 0, 2, 0, -1, -1, -1, 3, 8, },
        { -1, -1, 1, 8, 2, -1, 9, -1, -1, -1, 0, -1, 1, 7, -1, -1, -1, 0, 5, },
        { -1, -1, 1, -1, 2, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, 0, 5, },
    },
    {
        { -1, 6, 4, 1, 5, 10, 2, -1, 9, 7, 1, 0, 2, 0, -1, -1, -1, 3, -1, },
        { -1, 3, 1, 5, 2, -1, 6, -1, -1, -1, 0, -1, 1, 4, -1, -1, -1, 0, -1, },
        { -1, 3, 1, -1, 2, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, 0, -1, },
    },
    {
        { 2, -1, 4, 1, 5, 10, 2, -1, 9, 7, 1, 0, -1, 0, 6, -1, -1, 3, -1, },
        { 1, -1, 1, 5, 2, -1, 6, -1, -1, -1, 0, -1, -1, 4, 3, -1, -1, 0, -1, },
        { 0, -1, 1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3, -1, -1, 0, -1, },
    },
    {
        { 2, -1, 4, 1, 5, 10, 2, -1, 9, 7, 1, 0, -1, 0, 6, -1, -1, 3, -1, },
        { 1, -1, 1, 5, 2, -1, 6, -1, -1, -1, 0, -1, -1, 4, 3, -1, -1, 0, -1, },
        { 0, -1, 1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3, -1, -1, 0, -1, },
    },
};

static int mask_variable_locations[num_fragment_masks][num_fragment_variables] = {
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    { -1, -1, 2, -1, 3, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, },
};

#endif
