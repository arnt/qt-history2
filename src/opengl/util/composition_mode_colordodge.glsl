// Dca' = Sca.Da + Dca.Sa <= Sa.Da ?
//        Dca.Sa/(1 - Sca/Sa) + Sca.(1 - Da) + Dca.(1 - Sa) :
//        Sa.Da + Sca.(1 - Da) + Dca.(1 - Sa)
// Da'  = Sa + Da - Sa.Da 
vec4 composite(vec4 src, vec4 dst)
{
    vec4 result;
    result.rgb = mix(dst.rgb * src.a / (1 - src.rgb / src.a) + src.rgb * (1 - dst.a) + dst.rgb * (1 - src.a),
                     src.a * dst.a + src.rgb * (1 - dst.a) + dst.rgb * (1 - src.a),
                     step(src.a * dst.a, src.rgb * dst.a + dst.rgb * src.a));
    result.a = src.a + dst.a - src.a * dst.a;
    return result;
}
