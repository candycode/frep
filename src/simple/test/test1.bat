SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag
REM-primitives scenes\cone_sphere.txt
SET PRIMITIVES=-primitives %1
SET COMPOSITE=-composite composites\subtract.txt
SET COMPMAT=-compositeMat composites\composite_color.txt
frepup-simple.exe %SHADERS% %PRIMITIVES% %COMPOSITE% %COMPMAT%   