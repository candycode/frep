SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag
REM-primitives scenes\cone_sphere.txt
SET PRIMITIVES=-primitives %1
SET COMPOSITE=-composite composites\union.txt
SET COMPMAT=-compositeMat composites\composite_color_blended2.txt
SET STEP=-dCompositeStep 0.005
SET OPTIMIZATION=-optimizedComposite
frepup-simple.exe %SHADERS% %PRIMITIVES% %COMPOSITE% %COMPMAT% %STEP% 