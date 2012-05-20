SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute.frag
REM -optimizedComposite
REM-primitives scenes\sphere.txt
SET PRIMITIVES=-primitives %1
SET COMPOSITE=-composite composites\passthrough.txt
SET COMPMAT=-compositeMat composites\ssao_trace_per_frag2_optimal.frag.txt
SET STEP=-dCompositeStep 0.008
SET OPTIMIZATION=-optimizedComposite
frepup-simple.exe %SHADERS% %PRIMITIVES% %COMPOSITE% %COMPMAT% %STEP% 