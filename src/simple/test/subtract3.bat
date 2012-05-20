SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag
REM-primitives scenes\cone_sphere.txt
SET PRIMITIVES=-primitives %1
SET COMPOSITE=-composite composites\subtract3.txt
SET COMPMAT=-compositeMat composites\composite_color_blended3.txt
frepup-simple.exe %SHADERS% %PRIMITIVES% %COMPOSITE% %COMPMAT%   