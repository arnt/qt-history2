// Generated by src/opengl/util/./glsl_to_include.sh from ellipse_aa.glsl
"!!ARBfp1.0"
"OPTION NV_fragment_program2;"
"PARAM c[3] = { { 1, -0.55000001, 0.90909088, 2 },"
"{ 3 },"
"program.local[1] };"
"TEMP R0;"
"TEMP RC;"
"TEMP HC;"
"OUTPUT oCol = result.color;"
"MULR R0.xy, fragment.texcoord[0], fragment.texcoord[0];"
"ADDR R0.x, R0, R0.y;"
"ADDR R0.z, -R0.x, c[0].x;"
"DDYR R0.y, R0.z;"
"DDXR R0.x, R0.z;"
"MULR R0.xy, R0, R0;"
"ADDR R0.x, R0, R0.y;"
"RSQR R0.x, R0.x;"
"MADR R0.x, R0, R0.z, -c[0].y;"
"MULR_SAT R0.x, R0, c[0].z;"
"MULR R0.y, -R0.x, c[0].w;"
"ADDR R0.y, R0, c[1].x;"
"MULR R0.x, R0, R0;"
"MULR R0.x, R0, R0.y;"
"MULR oCol, R0.x, c[2];"
"END"
; // Generated by src/opengl/util/./glsl_to_include.sh from ellipse_aa.glsl
