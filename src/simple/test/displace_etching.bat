SET SHADERS=-vert shaders\iso.vert -frag shaders\traceandshade_bbox_autocompute_regula_falsi.frag
REM -optimizedComposite
REM-primitives scenes\sphere.txt
SET PRIMITIVES=-primitives %1
SET COMPOSITE=-composite composites\displacement_etching.txt
SET COMPMAT=-compositeMat composites\displace_color.txt
SET STEP=-dCompositeStep 0.01
SET OPTIMIZATION=-optimizedComposite
SET DISPLACEMENT_TEXTURE=-compositeTexture %2
REM textures\displacement\curve4_color.png
frepup-simple.exe %SHADERS% %PRIMITIVES% %COMPOSITE% %COMPMAT% %STEP% %DISPLACEMENT_TEXTURE%