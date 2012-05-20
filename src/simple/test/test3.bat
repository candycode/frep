SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag
SET SCENE=-primitives scenes\sphere_sphere.txt
SET COMPOSITE=-composite composites\subtract_blend.txt
SET COMPMATERIAL=-compositeMat composites\composite_color_blended2.txt
frepup-simple.exe %SHADERS% %SCENE% %COMPOSITE% %COMPMATERIAL%   