#include "colors.inc"
#include "textures.inc"

#declare C_Background = false
#declare C_Volume = true
#declare C_Funky = false
#declare C_Slider_Handles = false
#declare C_Light = true
#declare C_LightLine = false
#declare C_BHoles = false
#declare C_Glow = true

#declare C_GrooveRad = 4.5
#declare C_GrooveWidth = .15
#declare C_GrooveHeight = 4
#declare C_GrooveAngle = 90
#include "groove.inc"

#declare Volume_X = -3.5
#declare Volume_Y = 0
#declare Volume_Z = -0.57

#declare C_V_angle = 0
#declare C_H_angle = 0 //18

#if (C_Background)
difference {
   box {
#if (C_BHoles)
       <-10, -6, -.49>, <10, 6, -0.5>
#else
       <-10, -6, -.49>, <10, 6, -0.5>
#end
    } 
#if (C_BHoles)
   object {
       C_Groove
       rotate 45*z
       translate<-4,1.5,.1>
   }
#declare C_GrooveRad = 4
#declare C_GrooveWidth = .5
#include "groove.inc"
   union {
       object {
	   C_Groove
	   rotate 180*z
	   translate<3,.2,.5>
       }
       box { <-7,-3.3,1><3, -4.3, -1> }
       cylinder { <-7,-3.8,1><-7, -3.8, -1>, .5 }
   }
   box { <2.5,1.45,1><3.1,1.55, -1> }
   
   /* boxes for the dials, 2*2 */
   box { <6, 2, 1><8,4,-1> }
   box { <-.5, -2, 1><1.5,0,-1> }
   box { <.5, 2, 1><2.5,4,-1> }
   box { <-5, 3, 1><-3,5,-1> }

#end
   box { <3,-1,1><4.05, 4, -1> }

#if (C_BHoles)
   texture {
       pigment {
       //DMFWood5
           wood
	   turbulence .1
	   frequency 1
	   omega .3
	   color_map {
	      [0.00 color rgb <.9, .4, .1>]
	      [0.33 color rgb <1,.5,.2> ]
	      [0.73 color rgb <1,.6,.25> ]
	      [1.00 color rgb <1,.75,.25> ]
	   }
	   rotate 90*x
	   rotate 90*z
	   rotate 10*y
	   //scale 1.2
	   translate <9,0,0>
       }
   }
#end
}

/* the panels */
box { <5.9, 1.9, 0><8.1,4.1,-.1> pigment { color White } }
light_source { <8.2, 3, -.48> color rgb <.6,1,.8>  }

box { <-.5, -2.1, 0><1.6,0.1,-.1> pigment { color White } }
light_source { <1.7, -1, -.48> color rgb <.6,1,.8>  }

box { <.4, 1.9, 0><2.6,4.1,-.1> pigment { color White } }
light_source { <2.7, 3, -.48> color rgb <.6,1,.8>  }

box { <-5.1, 2.9, 0><-2.9,5.1,-.1> pigment { color White } }
light_source { <-2.8, 4, -.48> color rgb <.6,1,.8>  }
   
#end

// line for later 
#if (false)
#declare C_GrooveRad = 4.5
#declare C_GrooveWidth = .01
#include "groove.inc"
object {
   C_Groove
   rotate 45*z
   translate<-4,1.5,.1>
   pigment { color White }
}
#end

#if (C_LightLine)



#declare Count = 0
#declare C_Inc = .5
#declare C_LedSize = .15

#while (Count < 20) 
#if (C_Glow)
sphere { 0,1
  pigment { color rgbt <1,0,0,1> }
  halo {
      emitting
      spherical_mapping
      linear
      color_map {
          [0 color rgbt <.2,.2,1,1> ]
          [.3 color rgbt <.2,.2,1,.4> ]
	  [1 color rgbt <.2,.2,1,0> ]
      }
      samples 10
  }
  finish {
     ambient 1;
  }
  hollow
  scale 2*C_LedSize
  translate <-7 + (C_Inc * Count) ,-3.8,0>

}
#end

sphere { 0,1
#if (C_Glow)
    pigment { color rgbt <.3,.3,1,.0> }
#else 
    pigment { color rgbt <0,0,1,.03> }
#end
    scale C_LedSize;
    translate <-7 + (C_Inc * Count),-3.8,0>
    finish {
#if (C_Glow)
       ambient .9
#else 
       ambient .6
#end
       diffuse 0.1
       reflection 0.25
       specular 1
       roughness .001
   }
}
#declare Count = Count + 1
#end

#declare Count = 0
#declare C_Inc = 7

#while (Count < 14)
#if (C_Glow)
sphere { 0,1
  pigment { color rgbt <1,0,0,1> }
  halo {
      emitting
      spherical_mapping
      linear
      color_map {
          [0 color rgbt <.2,.2,1,1> ]
          [.3 color rgbt <.2,.2,1,.4> ]
	  [1 color rgbt <.2,.2,1,0> ]
      }
      samples 10
  }
  finish {
     ambient 1;
  }
  hollow
  scale 2*C_LedSize
  translate <0,-4, 0>
  rotate Count*C_Inc*z
  translate <3,.2,0>

}
#end

sphere { 0,1
#if (C_Glow)
    pigment { color rgbt <.3,.3,1,.0> }
#else 
    pigment { color rgbt <0,0,1,.03> }
#end
    scale C_LedSize;
  translate <0,-4, 0>
  rotate Count*C_Inc*z
  translate <3,.2,0>
    finish {
#if (C_Glow)
       ambient .9
#else 
       ambient .6
#end
       diffuse 0.1
       reflection 0.25
       specular 1
       roughness .001
   }
}
#declare Count = Count + 1
#end
#end


#if (C_Volume)
cylinder {
       <0,-.88,0><0,-.9,0>, 1.8
   pigment { color rgb <.9,.9,.9> }
   normal { wood 0.2 scale 0.15 rotate 90*x }
   finish { 
       roughness .001
       reflection .75
       metallic
       ambient .5 // it glows

   }
   rotate 90*x
   rotate (90 - C_V_angle)*z
   translate <Volume_X,Volume_Y,Volume_Z >
}

difference {
    cylinder { <0,0,0><0,-1,0>, 2 }
    cylinder { <0,1,0><0,-2,0>, 1.8 }
    pigment { color Black }
    normal { bumps .2 scale 0.04 }
    finish {
        phong .3
    }
   rotate 90*x
   translate <Volume_X,Volume_Y,Volume_Z >
}

sphere {
   <-1.3,0,0> .5
   pigment { color rgb <.9,0,0> }
   normal { bumps .3 scale 0.1 }
   finish {
       phong .3
   }
   rotate (90 - C_V_angle)*z
   translate <Volume_X,Volume_Y, (Volume_Z + -.6) >
}

light_source { 
    <Volume_X,Volume_Y, (Volume_Z + -.2 )> color rgb <1,1,1>  
    area_light <1, 0, 0>,<0,1,0>, 3, 3
    adaptive 1
    jitter
}
light_source { 
    <Volume_X,Volume_Y, (Volume_Z + -.2 )> color rgb <1,1,1>  
    area_light <1, 0, 0>,<0,1,0>, 3, 3
    adaptive 1
    jitter
}

#end

#if (C_Funky)
cylinder {
//   <3.1,1.5,3.5><3.9, 1.5, 3.5>, 4
   <0,0,0><0,1,0>, 3
   //pigment { color rgb <.8,.8,.7> }
   pigment {
       image_map {
           png "bar2.png"
	   map_type 2
       }
   }
   scale <.8, 1, 1>
   rotate -90*z;
   rotate (-90 - C_H_angle)*x;
   translate <3.025, 1.5,3>
}

light_source { <4.05, 2, -.48> color rgb <.6,1,.8>  }
#end

/* Now for the boxes for the blah blah blah */


camera {
    orthographic
    up <0,12,0>
    right <20,0,0>
    location <0,0,-18>
    look_at 0
}

light_source { 
    <12, 12, -25> 
    color White
    spotlight
    radius 15
    falloff 25
    tightness 10
    point_at <0,0,0>
    area_light <9, 0, 0>,<0,9,0>, 5, 5
    adaptive 1
    jitter
}


