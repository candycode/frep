REM NOT WORKING!!!
SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag

frepup-simple.exe  %SHADERS% -primitives %1 -composite composites\subtract_blend3.txt -compositeMat composites\composite_color_blended3.txt -dPrimitiveStep 0.3 -dCompositeStep 0.05