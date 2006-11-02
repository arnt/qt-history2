#ifndef FRAGMENTPROGRAMS_H
#define FRAGMENTPROGRAMS_H

enum FragmentVariable {
    VAR_INV_MATRIX,
    VAR_PALETTE,
    VAR_INV_BRUSH_TEXTURE_SIZE,
    VAR_BRUSH_TEXTURE,
    VAR_PORTERDUFF_AB,
    VAR_INV_MATRIX_OFFSET,
    VAR_FMP,
    VAR_FMP2_M_RADIUS2,
    VAR_LINEAR,
    VAR_INV_BUFFER_SIZE,
    VAR_MASK_TEXTURE,
    VAR_DST_TEXTURE,
    VAR_ANGLE,
    VAR_PORTERDUFF_XYZ,
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
    COMPOSITION_MODE_BLEND_MODE,
};

enum FragmentMaskType {
    FRAGMENT_PROGRAM_MASK_TRAPEZOID_AA,
    FRAGMENT_PROGRAM_MASK_ELLIPSE_AA,
};

static const unsigned int num_fragment_variables = 14;

static const unsigned int num_fragment_brushes = 6;
static const unsigned int num_fragment_composition_modes = 2;
static const unsigned int num_fragment_masks = 2;

static const char *FragmentProgram_FRAGMENT_PROGRAM_MASK_TRAPEZOID_AA =
    "!!ARBfp1.0\n"
    "PARAM c[1] = { { 0.5, 2, 9.9999999e-09, 0 } };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "TEMP R4;\n"
    "ADD R0.y, fragment.position, -c[0].x;\n"
    "MAX R4.y, R0, fragment.texcoord[0];\n"
    "ADD R0.x, fragment.position.y, c[0];\n"
    "MIN R4.x, R0, fragment.texcoord[0];\n"
    "MOV R0.yw, R4.y;\n"
    "MOV R0.xz, R4.x;\n"
    "MAD R1, fragment.texcoord[1].xxzz, R0, fragment.texcoord[1].yyww;\n"
    "MAD R0.zw, fragment.position.x, c[0].y, -R1;\n"
    "MOV R0.y, R0.z;\n"
    "MOV R0.x, R1;\n"
    "MOV R0.z, R1.y;\n"
    "MIN R2.xy, R0, R0.zwzw;\n"
    "MAX R2.zw, R0.xyxy, R0;\n"
    "ADD R3, R2.zzww, -R2.xxyy;\n"
    "RCP R0.y, R3.y;\n"
    "RCP R0.z, R3.z;\n"
    "ADD R3.z, fragment.position.x, -c[0].x;\n"
    "ADD R3.y, fragment.position.x, c[0].x;\n"
    "RCP R0.w, R3.w;\n"
    "MOV R3.w, R3.y;\n"
    "ADD R1, R3.zwzw, -R2.xxyy;\n"
    "ADD R3.w, R4.x, -R4.y;\n"
    "MUL R1, R3.w, R1;\n"
    "RCP R0.x, R3.x;\n"
    "MAD R0, R1, R0, R4.y;\n"
    "ADD R1.y, R3.x, -c[0].z;\n"
    "ADD R1.x, R2.w, -R2.y;\n"
    "ADD R1.x, R1, -c[0].z;\n"
    "CMP R0.xy, R1.y, c[0].w, R0;\n"
    "CMP R0.zw, R1.x, c[0].w, R0;\n"
    "ADD R1.xy, R0.ywzw, R0.xzzw;\n"
    "ADD R4.zw, R4.x, -R0.xyxz;\n"
    "ADD R0.zw, R0.xyyw, -R4.y;\n"
    "ADD R0.xy, R3.y, -R2;\n"
    "MUL R0.zw, R0.xyxy, R0;\n"
    "ADD R0.xy, R2.zwzw, R2;\n"
    "MAD R0.zw, -R0, c[0].x, R3.w;\n"
    "ADD R1.z, R3.y, -R3;\n"
    "MAD R1.xy, -R1, c[0].x, R4.x;\n"
    "MUL R1.xy, R1, R1.z;\n"
    "ADD R1.zw, R2, -R3.z;\n"
    "MUL R1.zw, R4, R1;\n"
    "MAD R0.xy, R0, c[0].x, -R3.z;\n"
    "MAD R4.zw, R0.xyxy, R3.w, -R0;\n"
    "MAD R1.zw, R1, c[0].x, -R1.xyxy;\n"
    "SGE R0.xy, R3.y, R2.zwzw;\n"
    "MUL R1.zw, R0.xyxy, R1;\n"
    "MAD R0.xy, R0, R4.zwzw, R0.zwzw;\n"
    "ADD R0.zw, R1.xyxy, R1;\n"
    "ADD R1.xy, R0.zwzw, -R0;\n"
    "SGE R0.zw, R3.z, R2.xyxy;\n"
    "MAD R0.xy, R0.zwzw, R1, R0;\n"
    "SGE R0.zw, R2, R3.z;\n"
    "ADD R0.xy, R0, -R3.w;\n"
    "SGE R1.xy, R3.y, R2;\n"
    "MAD R0.xy, R1, R0, R3.w;\n"
    "MUL R0.xy, R0, R0.zwzw;\n"
    "ADD R0.x, R3.w, -R0;\n"
    "SGE R0.z, R4.x, R4.y;\n"
    "ADD R0.x, R0, -R0.y;\n"
    "MUL result.color, R0.x, R0.z;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_MASK_ELLIPSE_AA =
    "!!ARBfp1.0\n"
    "PARAM c[2] = { { 2, 1, -0.55000001, 0.90909088 },\n"
    "		{ 3 } };\n"
    "TEMP R0;\n"
    "MUL R0.xy, fragment.texcoord[0].zwzw, fragment.texcoord[0].zwzw;\n"
    "RCP R0.y, R0.y;\n"
    "RCP R0.x, R0.x;\n"
    "MUL R0.zw, fragment.texcoord[0].xyxy, R0.xyxy;\n"
    "MUL R0.zw, R0, c[0].x;\n"
    "RCP R0.y, fragment.texcoord[0].w;\n"
    "RCP R0.x, fragment.texcoord[0].z;\n"
    "MUL R0.xy, fragment.texcoord[0], R0;\n"
    "MUL R0.xy, R0, R0;\n"
    "ADD R0.x, R0, R0.y;\n"
    "MUL R0.zw, R0, R0;\n"
    "ADD R0.y, R0.z, R0.w;\n"
    "RSQ R0.y, R0.y;\n"
    "ADD R0.x, -R0, c[0].y;\n"
    "MAD R0.x, R0.y, R0, -c[0].z;\n"
    "MUL_SAT R0.x, R0, c[0].w;\n"
    "MUL R0.y, -R0.x, c[0].x;\n"
    "ADD R0.y, R0, c[1].x;\n"
    "MUL R0.x, R0, R0;\n"
    "MUL result.color, R0.x, R0.y;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_SOLID_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[4] = { program.local[0],\n"
    "		{ 1 },\n"
    "		program.local[2..3] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "MUL R3.xy, fragment.position, c[0];\n"
    "TEX R0, R3, texture[0], 2D;\n"
    "MUL R1.xyz, R0, c[3].y;\n"
    "MUL R2.xyz, R1, fragment.color.primary.w;\n"
    "MUL R1.xyz, fragment.color.primary, c[3].x;\n"
    "MAD R2.xyz, R1, R0.w, R2;\n"
    "ADD R1.w, -R0, c[1].x;\n"
    "MUL R1.xyz, fragment.color.primary, c[2].y;\n"
    "MAD R2.xyz, R1, R1.w, R2;\n"
    "ADD R1.w, -fragment.color.primary, c[1].x;\n"
    "MUL R1.xyz, R0, c[2].z;\n"
    "MAD R1.xyz, R1, R1.w, R2;\n"
    "ADD R2.y, -R0.w, c[1].x;\n"
    "MUL R2.x, fragment.color.primary.w, R0.w;\n"
    "MUL R2.z, R0.w, R1.w;\n"
    "MUL R2.y, fragment.color.primary.w, R2;\n"
    "DP3 R1.w, R2, c[2];\n"
    "ADD R1, R1, -R0;\n"
    "TEX R2.x, R3, texture[1], 2D;\n"
    "MAD result.color, R2.x, R1, R0;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_SOLID_COMPOSITION_MODE_BLEND_MODE =
    "!!ARBfp1.0\n"
    "PARAM c[1] = { program.local[0] };\n"
    "TEMP R0;\n"
    "MUL R0.xy, fragment.position, c[0];\n"
    "TEX R0.x, R0, texture[0], 2D;\n"
    "MUL result.color, fragment.color.primary, R0.x;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_RADIAL_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[8] = { program.local[0..3],\n"
    "		{ 2, 4, 1 },\n"
    "		program.local[5..7] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "TEMP R4;\n"
    "MUL R0.zw, c[1].xyxy, fragment.position.xyxy;\n"
    "MUL R4.xy, fragment.position, c[0];\n"
    "TEX R1, R4, texture[0], 2D;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, c[1].zwzw, fragment.position;\n"
    "ADD R0.w, R0.x, R0.y;\n"
    "ADD R0.xy, R0.zwzw, c[2];\n"
    "MUL R0.zw, R0.xyxy, R0.xyxy;\n"
    "MUL R0.xy, R0, c[3];\n"
    "ADD R0.z, R0, R0.w;\n"
    "ADD R0.x, R0, R0.y;\n"
    "MUL R0.z, c[5].x, -R0;\n"
    "MUL R0.y, R0.z, c[4];\n"
    "MUL R0.x, R0, c[4];\n"
    "MAD R0.y, R0.x, R0.x, -R0;\n"
    "RSQ R0.z, R0.y;\n"
    "RCP R0.z, R0.z;\n"
    "MOV R0.y, c[4].x;\n"
    "MUL R0.y, c[5].x, R0;\n"
    "ADD R0.x, -R0, R0.z;\n"
    "RCP R0.y, R0.y;\n"
    "MUL R0.x, R0, R0.y;\n"
    "TEX R0, R0, texture[2], 1D;\n"
    "MUL R2.xyz, R1, c[7].y;\n"
    "MUL R3.xyz, R2, R0.w;\n"
    "MUL R2.xyz, R0, c[7].x;\n"
    "MAD R2.xyz, R2, R1.w, R3;\n"
    "ADD R3.x, -R0.w, c[4].z;\n"
    "MUL R0.xyz, R0, c[6].y;\n"
    "ADD R2.w, -R1, c[4].z;\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "MUL R0.xyz, R1, c[6].z;\n"
    "MAD R0.xyz, R0, R3.x, R2;\n"
    "MUL R2.x, R0.w, R1.w;\n"
    "MUL R2.z, R1.w, R3.x;\n"
    "MUL R2.y, R0.w, R2.w;\n"
    "DP3 R0.w, R2, c[6];\n"
    "ADD R0, R0, -R1;\n"
    "TEX R2.x, R4, texture[1], 2D;\n"
    "MAD result.color, R2.x, R0, R1;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_RADIAL_COMPOSITION_MODE_BLEND_MODE =
    "!!ARBfp1.0\n"
    "PARAM c[6] = { program.local[0..2],\n"
    "		{ 2, 4 },\n"
    "		program.local[4..5] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "MUL R0.zw, c[0].xyxy, fragment.position.xyxy;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, c[0].zwzw, fragment.position;\n"
    "ADD R0.w, R0.x, R0.y;\n"
    "ADD R0.xy, R0.zwzw, c[1];\n"
    "MUL R0.zw, R0.xyxy, R0.xyxy;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, R0, c[2];\n"
    "ADD R0.x, R0, R0.y;\n"
    "MUL R0.z, c[4].x, -R0;\n"
    "MUL R0.y, R0.z, c[3];\n"
    "MUL R0.x, R0, c[3];\n"
    "MAD R0.y, R0.x, R0.x, -R0;\n"
    "RSQ R0.z, R0.y;\n"
    "RCP R0.z, R0.z;\n"
    "ADD R0.x, -R0, R0.z;\n"
    "MUL R0.zw, fragment.position.xyxy, c[5].xyxy;\n"
    "MOV R0.y, c[3].x;\n"
    "MUL R0.y, c[4].x, R0;\n"
    "RCP R0.y, R0.y;\n"
    "TEX R1.x, R0.zwzw, texture[0], 2D;\n"
    "MUL R0.x, R0, R0.y;\n"
    "TEX R0, R0, texture[1], 1D;\n"
    "MUL result.color, R0, R1.x;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_CONICAL_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[9] = { program.local[0..2],\n"
    "		{ 0.0020000001, 9.9999997e-10, 0.1963, 0.9817 },\n"
    "		{ 2.3561945, 0.78539819, -1, 1 },\n"
    "		program.local[5],\n"
    "		{ 0.15915494 },\n"
    "		program.local[7..8] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "TEMP R4;\n"
    "MUL R0.zw, c[1].xyxy, fragment.position.xyxy;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, c[1].zwzw, fragment.position;\n"
    "ADD R0.w, R0.x, R0.y;\n"
    "ADD R0.xy, R0.zwzw, c[2];\n"
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
    "MUL R4.xy, fragment.position, c[0];\n"
    "TEX R1, R4, texture[0], 2D;\n"
    "CMP R0.y, -R0, c[4].z, c[4].w;\n"
    "MAD R0.x, R0.w, R0.z, R0;\n"
    "MAD R0.x, R0, R0.y, c[5];\n"
    "MUL R0.x, R0, c[6];\n"
    "FLR R0.y, R0.x;\n"
    "ADD R0.x, R0, -R0.y;\n"
    "TEX R0, R0, texture[2], 1D;\n"
    "MUL R2.xyz, R1, c[8].y;\n"
    "MUL R3.xyz, R2, R0.w;\n"
    "MUL R2.xyz, R0, c[8].x;\n"
    "MAD R2.xyz, R2, R1.w, R3;\n"
    "ADD R3.x, -R0.w, c[4].w;\n"
    "MUL R0.xyz, R0, c[7].y;\n"
    "ADD R2.w, -R1, c[4];\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "MUL R0.xyz, R1, c[7].z;\n"
    "MAD R0.xyz, R0, R3.x, R2;\n"
    "MUL R2.x, R0.w, R1.w;\n"
    "MUL R2.z, R1.w, R3.x;\n"
    "MUL R2.y, R0.w, R2.w;\n"
    "DP3 R0.w, R2, c[7];\n"
    "ADD R0, R0, -R1;\n"
    "TEX R2.x, R4, texture[1], 2D;\n"
    "MAD result.color, R2.x, R0, R1;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_CONICAL_COMPOSITION_MODE_BLEND_MODE =
    "!!ARBfp1.0\n"
    "PARAM c[7] = { program.local[0..1],\n"
    "		{ 0.0020000001, 9.9999997e-10, 0.1963, 0.9817 },\n"
    "		{ 2.3561945, 0.78539819, -1, 1 },\n"
    "		program.local[4],\n"
    "		{ 0.15915494 },\n"
    "		program.local[6] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "MUL R0.zw, c[0].xyxy, fragment.position.xyxy;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, c[0].zwzw, fragment.position;\n"
    "ADD R0.w, R0.x, R0.y;\n"
    "ADD R0.xy, R0.zwzw, c[1];\n"
    "ABS R0.w, R0.x;\n"
    "ABS R0.z, R0.y;\n"
    "ADD R0.z, R0, -R0.w;\n"
    "ADD R0.w, R0.y, c[2].x;\n"
    "ABS R0.z, R0;\n"
    "CMP R0.y, -R0.z, R0, R0.w;\n"
    "ABS R0.z, -R0.y;\n"
    "ADD R0.z, R0, c[2].y;\n"
    "ADD R0.w, R0.x, R0.z;\n"
    "ADD R1.x, R0.z, -R0;\n"
    "RCP R1.x, R1.x;\n"
    "RCP R1.y, R0.w;\n"
    "MUL R0.w, R0, R1.x;\n"
    "ADD R0.z, R0.x, -R0;\n"
    "MUL R0.z, R0, R1.y;\n"
    "CMP R0.z, R0.x, R0.w, R0;\n"
    "MUL R0.w, R0.z, R0.z;\n"
    "MOV R1.x, c[3].y;\n"
    "CMP R0.x, R0, c[3], R1;\n"
    "MAD R0.w, R0, c[2].z, -c[2];\n"
    "MAD R0.x, R0.w, R0.z, R0;\n"
    "CMP R0.y, -R0, c[3].z, c[3].w;\n"
    "MAD R0.x, R0, R0.y, c[4];\n"
    "MUL R0.x, R0, c[5];\n"
    "FLR R0.y, R0.x;\n"
    "MUL R0.zw, fragment.position.xyxy, c[6].xyxy;\n"
    "TEX R1.x, R0.zwzw, texture[0], 2D;\n"
    "ADD R0.x, R0, -R0.y;\n"
    "TEX R0, R0, texture[1], 1D;\n"
    "MUL result.color, R0, R1.x;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_LINEAR_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[7] = { program.local[0..3],\n"
    "		{ 1 },\n"
    "		program.local[5..6] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "TEMP R4;\n"
    "MUL R0.zw, fragment.position.xyxy, c[1].xyxy;\n"
    "MUL R4.xy, fragment.position, c[0];\n"
    "TEX R1, R4, texture[0], 2D;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, fragment.position, c[1].zwzw;\n"
    "ADD R0.w, R0.x, R0.y;\n"
    "ADD R0.xy, R0.zwzw, c[2];\n"
    "MUL R0.xy, R0, c[3];\n"
    "ADD R0.x, R0, R0.y;\n"
    "MUL R0.x, R0, c[3].z;\n"
    "TEX R0, R0, texture[2], 1D;\n"
    "MUL R2.xyz, R1, c[6].y;\n"
    "MUL R3.xyz, R2, R0.w;\n"
    "MUL R2.xyz, R0, c[6].x;\n"
    "MUL R0.xyz, R0, c[5].y;\n"
    "ADD R2.w, -R1, c[4].x;\n"
    "MAD R2.xyz, R2, R1.w, R3;\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "ADD R2.w, -R0, c[4].x;\n"
    "MUL R0.xyz, R1, c[5].z;\n"
    "MAD R0.xyz, R0, R2.w, R2;\n"
    "ADD R2.y, -R1.w, c[4].x;\n"
    "MUL R2.x, R0.w, R1.w;\n"
    "MUL R2.z, R1.w, R2.w;\n"
    "MUL R2.y, R0.w, R2;\n"
    "DP3 R0.w, R2, c[5];\n"
    "ADD R0, R0, -R1;\n"
    "TEX R2.x, R4, texture[1], 2D;\n"
    "MAD result.color, R2.x, R0, R1;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_LINEAR_COMPOSITION_MODE_BLEND_MODE =
    "!!ARBfp1.0\n"
    "PARAM c[4] = { program.local[0..3] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "MUL R0.zw, fragment.position.xyxy, c[0].xyxy;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, fragment.position, c[0].zwzw;\n"
    "ADD R0.w, R0.x, R0.y;\n"
    "ADD R0.xy, R0.zwzw, c[1];\n"
    "MUL R0.xy, R0, c[2];\n"
    "ADD R0.x, R0, R0.y;\n"
    "MUL R0.zw, fragment.position.xyxy, c[3].xyxy;\n"
    "TEX R1.x, R0.zwzw, texture[0], 2D;\n"
    "MUL R0.x, R0, c[2].z;\n"
    "TEX R0, R0, texture[1], 1D;\n"
    "MUL result.color, R0, R1.x;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_TEXTURE_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[7] = { program.local[0..3],\n"
    "		{ 1 },\n"
    "		program.local[5..6] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "TEMP R4;\n"
    "MUL R0.zw, fragment.position.xyxy, c[1].xyxy;\n"
    "MUL R4.xy, fragment.position, c[0];\n"
    "TEX R1, R4, texture[0], 2D;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, fragment.position, c[1].zwzw;\n"
    "ADD R0.w, R0.x, R0.y;\n"
    "ADD R0.xy, R0.zwzw, c[2];\n"
    "MUL R0.xy, R0, c[3];\n"
    "MOV R0.y, -R0;\n"
    "TEX R0, R0, texture[2], 2D;\n"
    "MUL R2.xyz, R1, c[6].y;\n"
    "MUL R3.xyz, R2, R0.w;\n"
    "MUL R2.xyz, R0, c[6].x;\n"
    "MUL R0.xyz, R0, c[5].y;\n"
    "ADD R2.w, -R1, c[4].x;\n"
    "MAD R2.xyz, R2, R1.w, R3;\n"
    "MAD R2.xyz, R0, R2.w, R2;\n"
    "ADD R2.w, -R0, c[4].x;\n"
    "MUL R0.xyz, R1, c[5].z;\n"
    "MAD R0.xyz, R0, R2.w, R2;\n"
    "ADD R2.y, -R1.w, c[4].x;\n"
    "MUL R2.x, R0.w, R1.w;\n"
    "MUL R2.z, R1.w, R2.w;\n"
    "MUL R2.y, R0.w, R2;\n"
    "DP3 R0.w, R2, c[5];\n"
    "ADD R0, R0, -R1;\n"
    "TEX R2.x, R4, texture[1], 2D;\n"
    "MAD result.color, R2.x, R0, R1;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_TEXTURE_COMPOSITION_MODE_BLEND_MODE =
    "!!ARBfp1.0\n"
    "PARAM c[4] = { program.local[0..3] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "MUL R0.zw, fragment.position.xyxy, c[0].xyxy;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, fragment.position, c[0].zwzw;\n"
    "ADD R0.w, R0.x, R0.y;\n"
    "ADD R0.xy, R0.zwzw, c[1];\n"
    "MUL R0.xy, R0, c[2];\n"
    "MUL R0.zw, fragment.position.xyxy, c[3].xyxy;\n"
    "TEX R1.x, R0.zwzw, texture[0], 2D;\n"
    "MOV R0.y, -R0;\n"
    "TEX R0, R0, texture[1], 2D;\n"
    "MUL result.color, R0, R1.x;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_PATTERN_COMPOSITION_MODES_SIMPLE_PORTER_DUFF =
    "!!ARBfp1.0\n"
    "PARAM c[7] = { program.local[0..3],\n"
    "		{ 1 },\n"
    "		program.local[5..6] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "TEMP R2;\n"
    "TEMP R3;\n"
    "TEMP R4;\n"
    "MUL R0.zw, fragment.position.xyxy, c[1].xyxy;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, fragment.position, c[1].zwzw;\n"
    "ADD R0.w, R0.x, R0.y;\n"
    "ADD R0.xy, R0.zwzw, c[2];\n"
    "MUL R0.xy, R0, c[3];\n"
    "MOV R0.y, -R0;\n"
    "TEX R1.x, R0, texture[2], 2D;\n"
    "MUL R4.xy, fragment.position, c[0];\n"
    "TEX R0, R4, texture[0], 2D;\n"
    "MUL R1, fragment.color.primary, R1.x;\n"
    "MUL R2.xyz, R0, c[6].y;\n"
    "MUL R3.xyz, R2, R1.w;\n"
    "MUL R2.xyz, R1, c[6].x;\n"
    "MUL R1.xyz, R1, c[5].y;\n"
    "ADD R2.w, -R0, c[4].x;\n"
    "MAD R2.xyz, R2, R0.w, R3;\n"
    "MAD R2.xyz, R1, R2.w, R2;\n"
    "ADD R2.w, -R1, c[4].x;\n"
    "MUL R1.xyz, R0, c[5].z;\n"
    "MAD R1.xyz, R1, R2.w, R2;\n"
    "ADD R2.y, -R0.w, c[4].x;\n"
    "MUL R2.x, R1.w, R0.w;\n"
    "MUL R2.y, R1.w, R2;\n"
    "MUL R2.z, R0.w, R2.w;\n"
    "DP3 R1.w, R2, c[5];\n"
    "ADD R2, R1, -R0;\n"
    "TEX R1.x, R4, texture[1], 2D;\n"
    "MAD result.color, R1.x, R2, R0;\n"
    "END\n"
    ;

static const char *FragmentProgram_FRAGMENT_PROGRAM_BRUSH_PATTERN_COMPOSITION_MODE_BLEND_MODE =
    "!!ARBfp1.0\n"
    "PARAM c[4] = { program.local[0..3] };\n"
    "TEMP R0;\n"
    "TEMP R1;\n"
    "MUL R0.zw, fragment.position.xyxy, c[0].xyxy;\n"
    "ADD R0.z, R0, R0.w;\n"
    "MUL R0.xy, fragment.position, c[0].zwzw;\n"
    "ADD R0.w, R0.x, R0.y;\n"
    "ADD R0.xy, R0.zwzw, c[1];\n"
    "MUL R0.xy, R0, c[2];\n"
    "MOV R0.w, -R0.y;\n"
    "MOV R0.z, R0.x;\n"
    "MUL R0.xy, fragment.position, c[3];\n"
    "TEX R1.x, R0.zwzw, texture[1], 2D;\n"
    "TEX R0.x, R0, texture[0], 2D;\n"
    "MUL R1, fragment.color.primary, R1.x;\n"
    "MUL result.color, R1, R0.x;\n"
    "END\n"
    ;

static const char *mask_fragment_program_sources[num_fragment_masks] = {
    FragmentProgram_FRAGMENT_PROGRAM_MASK_TRAPEZOID_AA,
    FragmentProgram_FRAGMENT_PROGRAM_MASK_ELLIPSE_AA,
};

static const char *painter_fragment_program_sources[num_fragment_brushes][num_fragment_composition_modes] = {
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_SOLID_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_SOLID_COMPOSITION_MODE_BLEND_MODE,
    },
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_RADIAL_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_RADIAL_COMPOSITION_MODE_BLEND_MODE,
    },
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_CONICAL_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_CONICAL_COMPOSITION_MODE_BLEND_MODE,
    },
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_LINEAR_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_LINEAR_COMPOSITION_MODE_BLEND_MODE,
    },
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_TEXTURE_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_TEXTURE_COMPOSITION_MODE_BLEND_MODE,
    },
    {
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_PATTERN_COMPOSITION_MODES_SIMPLE_PORTER_DUFF,
        FragmentProgram_FRAGMENT_PROGRAM_BRUSH_PATTERN_COMPOSITION_MODE_BLEND_MODE,
    },
};

static int painter_variable_locations[num_fragment_brushes][num_fragment_composition_modes][num_fragment_variables] = {
    {
        { -1, -1, -1, -1, 3, -1, -1, -1, -1, 0, 1, 0, -1, 2, },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, -1, -1, -1, },
    },
    {
        { 1, 2, -1, -1, 7, 2, 3, 5, -1, 0, 1, 0, -1, 6, },
        { 0, 1, -1, -1, -1, 1, 2, 4, -1, 5, 0, -1, -1, -1, },
    },
    {
        { 1, 2, -1, -1, 8, 2, -1, -1, -1, 0, 1, 0, 5, 7, },
        { 0, 1, -1, -1, -1, 1, -1, -1, -1, 6, 0, -1, 4, -1, },
    },
    {
        { 1, 2, -1, -1, 6, 2, -1, -1, 3, 0, 1, 0, -1, 5, },
        { 0, 1, -1, -1, -1, 1, -1, -1, 2, 3, 0, -1, -1, -1, },
    },
    {
        { 1, -1, 3, 2, 6, 2, -1, -1, -1, 0, 1, 0, -1, 5, },
        { 0, -1, 2, 1, -1, 1, -1, -1, -1, 3, 0, -1, -1, -1, },
    },
    {
        { 1, -1, 3, 2, 6, 2, -1, -1, -1, 0, 1, 0, -1, 5, },
        { 0, -1, 2, 1, -1, 1, -1, -1, -1, 3, 0, -1, -1, -1, },
    },
};

#endif
