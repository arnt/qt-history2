#include "colors.inc"
#include "textures.inc"

plane {
    -z,-0.7
    pigment { rgb <1,1,1>*1.225 }
}

#declare Font = "timesbd.ttf"

/*
union {
    text { ttf Font "Troll", 0.2, 0.0 translate <0,1,0> }
    text { ttf Font "Tech", 0.2, 0.0 }
    pigment { 
	gradient y
	color_map {
	    [ 0 rgb <1.4,1.4,1.4> ]
	    [ 1 rgb <0.2,0.4,1> ]
	}
	translate <0,-0.1,0>
    }
    finish { diffuse 1 }
    translate <-0.1,-0.8,0.6>
    scale 0.9
}
*/

#declare R = 0.07
union {
    sphere { <-0.5,-0.5,-0.5> R }
    sphere { <+0.5,-0.5,-0.5> R }
    sphere { <-0.5,+0.5,-0.5> R }
    sphere { <+0.5,+0.5,-0.5> R }
    sphere { <-0.5,-0.5,+0.5> R }
    sphere { <+0.5,-0.5,+0.5> R }
    sphere { <-0.5,+0.5,+0.5> R }
    sphere { <+0.5,+0.5,+0.5> R }
    cylinder { <-0.5,-0.5,-0.5> <+0.5,-0.5,-0.5> R }
    cylinder { <-0.5,-0.5,-0.5> <-0.5,+0.5,-0.5> R }
    cylinder { <-0.5,-0.5,-0.5> <-0.5,-0.5,+0.5> R }
    cylinder { <+0.5,+0.5,+0.5> <-0.5,+0.5,+0.5> R }
    cylinder { <+0.5,+0.5,+0.5> <+0.5,-0.5,+0.5> R }
    cylinder { <+0.5,+0.5,+0.5> <+0.5,+0.5,-0.5> R }
    cylinder { <+0.5,-0.5,-0.5> <+0.5,-0.5,+0.5> R }
    cylinder { <+0.5,-0.5,-0.5> <+0.5,+0.5,-0.5> R }
    cylinder { <-0.5,+0.5,-0.5> <+0.5,+0.5,-0.5> R }
    cylinder { <-0.5,+0.5,-0.5> <-0.5,+0.5,+0.5> R }
    cylinder { <-0.5,-0.5,+0.5> <+0.5,-0.5,+0.5> R }
    cylinder { <-0.5,-0.5,+0.5> <-0.5,+0.5,+0.5> R }

    scale 0.7

    rotate x*-90

    pigment { colour rgb <1,1,0> }
    finish { Shiny }

    rotate z*-25
    rotate y*(clock*360+10)
    rotate x*40
    rotate z*-20

    translate <-0.8,0.06,-0.5>
}

camera {
    location <-0.7,0,-2.9>
    direction z*1.5
    right x
    look_at <-0.7,0,0>
}

light_source {
    <-5,4,-13>*1000
    colour rgb<1,1,1>
    area_light x*1000,y*1000,4,4
    adaptive 1
    jitter
}
